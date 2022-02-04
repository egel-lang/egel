#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>


#include <egel/runtime.hpp> // build against an installed egel
#include "utils.hpp"        // same as egel's but only the runtime should be included

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using EgelRpc::EgelText;

#define LIBRARY_VERSION_MAJOR "0"
#define LIBRARY_VERSION_MINOR "0"
#define LIBRARY_VERSION_PATCH "1"

class EgelRpcImpl final : public EgelRpc::Service {
public:
    explicit EgelRpcImpl(const std::string& db) {
        egel_rpc::ParseDb(db, &feature_list_);
    }

    Status EgelCall(ServerContext* context, const EgelText* in, EgelText* out) override {
        auto s = in->text();
        return Status::OK;
    }

    Status EgelStream(ServerContext* context, ServerReaderWriter<EgelText, EgelText>* stream) override {
        EgelText text;
        while (stream->Read(&text)) {
            stream->Write(text); // XXX
        }
    }

    return Status::OK;
  }

private:
  std::vector<Feature> feature_list_;
};

void RunServer(const std::string& db_path) {
  std::string server_address("0.0.0.0:50051");
  RouteGuideImpl service(db_path);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
};
