#include <iostream>

#include <gtest/gtest.h>

#include <dummy.grpc.pb.h>

#include "nrp_general_library/engine_interfaces/engine_grpc_interface/engine_server/engine_grpc_device_controller.h"
#include "nrp_general_library/engine_interfaces/engine_grpc_interface/engine_server/engine_grpc_server.h"
#include "nrp_general_library/engine_interfaces/engine_grpc_interface/engine_client/engine_grpc_client.h"
#include "nrp_general_library/process_launchers/process_launcher_basic.h"

class TestGrpcDeviceController : public EngineGrpcDeviceController
{
    public:

        TestGrpcDeviceController(const DeviceIdentifier &devID) : EngineGrpcDeviceController(devID) {}

        virtual const google::protobuf::Message * getData() override
        {
            return &_reply;
        }

        virtual void setData(const google::protobuf::Message & data) override
        {
            _data.CopyFrom(data);

            this->_reply.set_numcalls(_reply.numcalls() + 1);
        }

        dummy::DummyRequest _data;
        dummy::DummyReply   _reply;
};

class TestEngineJSONConfig
        : public EngineJSONConfig<TestEngineJSONConfig, PropNames<> >
{
    public:
        static constexpr FixedString ConfigType = "TestEngineConfig";

        TestEngineJSONConfig(EngineConfigConst::config_storage_t &config)
            : EngineJSONConfig(config)
        {}
};

class TestEngineGrpcClient
        : public EngineGrpcClient<TestEngineGrpcClient, TestEngineJSONConfig>
{
    public:
        TestEngineGrpcClient(EngineConfigConst::config_storage_t &config, ProcessLauncherInterface::unique_ptr &&launcher)
            : EngineGrpcClient(config, std::move(launcher))
        {}

        virtual ~TestEngineGrpcClient() override = default;

        RESULT initialize() override
        {
            this->sendInitCommand();

            return RESULT::SUCCESS;
        }

        RESULT shutdown() override
        {
            this->sendShutdownCommand();

            return RESULT::SUCCESS;
        }
};

TEST(EngineGrpc, BASIC)
{
    // TODO This one has a linking issue, fix it!

    /*EngineGrpcServer server;

    SimulationConfig::config_storage_t config;
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    ASSERT_EQ(client.getChannelStatus(), grpc_connectivity_state::GRPC_CHANNEL_IDLE);

    server.startServer();

    ASSERT_EQ(client.connect(), grpc_connectivity_state::GRPC_CHANNEL_READY);

    server.shutdownServer();*/
}

TEST(EngineGrpc, InitCommand)
{
    EngineGrpcServer server;
    SimulationConfig::config_storage_t config;
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    ASSERT_THROW(client.sendInitCommand(), std::runtime_error);

    server.startServer();
    ASSERT_NO_THROW(client.sendInitCommand());
}

TEST(EngineGrpc, ShutdownCommand)
{
    EngineGrpcServer server;
    SimulationConfig::config_storage_t config;
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    ASSERT_THROW(client.sendShutdownCommand(), std::runtime_error);

    server.startServer();
    ASSERT_NO_THROW(client.sendShutdownCommand());
}

TEST(EngineGrpc, RunLoopStepCommand)
{
    EngineGrpcServer server;
    SimulationConfig::config_storage_t config;
    TestEngineGrpcClient client(config, ProcessLauncherInterface::unique_ptr(new ProcessLauncherBasic()));

    server.startServer();

    float timeStep = 0.1f;
    ASSERT_NEAR(client.sendRunLoopStepCommand(timeStep), timeStep, 0.0001);

    timeStep = -0.1f;
    ASSERT_THROW(client.sendRunLoopStepCommand(timeStep), std::runtime_error);

    timeStep = 2.0f;
    ASSERT_NO_THROW(client.sendRunLoopStepCommand(timeStep));
    timeStep = 1.0f;
    ASSERT_THROW(client.sendRunLoopStepCommand(timeStep), std::runtime_error);
}

TEST(EngineGrpc, RegisterDevices)
{
    EngineGrpcServer server;

    TestGrpcDeviceController dev1(DeviceIdentifier("dev1", "test", "test"));

    ASSERT_EQ(server.getNumRegisteredDevices(), 0);
    server.registerDevice("dev1", &dev1);
    ASSERT_EQ(server.getNumRegisteredDevices(), 1);
}

TEST(EngineGrpc, SetDeviceData)
{
    EngineGrpcServer server;

    const std::string deviceName = "device";

    TestGrpcDeviceController device(DeviceIdentifier("dev1", "test", "test"));

    dummy::DummyRequest request;
    request.set_name("test");

    server.registerDevice(deviceName, &device);

    server.setDeviceData(deviceName, request);

    ASSERT_EQ(device._data.name(), "test");
}

TEST(EngineGrpc, GetDeviceData)
{
    EngineGrpcServer server;

    const std::string deviceName = "device";

    TestGrpcDeviceController device(DeviceIdentifier("dev1", "test", "test"));

    dummy::DummyRequest request;
    request.set_name("test");

    server.registerDevice(deviceName, &device);

    server.setDeviceData(deviceName, request);
    const dummy::DummyReply * reply = dynamic_cast<const dummy::DummyReply *>(server.getDeviceData(deviceName));
    ASSERT_EQ(reply->numcalls(), 1); 
}