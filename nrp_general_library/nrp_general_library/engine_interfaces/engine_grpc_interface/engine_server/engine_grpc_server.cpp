#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "nrp_general_library/engine_interfaces/engine_grpc_interface/engine_server/engine_grpc_server.h"

grpc::Status EngineGrpcServer::init(grpc::ServerContext * context, const EngineGrpc::InitRequest * request, EngineGrpc::InitReply * reply)
{
    try
    {
        // Run initialization function
        reply->set_json(this->initialize(request->json()).dump());
    }
    catch(const std::exception &e)
    {
        std::cerr << "Error while executing initialization\n";
        std::cerr << e.what();

        return grpc::Status::CANCELLED;
    }

    return grpc::Status::OK;
}

grpc::Status EngineGrpcServer::shutdown(grpc::ServerContext * context, const EngineGrpc::ShutdownRequest * request, EngineGrpc::ShutdownReply * reply)
{
    try
    {
        // Run shutdown function
        reply->set_json(this->shutdown(request->json()).dump());
    }
    catch(const std::exception &e)
    {
        std::cerr << "Error while executing shutdown\n";
        std::cerr << e.what();

        return grpc::Status::CANCELLED;
    }

    return grpc::Status::OK;
}

grpc::Status EngineGrpcServer::runLoopStep(grpc::ServerContext * context, const EngineGrpc::RunLoopStepRequest * request, EngineGrpc::RunLoopStepReply * reply)
{
    try
    {
        reply->set_enginetime(this->runLoopStep(request->timestep()));
    }
    catch(const std::exception &e)
    {
        std::cerr << "Error while executing runLoopStep\n";
        std::cerr << e.what();

        return grpc::Status::CANCELLED;
    }

    return grpc::Status::OK;
}

grpc::Status EngineGrpcServer::setDevice(grpc::ServerContext * context, const EngineGrpc::SetDeviceRequest * request, EngineGrpc::SetDeviceReply * reply)
{
    try
    {
        this->setDeviceData(*request);
    }
    catch(const std::exception &e)
    {
        std::cerr << "Error while executing runLoopStep\n";
        std::cerr << e.what();

        return grpc::Status::CANCELLED;
    }

    return grpc::Status::OK;
}

grpc::Status EngineGrpcServer::getDevice(grpc::ServerContext * context, const EngineGrpc::GetDeviceRequest * request, EngineGrpc::GetDeviceReply * reply)
{
    try
    {
        reply->CopyFrom(*this->getDeviceData(*request));
    }
    catch(const std::exception &e)
    {
        std::cerr << "Error while executing runLoopStep\n";
        std::cerr << e.what();

        return grpc::Status::CANCELLED;
    }

    return grpc::Status::OK;
}


EngineGrpcServer::EngineGrpcServer()
{
    this->_serverAddress   = "0.0.0.0:9002";
    this->_isServerRunning = false;

    grpc::EnableDefaultHealthCheckService(true);
}

EngineGrpcServer::~EngineGrpcServer()
{
	this->shutdownServer();
}

void EngineGrpcServer::startServer()
{
	if(!this->_isServerRunning)
	{
        grpc::ServerBuilder builder;
        builder.AddListeningPort(_serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(this);

        this->_server = builder.BuildAndStart();
		this->_isServerRunning = true;
	}
}

void EngineGrpcServer::shutdownServer()
{
	if(this->_isServerRunning)
	{
		this->_server->Shutdown();
		this->_isServerRunning = false;
	}
}

void EngineGrpcServer::registerDevice(const std::string &deviceName, EngineGrpcDeviceController *interface)
{
    this->_devicesControllers.emplace(deviceName, interface);
}

unsigned EngineGrpcServer::getNumRegisteredDevices()
{
    return this->_devicesControllers.size();
}

void EngineGrpcServer::setDeviceData(const EngineGrpc::SetDeviceRequest & data)
{
    const auto numDevices = data.request_size();

    for(unsigned i = 0; i < numDevices; i++)
    {
        const auto r = data.request(i);
        const auto devInterface = this->_devicesControllers.find(r.deviceid().devicename());

        if(devInterface != _devicesControllers.end())
        {
            devInterface->second->setData(r);
            // TODO Error handling for dev not found
        }
    }
}

const EngineGrpc::GetDeviceReply * EngineGrpcServer::getDeviceData(const EngineGrpc::GetDeviceRequest & data)
{
    // TODO Check if clearing is enough, it may be that the message will grow uncontrollably, because of add_reply() calls!
    this->_getReply.Clear();

    const auto numDevices = data.deviceid_size();

    for(unsigned i = 0; i < numDevices; i++)
    {
        const auto devInterface = this->_devicesControllers.find(data.deviceid(i).devicename());

        if(devInterface != _devicesControllers.end())
        {
            auto r = _getReply.add_reply();
            r->CopyFrom(*devInterface->second->getData());
            // TODO Error handling for dev not found
        }
    }

    return &_getReply;
}