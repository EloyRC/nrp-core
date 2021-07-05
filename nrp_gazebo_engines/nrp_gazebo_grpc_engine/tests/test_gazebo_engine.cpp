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

#include <gtest/gtest.h>

#include "nrp_gazebo_grpc_engine/config/gazebo_grpc_config.h"
#include "nrp_gazebo_grpc_engine/config/cmake_constants.h"
<<<<<<< HEAD
=======
#include "nrp_gazebo_devices/physics_camera.h"
#include "nrp_gazebo_devices/physics_joint.h"
#include "nrp_gazebo_devices/physics_link.h"
>>>>>>> 0c552da4cd6b3368efa7cf51b04f1c46ad2e2283
#include "nrp_gazebo_grpc_engine/nrp_client/gazebo_engine_grpc_nrp_client.h"
#include "nrp_general_library/process_launchers/process_launcher_basic.h"

#include "tests/test_env_cmake.h"

#include <fstream>

static constexpr int MAX_DATA_ACQUISITION_TRIALS = 5;

TEST(TestGazeboGrpcEngine, Start)
{
    // Setup config
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "gazebo_grpc";
    config["GazeboWorldFile"] = TEST_EMPTY_WORLD_FILE;
    config["WorldLoadTime"] = 1;
    config["GazeboRNGSeed"] = 12345;

    // Launch gazebo server
    GazeboEngineGrpcLauncher launcher;
    PtrTemplates<GazeboEngineGrpcNRPClient>::shared_ptr engine = std::dynamic_pointer_cast<GazeboEngineGrpcNRPClient>(
            launcher.launchEngine(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic())));

    ASSERT_NE(engine, nullptr);
    sleep(1);

    ASSERT_ANY_THROW(engine->initialize());
}

TEST(TestGazeboGrpcEngine, WorldPlugin)
{
    // Setup config
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "gazebo_grpc";
    config["GazeboWorldFile"] = TEST_WORLD_PLUGIN_FILE;
    config["GazeboRNGSeed"] = 12345;

    // Launch gazebo server
    GazeboEngineGrpcLauncher launcher;
    PtrTemplates<GazeboEngineGrpcNRPClient>::shared_ptr engine = std::dynamic_pointer_cast<GazeboEngineGrpcNRPClient>(
            launcher.launchEngine(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic())));

    ASSERT_NE(engine, nullptr);
    sleep(1);

    ASSERT_NO_THROW(engine->initialize());
    ASSERT_NO_THROW(engine->runLoopStep(toSimulationTime<int, std::milli>(100)));
    ASSERT_NO_THROW(engine->waitForStepCompletion(5.0f));
}

TEST(TestGazeboGrpcEngine, CameraPlugin)
{
    // Setup config
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "gazebo_grpc";
    config["GazeboWorldFile"] = TEST_CAMERA_PLUGIN_FILE;
    config["GazeboRNGSeed"] = 12345;
    std::vector<std::string> env_params ={"GAZEBO_MODEL_PATH=" TEST_GAZEBO_MODELS_DIR ":$GAZEBO_MODEL_PATH"};
    config["EngineEnvParams"] = env_params;

    // Launch gazebo server
    GazeboEngineGrpcLauncher launcher;
    PtrTemplates<GazeboEngineGrpcNRPClient>::shared_ptr engine = std::dynamic_pointer_cast<GazeboEngineGrpcNRPClient>(
            launcher.launchEngine(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic())));

    ASSERT_NE(engine, nullptr);
    sleep(1);

    ASSERT_NO_THROW(engine->initialize());

	// The data is updated asynchronously, on every new frame. It may happen that on first
	// acquisition there's no camera image yet (isEmpty function returns true), so we allow for few acquisition trials.

    const EngineClientInterface::devices_t * devices;
    int trial = 0;

    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
<<<<<<< HEAD
        devices = &engine->updateDevicesFromEngine({DeviceIdentifier("nrp_camera::camera", engine->engineName(), "irrelevant_type")});
=======
        devices = &engine->updateDevicesFromEngine({DeviceIdentifier("nrp_camera::camera", engine->engineName(), PhysicsCamera::TypeName.data())});
>>>>>>> 0c552da4cd6b3368efa7cf51b04f1c46ad2e2283
        ASSERT_EQ(devices->size(), 1);
    }
    while(dynamic_cast<const DeviceInterface&>(*(devices->at(0))).isEmpty() && trial++ < MAX_DATA_ACQUISITION_TRIALS);

	ASSERT_LE(trial, MAX_DATA_ACQUISITION_TRIALS);

<<<<<<< HEAD
    const DataDevice<EngineGrpc::GazeboCamera>& camDat = dynamic_cast<const DataDevice<EngineGrpc::GazeboCamera>&>(*(devices->at(0)));

    ASSERT_EQ(camDat.getData().imageheight(), 240);
    ASSERT_EQ(camDat.getData().imagewidth(),  320);
    ASSERT_EQ(camDat.getData().imagedepth(),  3);
    ASSERT_EQ(camDat.getData().imagedata().size(), 320*240*3);
=======
	const PhysicsCamera &camDat = dynamic_cast<const PhysicsCamera&>(*(devices->at(0)));

    ASSERT_EQ(camDat.imageHeight(), 240);
    ASSERT_EQ(camDat.imageWidth(),  320);
    ASSERT_EQ(camDat.imagePixelSize(),  3);
    ASSERT_EQ(camDat.imageData().size(), 320*240*3);
>>>>>>> 0c552da4cd6b3368efa7cf51b04f1c46ad2e2283
}


TEST(TestGazeboGrpcEngine, JointPlugin)
{
    // Setup config
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "gazebo_grpc";
    config["GazeboWorldFile"] = TEST_JOINT_PLUGIN_FILE;
    config["GazeboRNGSeed"] = 12345;
    std::vector<std::string> env_params ={"GAZEBO_MODEL_PATH=" TEST_GAZEBO_MODELS_DIR ":$GAZEBO_MODEL_PATH"};
    config["EngineEnvParams"] = env_params;

    // Launch gazebo server
    GazeboEngineGrpcLauncher launcher;
    PtrTemplates<GazeboEngineGrpcNRPClient>::shared_ptr engine = std::dynamic_pointer_cast<GazeboEngineGrpcNRPClient>(
            launcher.launchEngine(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic())));

    ASSERT_NE(engine, nullptr);
    sleep(1);

    ASSERT_NO_THROW(engine->initialize());

    // Test device data getting
<<<<<<< HEAD
    auto devices = engine->updateDevicesFromEngine({DeviceIdentifier("youbot::base_footprint_joint",
                                                    engine->engineName(), "irrelevant_type")});
    ASSERT_EQ(devices.size(), 1);

    const auto *pJointDev = dynamic_cast<const DataDevice<EngineGrpc::GazeboJoint> *>(devices[0].get());
    ASSERT_NE(pJointDev, nullptr);
    ASSERT_EQ(pJointDev->getData().position(), 0);
=======
    auto devices = engine->updateDevicesFromEngine({DeviceIdentifier("youbot::base_footprint_joint", engine->engineName(), PhysicsJoint::TypeName.data())});
    ASSERT_EQ(devices.size(), 1);

    const PhysicsJoint *pJointDev = dynamic_cast<const PhysicsJoint*>(devices[0].get());
    ASSERT_EQ(pJointDev->position(), 0);
>>>>>>> 0c552da4cd6b3368efa7cf51b04f1c46ad2e2283

    // Test device data setting
    const auto newTargetPos = 2.0f;

<<<<<<< HEAD
    auto newJointDev = new EngineGrpc::GazeboJoint();
    newJointDev->set_effort(NAN);
    newJointDev->set_velocity(NAN);
    newJointDev->set_position(newTargetPos);
    DataDevice<EngineGrpc::GazeboJoint> dev("youbot::base_footprint_joint", engine->engineName(), newJointDev);

    ASSERT_NO_THROW(engine->sendDevicesToEngine({&dev}));
=======
    PhysicsJoint newJointDev(DeviceIdentifier(pJointDev->id()));
    newJointDev.setEffort(NAN);
    newJointDev.setVelocity(NAN);
    newJointDev.setPosition(newTargetPos);

    ASSERT_NO_THROW(engine->sendDevicesToEngine({&newJointDev}));
>>>>>>> 0c552da4cd6b3368efa7cf51b04f1c46ad2e2283
}

TEST(TestGazeboGrpcEngine, LinkPlugin)
{
    // Setup config
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "gazebo_grpc";
    config["GazeboWorldFile"] = TEST_LINK_PLUGIN_FILE;
    config["GazeboRNGSeed"] = 12345;
    std::vector<std::string> env_params ={"GAZEBO_MODEL_PATH=" TEST_GAZEBO_MODELS_DIR ":$GAZEBO_MODEL_PATH"};
    config["EngineEnvParams"] = env_params;

    // Launch gazebo server
    GazeboEngineGrpcLauncher launcher;
    PtrTemplates<GazeboEngineGrpcNRPClient>::shared_ptr engine = std::dynamic_pointer_cast<GazeboEngineGrpcNRPClient>(
            launcher.launchEngine(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic())));

    ASSERT_NE(engine, nullptr);
    sleep(1);

    ASSERT_NO_THROW(engine->initialize());

    // Test device data getting
<<<<<<< HEAD
    auto devices = engine->updateDevicesFromEngine({DeviceIdentifier("link_youbot::base_footprint",
                                                    engine->engineName(), "irrelevant_type")});
    ASSERT_EQ(devices.size(), 1);

    const auto *pLinkDev = dynamic_cast<const DataDevice<EngineGrpc::GazeboLink> *>(devices[0].get());
=======
    auto devices = engine->updateDevicesFromEngine({DeviceIdentifier("link_youbot::base_footprint", engine->engineName(), PhysicsJoint::TypeName.data())});
    ASSERT_EQ(devices.size(), 1);

    const PhysicsLink *pLinkDev = dynamic_cast<const PhysicsLink*>(devices[0].get());
>>>>>>> 0c552da4cd6b3368efa7cf51b04f1c46ad2e2283
    ASSERT_NE(pLinkDev, nullptr);

    // TODO: Check that link state is correct
}
