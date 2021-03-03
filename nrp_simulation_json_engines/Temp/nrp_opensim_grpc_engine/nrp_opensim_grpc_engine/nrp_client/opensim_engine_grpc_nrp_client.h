/* * NRP Core - Backend infrastructure to synchronize simulations
 *
 * Copyright 2020 Michael Zechmair
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project has received funding from the European Union’s Horizon 2020
 * Framework Programme for Research and Innovation under the Specific Grant
 * Agreement No. 945539 (Human Brain Project SGA3).
 */

#ifndef OPENSIM_ENGINE_GRPC_NRP_CLIENT_H
#define OPENSIM_ENGINE_GRPC_NRP_CLIENT_H

#include "nrp_grpc_engine_protocol/engine_client/engine_grpc_client.h"
#include "nrp_general_library/engine_interfaces/engine_interface.h"
#include "nrp_general_library/plugin_system/plugin.h"

#include "nrp_opensim_grpc_engine/devices/grpc_opensim_physics_joint.h"

#include "nrp_opensim_grpc_engine/config/opensim_grpc_config.h"

#include <unistd.h>


/*!
 *  \brief NRP - Opensim Communicator on the NRP side. Converts DeviceInterface classes from/to JSON objects
 */
class OpensimEngineGrpcNRPClient
        : public EngineGrpcClient<OpensimEngineGrpcNRPClient, OpensimGrpcConfig, OpensimPhysicsJoint>
{
	public:
		OpensimEngineGrpcNRPClient(EngineConfigConst::config_storage_t &config, ProcessLauncherInterface::unique_ptr &&launcher);
		virtual ~OpensimEngineGrpcNRPClient() override = default;

		virtual void initialize() override;

		virtual void shutdown() override;
};

using OpensimEngineGrpcLauncher = OpensimEngineGrpcNRPClient::EngineLauncher<OpensimGrpcConfig::DefEngineType>;

CREATE_NRP_ENGINE_LAUNCHER(OpensimEngineGrpcLauncher);



#endif // OPENSIM_ENGINE_GRPC_NRP_CLIENT_H