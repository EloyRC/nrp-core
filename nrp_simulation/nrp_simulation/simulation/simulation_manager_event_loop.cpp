//
// NRP Core - Backend infrastructure to synchronize simulations
//
// Copyright 2020-2021 NRP Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This project has received funding from the European Union’s Horizon 2020
// Framework Programme for Research and Innovation under the Specific Grant
// Agreement No. 945539 (Human Brain Project SGA3).
//

#include "nrp_simulation/simulation/simulation_manager_event_loop.h"

#include "nrp_general_library/utils/file_finder.h"
#include "nrp_general_library/utils/nrp_exceptions.h"
#include "nrp_simulation/config/cmake_conf.h"
#include "nrp_general_library/utils/nrp_logger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <future>
#include <filesystem>


EventLoopSimManager::EventLoopSimManager(const jsonSharedPtr &simulationConfig, const EngineLauncherManagerConstSharedPtr& engineLauncherManager,
                  const MainProcessLauncherManager::const_shared_ptr &processLauncherManager) :
                  SimulationManager(simulationConfig),
                  _engineLauncherManager(engineLauncherManager),
                  _processLauncherManager(processLauncherManager)
{}

void EventLoopSimManager::initializeCB()
{
    if(this->_loop == nullptr) {
        // Compute time step and timeout
        int eTstep;
        if(this->_simConfig->contains("EventLoopTimestep"))
            eTstep = 1000 * this->_simConfig->at("EventLoopTimestep").get<float>();
        else
            eTstep = 1000 * this->_simConfig->at("SimulationTimestep").get<float>();

        int eTout;
        if(this->_simConfig->contains("EventLoopTimeout"))
            eTout = 1000 * this->_simConfig->at("EventLoopTimeout").get<float>();
        else
            eTout = 1000 * this->_simConfig->at("SimulationTimeout").get<float>();

        _timestep = std::chrono::milliseconds(eTstep);
        _timeout = std::chrono::milliseconds(eTout);

        // Create and initialize EventLoop
        this->_loop.reset(new EventLoop(this->_simConfig->at("ComputationalGraph"), _timestep,
                                        false, this->_simConfig->contains("ConnectROS")));

        // If there are engines in the configuration, an FTILoop has to be run as well
        if(this->_simConfig->at("EngineConfigs").size() > 0) {
            _fTILoopSimManager.reset(new FTILoopSimManager(this->_simConfig, this->_engineLauncherManager,
                                                           this->_processLauncherManager));
            _fTILoopSimManager->initializeSimulation();
        }
    }
    else
        throw NRPException::logCreate("EventLoop already initialized");
}

bool EventLoopSimManager::resetCB()
{
    throw NRPException::logCreate("Reset is not implemented in the EventLoop");
}

void EventLoopSimManager::stopCB()
{
    this->_loop->stopLoop();
}

bool  EventLoopSimManager::runUntilMilliseconds(const std::chrono::milliseconds& eTout)
{
    std::future<void> runFuture;

    // start FTILoop
    if(_fTILoopSimManager) {
        std::function<void()> run_ftiloop = [&]() {
            _fTILoopSimManager->runSimulation(0, nlohmann::json());
        };

        runFuture = std::async(run_ftiloop);
    }

    // run EventLoop
    _loop->runLoop(eTout);

    // stop FTILoop
    if(_fTILoopSimManager) {
        _fTILoopSimManager->stopSimulation();
        runFuture.wait();
    }

    return true;
}

bool EventLoopSimManager::runUntilTimeOutCB()
{
    return runUntilMilliseconds(_timeout);
}

bool EventLoopSimManager::runCB(unsigned numIterations, const nlohmann::json & /*clientData*/)
{
    return runUntilMilliseconds(_timestep * numIterations);
}

void EventLoopSimManager::shutdownCB()
{
    if(this->_loop != nullptr) {
        this->_loop->shutdown();
        this->_loop.reset();
    }
    if(_fTILoopSimManager) {
        _fTILoopSimManager->shutdownSimulation();
        _fTILoopSimManager.reset();
    }
}
