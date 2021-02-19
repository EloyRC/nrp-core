//
// NRP Core - Backend infrastructure to synchronize simulations
//
// Copyright 2020 Michael Zechmair
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

#include "nrp_opensim_grpc_engine/nrp_client/opensim_engine_grpc_nrp_client.h"

#include <signal.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <chrono>

OpensimEngineGrpcNRPClient::OpensimEngineGrpcNRPClient(EngineConfigConst::config_storage_t &config, ProcessLauncherInterface::unique_ptr &&launcher)
    : EngineGrpcClient(config, std::move(launcher))
{}

void OpensimEngineGrpcNRPClient::initialize()
{
	std::cout << "\n From opensim_engine_grpc_nrp_client.cpp \n \n";
	// Wait for Opensim to load world
	auto confDat = this->engineConfig()->writeConfig();
	try
	{
		this->sendInitCommand(confDat);
	}
	catch(std::exception& e)
	{
		throw NRPException::logCreate(e, "Engine \"" + this->engineName() + "\" initialization failed");
	}
}

void OpensimEngineGrpcNRPClient::shutdown()
{
	try
	{
		this->sendShutdownCommand(nlohmann::json());
	}
	catch(std::exception &e)
	{
		throw NRPException::logCreate("Engine \"" + this->engineName() + "\" shutdown failed");
	}
}