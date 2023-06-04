#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>


// #include <src/runtime.hpp> // build against the current distribution

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <egel.pb.h>
#include <egel.grpc.pb.h>

#include <egel/runtime.hpp>


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using egel_rpc::EgelText;
using egel_rpc::EgelRpc;


#define LIBRARY_VERSION_MAJOR "0"
#define LIBRARY_VERSION_MINOR "0"
#define LIBRARY_VERSION_PATCH "1"

class EgelRpcImpl final : public egel_rpc::EgelRpc::Service {
public:
    virtual Status EgelCall(ServerContext* context, const EgelText* in, EgelText* out) override {
        auto s = in->text();
        return Status::OK;
    }

    virtual Status EgelStream(ServerContext* context, ServerReaderWriter<EgelText, EgelText>* stream) override {
        EgelText text;
        while (stream->Read(&text)) {
            stream->Write(text); // XXX
        }
        return Status::OK;
    }

    virtual Status EgelNodeInfo(ServerContext* context, const EgelText* in, EgelText* out) override {
        return Status::OK;
    }

    void run_server(const std::string& server_address) {
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;
        server->Wait();
    };
};

class EgelRpcConnection {
public:
    EgelRpcConnection(std::shared_ptr<Channel> channel)
      : _stub(EgelRpc::NewStub(channel)) {}

    EgelRpcConnection(const std::string& server_address) :
        EgelRpcConnection(grpc::CreateChannel(server_address,
                                            grpc::InsecureChannelCredentials())) {
    }

    std::string EgelCall(const std::string& name) {
        EgelText request;
        request.set_text(name);

        EgelText reply;

        ClientContext context;

        Status status = _stub->EgelCall(&context, request, &reply);

        if (status.ok()) {
          return reply.text();
        } else {
          return ""; // default empty
        }
    }

    std::string EgelInfo(const std::string& name) {
        EgelText request;
        request.set_text(name);

        EgelText reply;

        ClientContext context;

        Status status = _stub->EgelNodeInfo(&context, request, &reply);

        if (status.ok()) {
          return reply.text();
        } else {
          return ""; // default empty
        }
    }

private:
    std::unique_ptr<EgelRpc::Stub> _stub;
};
