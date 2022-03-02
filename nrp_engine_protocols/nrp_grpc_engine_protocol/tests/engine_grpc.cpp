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

#include <iostream>

#include <gtest/gtest.h>

#include "nrp_general_library/process_launchers/process_launcher_basic.h"
#include "nrp_grpc_engine_protocol/config/engine_grpc_config.h"
#include "nrp_grpc_engine_protocol/engine_server/engine_grpc_server.h"
#include "nrp_grpc_engine_protocol/engine_client/engine_grpc_client.h"
#include "nrp_protobuf/engine_grpc.grpc.pb.h"
#include "nrp_protobuf/test_msgs.pb.h"



void testSleep(unsigned sleepMs)
{
    std::chrono::milliseconds timespan(sleepMs);
    std::this_thread::sleep_for(timespan);
}


class TestGrpcDataPackController
        : public DataPackController<google::protobuf::Message>
{
    public:

        TestGrpcDataPackController()
            : _data(new EngineTest::TestPayload())
        { }

        virtual void handleDataPackData(const google::protobuf::Message &data) override
        {
            // throws bad_cast
            const auto &j = dynamic_cast<const EngineTest::TestPayload &>(data);
            _data->CopyFrom(j);
        }

        virtual google::protobuf::Message *getDataPackInformation() override
        {
            if(this->_returnEmptyDataPack)
                return nullptr;
            else {
                auto old_data = _data;
                _data = new EngineTest::TestPayload();
                _data->CopyFrom(*old_data);
                return old_data;
            }
        }

        void triggerEmptyDataPackReturn(bool value)
        {
            this->_returnEmptyDataPack = value;
        }

    private:
        EngineTest::TestPayload* _data;
        bool _returnEmptyDataPack = false;
};

struct TestEngineGRPCConfigConst
{
    static constexpr char EngineType[] = "test_engine";
    static constexpr char EngineSchema[] = "https://neurorobotics.net/engines/engine_comm_protocols.json#/engine_grpc";
};

class TestEngineGrpcClient
: public EngineGrpcClient<TestEngineGrpcClient, TestEngineGRPCConfigConst::EngineSchema, EngineTest::TestPayload>
{
    public:
        TestEngineGrpcClient(nlohmann::json &config, ProcessLauncherInterface::unique_ptr &&launcher)
            : EngineGrpcClient(config, std::move(launcher))
        {}

        void initialize() override
        {
            this->sendInitializeCommand("test");
        }

        void reset() override
        {
            this->sendResetCommand();
        }

        void shutdown() override
        {
            this->sendShutdownCommand("test");
        }
};

class TestEngineGrpcServer
    : public EngineGrpcServer
{
    public:

        TestEngineGrpcServer(const std::string & serverAddress)
            : EngineGrpcServer(serverAddress)
        {

        }

        virtual ~TestEngineGrpcServer() override = default;

        void initialize(const nlohmann::json &data, EngineGrpcServer::lock_t &) override
        {
            specialBehaviour();

            if(data.at("throw"))
            {
                throw std::runtime_error("Init failed");
            }
        }

        void reset() override
        {
            specialBehaviour();

            this->resetEngineTime();
        }

        void shutdown(const nlohmann::json &data) override
        {
            specialBehaviour();

            if(data.at("throw"))
            {
                throw std::runtime_error("Shutdown failed");
            }
        }

        void timeoutOnNextCommand(int sleepTimeMs = 1500)
        {
            this->_sleepTimeMs = sleepTimeMs;
        }

        SimulationTime runLoopStep(const SimulationTime timeStep) override
        {
            specialBehaviour();

            _time += timeStep;

            return _time;
        }

        void resetEngineTime()
        {
            this->_time = SimulationTime::zero();
        }

        SimulationTime getEngineTime()
        {
            return this->_time;
        }

    private:

        void specialBehaviour()
        {
            if(this->_sleepTimeMs != 0)
            {
                testSleep(this->_sleepTimeMs);
                this->_sleepTimeMs = 0;
            }
        }

        SimulationTime _time = SimulationTime::zero();
        int            _sleepTimeMs = 0;
};

TEST(EngineGrpc, BASIC)
{
    // TODO This one has a linking issue, fix it!

    /*TestEngineGrpcServer               server;
    SimulationConfig::config_storage_t config;
    TestEngineGrpcClient               client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    ASSERT_EQ(client.getChannelStatus(), grpc_connectivity_state::GRPC_CHANNEL_IDLE);

    server.startServer();

    ASSERT_EQ(client.connect(), grpc_connectivity_state::GRPC_CHANNEL_READY);

    server.shutdownServer();*/
}

TEST(EngineGrpc, InitCommand)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    nlohmann::json jsonMessage;
    jsonMessage["init"]    = true;
    jsonMessage["throw"]   = false;

    // The gRPC server isn't running, so the init command should fail

    ASSERT_THROW(client.sendInitializeCommand(jsonMessage), std::runtime_error);

    // Start the server and send the init command. It should succeed

    server.startServer();
    // TODO Investigate why this is needed. It seems to be caused by the previous call to sendInitCommand function
    testSleep(1500);
    client.sendInitializeCommand(jsonMessage);

    // Force the server to return an error from the rpc
    // Check if the client receives an error response on command handling failure

    jsonMessage["throw"] = true;
    ASSERT_THROW(client.sendInitializeCommand(jsonMessage), std::runtime_error);
}

TEST(EngineGrpc, InitCommandTimeout)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";
    config["EngineCommandTimeout"] = 0.0005;

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    nlohmann::json jsonMessage;
    jsonMessage["init"]    = true;
    jsonMessage["throw"]   = false;

    // Test init command timeout

    server.startServer();
    server.timeoutOnNextCommand();
    ASSERT_THROW(client.sendInitializeCommand(jsonMessage), std::runtime_error);
}

TEST(EngineGrpc, ShutdownCommand)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    nlohmann::json jsonMessage;
    jsonMessage["shutdown"] = true;
    jsonMessage["throw"]    = false;

    // The gRPC server isn't running, so the shutdown command should fail

    ASSERT_THROW(client.sendShutdownCommand(jsonMessage), std::runtime_error);

    // Start the server and send the shutdown command. It should succeed

    server.startServer();
    // TODO Investigate why this is needed. It seems to be caused by the previous call to sendInitCommand function
    testSleep(1500);
    ASSERT_NO_THROW(client.sendShutdownCommand(jsonMessage));

    // Force the server to return an error from the rpc
    // Check if the client receives an error response on command handling failure

    jsonMessage["throw"] = true;
    ASSERT_THROW(client.sendShutdownCommand(jsonMessage), std::runtime_error);
}

TEST(EngineGrpc, ShutdownCommandTimeout)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";
    config["EngineCommandTimeout"] = 1;

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    nlohmann::json jsonMessage;
    jsonMessage["shutdown"] = true;
    jsonMessage["throw"]    = false;

    // Test shutdown command timeout

    server.startServer();
    server.timeoutOnNextCommand();
    ASSERT_THROW(client.sendShutdownCommand(jsonMessage), std::runtime_error);
}

static SimulationTime floatToSimulationTime(float time)
{
    return toSimulationTime<float, std::ratio<1>>(time);
}

TEST(EngineGrpc, RunLoopStepCommand)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    // The gRPC server isn't running, so the runLoopStep command should fail

    SimulationTime timeStep = floatToSimulationTime(0.1f);
    ASSERT_THROW(client.runLoopStepCallback(timeStep), std::runtime_error);

    server.startServer();

    // Engine time should never be smaller than 0

    server.resetEngineTime();
    timeStep = floatToSimulationTime(-0.1f);
    ASSERT_THROW(client.runLoopStepCallback(timeStep), std::runtime_error);

    // Normal loop execution, the command should return engine time

    server.resetEngineTime();
    timeStep = floatToSimulationTime(1.0f);
    ASSERT_NEAR(client.runLoopStepCallback(timeStep).count(), timeStep.count(), 0.0001);

    // Try to go back in time. The client should raise an error when engine time is decreasing

    server.resetEngineTime();
    timeStep = floatToSimulationTime(2.0f);
    ASSERT_NO_THROW(client.runLoopStepCallback(timeStep));
    timeStep = floatToSimulationTime(-1.0f);
    ASSERT_THROW(client.runLoopStepCallback(timeStep), std::runtime_error);

    // TODO Add test for failure on server side
}

TEST(EngineGrpc, runLoopStepCommandTimeout)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";
    config["EngineCommandTimeout"] = 1;

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    // Test runLoopStep command timeout

    server.startServer();
    server.timeoutOnNextCommand();
    SimulationTime timeStep = floatToSimulationTime(2.0f);
    ASSERT_THROW(client.runLoopStepCallback(timeStep), std::runtime_error);
}

TEST(EngineGrpc, ResetCommand)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    nlohmann::json jsonMessage;
    jsonMessage["init"]    = true;
    jsonMessage["throw"]   = false;

    // The gRPC server isn't running, so the reset command should fail

    ASSERT_THROW(client.sendResetCommand(), std::runtime_error);

    // Start the server and send the reset command. It should succeed

    server.startServer();
    // TODO Investigate why this is needed. It seems to be caused by the previous call to sendInitCommand function
    testSleep(1500);
    ASSERT_NO_THROW(client.sendResetCommand());

    // Normal loop execution, the reset should return time to zero

    SimulationTime timeStep = floatToSimulationTime(0.1f);
    ASSERT_NO_THROW(client.runLoopStepCallback(timeStep));
    ASSERT_NO_THROW(client.sendResetCommand());
    ASSERT_EQ(client.getEngineTime().count(), 0);
}

TEST(EngineGrpc, ResetCommandTimeout)
{
    nlohmann::json config;
    config["EngineName"] = "engine";
    config["EngineType"] = "test_engine_grpc";
    config["EngineCommandTimeout"] = 1;

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    // Test runLoopStep command timeout

    server.startServer();
    server.timeoutOnNextCommand();
    ASSERT_THROW(client.sendResetCommand(), std::runtime_error);
}

TEST(EngineGrpc, RegisterDataPacks)
{
    TestEngineGrpcServer server("localhost:9004");
    TestGrpcDataPackController *dev1 = nullptr;

    ASSERT_EQ(server.getNumRegisteredDataPacks(), 0);
    server.registerDataPack("dev1", dev1);
    ASSERT_EQ(server.getNumRegisteredDataPacks(), 1);
}

TEST(EngineGrpc, SetDataPackData)
{
    const std::string datapackName = "a";
    const std::string engineName = "c";

    nlohmann::json config;
    config["EngineName"] = engineName;
    config["EngineType"] = "test_engine_grpc";

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    std::vector<DataPackInterface*> input_datapacks;

    std::shared_ptr<TestGrpcDataPackController> datapackController(new TestGrpcDataPackController()); // Server side
    server.registerDataPack(datapackName, datapackController.get());

    std::shared_ptr<DataPack<EngineTest::TestPayload>> dev1(new DataPack<EngineTest::TestPayload>(datapackName, engineName)); // Client side
    input_datapacks.push_back(dev1.get());

    // The gRPC server isn't running, so the sendDataPacksToEngine command should fail
    ASSERT_THROW(client.sendDataPacksToEngine(input_datapacks), NRPException::exception);

    // Starts the Engine
    server.startServer();
    testSleep(1500);

    input_datapacks.clear();
    auto d = new EngineTest::TestPayload();
    d->set_integer(111);
    dev1.reset(new DataPack<EngineTest::TestPayload>(datapackName, engineName, d));
    input_datapacks.push_back(dev1.get());

    // Normal command execution
    client.sendDataPacksToEngine(input_datapacks);
    d = dynamic_cast<EngineTest::TestPayload *>(datapackController->getDataPackInformation());

    ASSERT_EQ(d->integer(),       111);

    // Test setting data on a datapack that wasn't registered in the engine server
    const std::string datapackName2 = "b";
    DataPack<EngineTest::TestPayload> dev2(datapackName2, engineName);
    input_datapacks.clear();
    input_datapacks.push_back(&dev2);

    ASSERT_THROW(client.sendDataPacksToEngine(input_datapacks), NRPException::exception);

    // TODO Add test for setData timeout
}

TEST(EngineGrpc, GetDataPackData)
{
    const std::string datapackName = "a";
    const std::string datapackType = "b";
    const std::string engineName = "c";

    nlohmann::json config;
    config["EngineName"] = engineName;
    config["EngineType"] = "test_engine_grpc";

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    // Client sends a request to the server

    std::vector<DataPackInterface*> input_datapacks;

    DataPackIdentifier         devId(datapackName, engineName, datapackType);
    DataPack<EngineTest::TestPayload> dev1(datapackName, engineName); // Client side
    std::shared_ptr<TestGrpcDataPackController> datapackController(new TestGrpcDataPackController()); // Server side

    server.registerDataPack(datapackName, datapackController.get());

    input_datapacks.push_back(&dev1);

    EngineClientInterface::datapack_identifiers_set_t datapackIdentifiers;
    datapackIdentifiers.insert(devId);

    // The gRPC server isn't running, so the updateDataPacksFromEngine command should fail

    ASSERT_THROW(client.updateDataPacksFromEngine(datapackIdentifiers), std::runtime_error);

    server.startServer();
    testSleep(1500);
    client.sendDataPacksToEngine(input_datapacks);

    // Return an empty datapack from the server
    // It should be inserted into the engines cache, but should be marked as empty

    datapackController->triggerEmptyDataPackReturn(true);

    auto output = client.updateDataPacksFromEngine(datapackIdentifiers);

    ASSERT_EQ(output.size(), 1);
    ASSERT_EQ(output.at(0)->name(),       datapackName);
    ASSERT_EQ(output.at(0)->engineName(), engineName);
    ASSERT_EQ(output.at(0)->isEmpty(),    true);

    datapackController->triggerEmptyDataPackReturn(false);

    // Normal command execution
    // Engine cache should be updated with a non-empty datapack

    output = client.updateDataPacksFromEngine(datapackIdentifiers);

    ASSERT_EQ(output.size(), 1);
    ASSERT_EQ(output.at(0)->name(),       datapackName);
    ASSERT_EQ(output.at(0)->type(),  dev1.type());
    ASSERT_EQ(output.at(0)->engineName(), engineName);
    ASSERT_EQ(output.at(0)->isEmpty(),    false);

    // Trigger return of an empty datapack again
    // Check that it doesn't overwrite the cache

    datapackController->triggerEmptyDataPackReturn(true);

    output = client.updateDataPacksFromEngine(datapackIdentifiers);

    ASSERT_EQ(output.size(), 1);
    ASSERT_EQ(output.at(0)->isEmpty(), false);

    datapackController->triggerEmptyDataPackReturn(false);

    // Test requesting a datapack that wasn't registered in the engine server

    const std::string datapackName2 = "b";

    DataPackIdentifier         devId2(datapackName2, engineName, datapackType);
    datapackIdentifiers.insert(devId2);

    ASSERT_THROW(client.updateDataPacksFromEngine(datapackIdentifiers), std::runtime_error);

    // TODO Add test for getData timeout
}

TEST(EngineGrpc, GetDataPackData2)
{
    const std::string engineName = "c";

    const std::string datapackName1 = "a";
    const std::string datapackType1 = "test_type1";
    const std::string datapackName2 = "b";
    const std::string datapackType2 = "test_type2";

    nlohmann::json config;
    config["EngineName"] = engineName;
    config["EngineType"] = "test_engine_grpc";;

    TestEngineGrpcServer server("localhost:9004");
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    // Client sends a request to the server

    std::vector<DataPackInterface*> input_datapacks;

    DataPackIdentifier         devId1(datapackName1, engineName, datapackType1);
    DataPackIdentifier         devId2(datapackName2, engineName, datapackType2);
    DataPack<EngineTest::TestPayload> dev1(datapackName1, engineName); // Client side
    DataPack<EngineTest::TestPayload> dev2(datapackName2, engineName); // Client side
    std::shared_ptr<TestGrpcDataPackController> datapackController1(new TestGrpcDataPackController()); // Server side
    std::shared_ptr<TestGrpcDataPackController> datapackController2(new TestGrpcDataPackController()); // Server side

    server.registerDataPack(datapackName1, datapackController1.get());
    server.registerDataPack(datapackName2, datapackController2.get());

    input_datapacks.push_back(&dev1);
    input_datapacks.push_back(&dev2);

    server.startServer();
    client.sendDataPacksToEngine(input_datapacks);

    EngineClientInterface::datapack_identifiers_set_t datapackIdentifiers;
    datapackIdentifiers.insert(devId1);
    datapackIdentifiers.insert(devId2);

    const auto output = client.updateDataPacksFromEngine(datapackIdentifiers);

    ASSERT_EQ(output.size(), 2);
    ASSERT_EQ(output.at(0)->engineName(), engineName);
    ASSERT_EQ(output.at(1)->engineName(), engineName);
    ASSERT_EQ(output.at(0)->type(), dev1.type());
    ASSERT_EQ(output.at(1)->type(), dev1.type());

    if(output.at(0)->name().compare(datapackName1) == 0)
    {
        ASSERT_EQ(output.at(0)->name(), datapackName1);
        ASSERT_EQ(output.at(1)->name(), datapackName2);
    }
    else
    {
        ASSERT_EQ(output.at(0)->name(), datapackName2);
        ASSERT_EQ(output.at(1)->name(), datapackName1);
    }
}

// EOF
