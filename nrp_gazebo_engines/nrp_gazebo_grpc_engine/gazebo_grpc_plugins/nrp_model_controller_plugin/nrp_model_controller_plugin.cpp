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

#include "nrp_model_controller_plugin/nrp_model_controller_plugin.h"
#include "nrp_gazebo_grpc_engine/engine_server/nrp_communication_controller.h"

#include <gazebo/physics/Model.hh>

using namespace nlohmann;

void gazebo::NRPModelControllerPlugin::Load(gazebo::physics::ModelPtr model, sdf::ElementPtr)
{
    NRP_LOGGER_TRACE("{} called", __FUNCTION__);
    
    auto &commControl = NRPCommunicationController::getInstance();

    // Register a datapack for the model
    const auto datapackName = NRPCommunicationController::createDataPackName(*this, model->GetName());
    this->_modelInterface.reset(new ModelGrpcDataPackController(datapackName, model));
    NRPLogger::info("Registering Model controller for model [ {} ]", datapackName);
    commControl.registerDataPack(datapackName, this->_modelInterface.get());

    // Register plugin
    commControl.registerModelPlugin(this);
}