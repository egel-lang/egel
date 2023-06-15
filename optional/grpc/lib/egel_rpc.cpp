#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>


#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <egel.pb.h>
#include <egel.grpc.pb.h>

#include <egel/runtime.hpp> // compile against an installed egel

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
using egel_rpc::EgelTexts;
using egel_rpc::EgelResult;
using egel_rpc::EgelRpc;

#define LIBRARY_VERSION_MAJOR "0"
#define LIBRARY_VERSION_MINOR "0"
#define LIBRARY_VERSION_PATCH "1"

using icu::UnicodeString;
using icu::StringPiece;;

inline std::string unicode_to_string(const UnicodeString s) {
    std::string utf8;
    s.toUTF8String(utf8);
    return utf8;
};

inline UnicodeString unicode_from_string(const std::string& s) {
    StringPiece sp(s);
    return UnicodeString::fromUTF8(sp);
};

inline char* char_from_string(const std::string& s) {
    char *cstr = new char[s.length() + 1];
    std::strcpy(cstr, s.c_str());
    return cstr;
};

class EgelRpcImpl final : public egel_rpc::EgelRpc::Service {
public:

    void set_machine(VM* vm) {
        _machine = vm;
    }

    VM* machine() {
        return _machine;
    }

    virtual Status EgelCall(ServerContext* context, const EgelText* in, EgelResult* out) override {
        auto s = unicode_from_string(in->text());
        auto o = machine()->deserialize(s);
        auto n = machine()->create_none();
        VMObjectPtrs thunk;
        thunk.push_back(o);
        thunk.push_back(n);
        auto app = machine()->create_array(thunk);
        auto r = machine()->reduce(app);

        if (r.exception) {
            out->set_exception(true);
        } else {
            out->set_exception(false);
        }
        auto t = machine()->serialize(r.result);
        out->set_text(unicode_to_string(t));
        return Status::OK;
    }
    
    virtual Status EgelBundle(ServerContext* context, const EgelTexts* in, EgelText* out) override {
        auto texts = in->texts();
        for (auto &t : texts) {
            auto s = unicode_from_string(t);
            auto o = machine()->assemble(s);
        }
        out->set_text("none");
        return Status::OK;
    }

    virtual Status EgelNodeInfo(ServerContext* context, const EgelText* in, EgelText* out) override {
        out->set_text("none");
        return Status::OK;
    }

    virtual Status EgelImport(ServerContext* context, const EgelText* in, EgelText* out) override {
        auto m = unicode_from_string(in->text());
        machine()->eval_module(m);
        out->set_text("none");
        return Status::OK;
    }

    void run_server(const std::string& server_address) {
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<Server> server(builder.BuildAndStart());
//        std::cout << "Server listening on " << server_address << std::endl;
        server->Wait();
    };
private:
    VM*         _machine;
};

struct EgelRpcReturn {
    std::string text;
    bool exception;
    bool okay;
};

class EgelRpcConnection {
public:
    EgelRpcConnection(std::shared_ptr<Channel> channel)
      : _stub(EgelRpc::NewStub(channel)) {}

    EgelRpcConnection(const std::string& server_address) :
        EgelRpcConnection(grpc::CreateChannel(server_address,
                                            grpc::InsecureChannelCredentials())) {
    }

    EgelRpcReturn EgelCall(const std::string& data) {
        EgelText in;
        EgelResult out;

        in.set_text(data);
        ClientContext context;

        Status status = _stub->EgelCall(&context, in, &out);

        if (status.ok()) {
            EgelRpcReturn r;
            r.okay = true;
            r.exception = out.exception();
            r.text = out.text();
            return r;
        } else {
            EgelRpcReturn r;
            r.okay = false;;
            return r;
        }
    }

    EgelRpcReturn EgelBundle(const std::vector<std::string>& data) {
        EgelTexts in;
        EgelText out;

        for (const auto& d : data) {
            in.add_texts(d);
        }

        ClientContext context;

        Status status = _stub->EgelBundle(&context, in, &out);

        if (status.ok()) {
            EgelRpcReturn r;
            r.okay = true;
            r.exception = false;
            r.text = out.text();
            return r;
        } else {
            EgelRpcReturn r;
            r.okay = false;;
            return r;
        }
    }

    EgelRpcReturn EgelImport(const std::string& data) {
        EgelText in;
        EgelText out;

        in.set_text(data);
        ClientContext context;

        Status status = _stub->EgelImport(&context, in, &out);

        if (status.ok()) {
            EgelRpcReturn r;
            r.okay = true;
            r.exception = false;
            r.text = out.text();
            return r;
        } else {
            EgelRpcReturn r;
            r.okay = false;;
            return r;
        }
    }

    EgelRpcReturn EgelInfo(const std::string& s) {
        EgelText in;
        EgelText out;

        in.set_text(s);
        ClientContext context;

        Status status = _stub->EgelNodeInfo(&context, in, &out);

        if (status.ok()) {
            EgelRpcReturn r;
            r.okay = true;
            r.exception = false;
            r.text = out.text();
            return r;
        } else {
            EgelRpcReturn r;
            r.okay = false;;
            return r;
        }
    }

private:
    std::unique_ptr<EgelRpc::Stub> _stub;
};
