#ifndef ENGINE_GRPC_SERVER_H
#define ENGINE_GRPC_SERVER_H

#include <grpcpp/grpcpp.h>

#include <dummy.grpc.pb.h>

using dummy::Dummy;
using dummy::DummyRequest;
using dummy::DummyReply;

// Logic and data behind the server's behavior.
class DummyServiceImpl final : public Dummy::Service
{
    grpc::Status dummy(grpc::ServerContext * context, const DummyRequest * request, DummyReply * reply) override;
};

class EngineGrpcServer
{
    public:

        EngineGrpcServer();

    private:

    DummyServiceImpl _service;
    std::unique_ptr<grpc::Server> _server;
};

#endif // ENGINE_GRPC_SERVER_H