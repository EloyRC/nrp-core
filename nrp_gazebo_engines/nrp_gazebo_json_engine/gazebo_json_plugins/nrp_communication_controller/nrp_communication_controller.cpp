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

#include "nrp_communication_controller/nrp_communication_controller.h"

#include "nrp_general_library/utils/nrp_exceptions.h"

#include <nlohmann/json.hpp>

using namespace nlohmann;

std::unique_ptr<NRPCommunicationController> NRPCommunicationController::_instance = nullptr;

NRPCommunicationController::~NRPCommunicationController()
{
    this->_stepController = nullptr;
}

NRPCommunicationController &NRPCommunicationController::getInstance()
{
    return *(NRPCommunicationController::_instance.get());
}

NRPCommunicationController &NRPCommunicationController::resetInstance(const std::string &serverURL, const std::string &engineName, const std::string &registrationURL)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);

    NRPCommunicationController::_instance.reset(new NRPCommunicationController(serverURL, engineName, registrationURL));
    return NRPCommunicationController::getInstance();

}

void NRPCommunicationController::registerStepController(GazeboStepController *stepController)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);

    EngineJSONServer::lock_t lock(this->_datapackLock);
    this->_stepController = stepController;
}

SimulationTime NRPCommunicationController::runLoopStep(SimulationTime timeStep)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);

    if(this->_stepController == nullptr)
        throw NRPException::logCreate("Tried to run loop while the controller has not yet been initialized");

    try
    {
        // Execute loop step (Note: The _datapackLock mutex has already been set by EngineJSONServer::runLoopStepHandler, so no calls to reading/writing from/to datapacks is possible at this moment)
        return this->_stepController->runLoopStep(timeStep);
    }
    catch(std::exception &e)
    {
        throw NRPException::logCreate(e, "Error during Gazebo stepping");
    }
}

json NRPCommunicationController::initialize(const json &config, const nlohmann::json &/*clientData*/, EngineJSONServer::lock_t &lock)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);

    double waitTime = config.at("WorldLoadTime");
    if(waitTime <= 0)
        waitTime = std::numeric_limits<double>::max();

    // Allow datapacks to register
    lock.unlock();

    // Wait until world plugin loads and forces a load of all other plugins
    while(this->_stepController == nullptr ? 1 : !this->_stepController->finishWorldLoading())
    {
        // Wait for 100ms before retrying
        waitTime -= 0.1;
        usleep(100*1000);

        if(waitTime <= 0)
        {
            lock.lock();
            return nlohmann::json({false});
        }
    }

    lock.lock();

    return nlohmann::json({true});
}

json NRPCommunicationController::reset(const nlohmann::json &/*clientData*/, EngineJSONServer::lock_t &lock)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);

    try{
        // Is it enough to reset just the world?
        this->_stepController->resetWorld();
        for (size_t i = 0; i < this->_sensorPlugins.size(); i++){
            this->_sensorPlugins[i]->Reset();
        }
        for (size_t i = 0; i < this->_modelPlugins.size(); i++){
            this->_modelPlugins[i]->Reset();
        }
    }
    catch(const std::exception &e)
    {
        NRPLogger::error("NRPCommunicationController::reset: failed to resetWorld()");

        return nlohmann::json({false});
    }
    
    return nlohmann::json({true});
}

json NRPCommunicationController::shutdown(const json&)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);

    return nlohmann::json();
}


NRPCommunicationController::NRPCommunicationController(const std::string &serverURL, const std::string &engineName, const std::string &registrationURL)
    : EngineJSONServer(serverURL, engineName, registrationURL)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);
}
