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

using namespace egel;

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
using icu::StringPiece;

const auto MAX_MESSAGE_LENGTH = 128*1024*1024;

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
//std::cout << "call received: " << s << "\n";
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
//std::cout << "call send: " << t << "\n";
        out->set_text(unicode_to_string(t));
        return Status::OK;
    }
    
    virtual Status EgelDependencies(ServerContext* context, const EgelTexts* in, EgelText* out) override {
        auto texts = in->texts();
        for (auto &t : texts) {
//std::cout << "received: " << t << "\n";
            auto s = unicode_from_string(t);
            auto o = machine()->assemble(s);
            machine()->overwrite(o);
        }
//std::cout << "machine state: \n";
//machine()->render(std::cout);
        out->set_text("none");
        return Status::OK;
    }

    virtual Status EgelNodeInfo(ServerContext* context, const EgelText* in, EgelText* out) override {
        out->set_text("none");
        return Status::OK;
    }

    virtual Status EgelImport(ServerContext* context, const EgelText* in, EgelText* out) override {
        auto m = unicode_from_string(in->text());
// std::cout << "import : " << m << "\n";
        machine()->eval_module(m);
// std::cout << "machine state: \n";
// machine()->render(std::cout);
        out->set_text("none");
        return Status::OK;
    }

    void run(VM* m, const std::string& server_address) {
        set_machine(m);
        ServerBuilder builder;
        builder.SetMaxReceiveMessageSize(MAX_MESSAGE_LENGTH);
        builder.SetMaxSendMessageSize(MAX_MESSAGE_LENGTH);
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
    EgelRpcConnection() {
    }

    EgelRpcConnection(std::shared_ptr<Channel> channel)
      : _stub(EgelRpc::NewStub(channel)) {}

    EgelRpcConnection(const std::string& server_address) {
        grpc::ChannelArguments channel_args;
        channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, MAX_MESSAGE_LENGTH);
        channel_args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, MAX_MESSAGE_LENGTH);

        auto channel = grpc::CreateCustomChannel(server_address, grpc::InsecureChannelCredentials(), channel_args);
        _stub = EgelRpc::NewStub(channel);
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

    EgelRpcReturn EgelDependencies(const std::vector<std::string>& data) {
        EgelTexts in;
        EgelText out;

        for (const auto& d : data) {
            in.add_texts(d);
        }

        ClientContext context;

        Status status = _stub->EgelDependencies(&context, in, &out);

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
    std::unique_ptr<EgelRpc::Stub> _stub = nullptr;
};

const icu::UnicodeString STRING_SYSTEM = "System";

//## System::rpc_connection - a connection to an egel rpc server
class RpcConnection : public Opaque {
public:
    OPAQUE_PREAMBLE(VM_SUB_EGO, RpcConnection, STRING_SYSTEM, "rpc_connection");

    RpcConnection(VM* m, const std::string& address) : RpcConnection(m) {
        _address = address;
        _connection = new EgelRpcConnection(address);
    }

    ~RpcConnection() {
        delete _connection;
    }

    static VMObjectPtr create(VM* m, const std::string& address) {
        return VMObjectPtr(new RpcConnection(m, address));
    }

    int compare(const VMObjectPtr &o) {
        return -1; // XXX for now
    }

    VMObjectPtr call(const VMObjectPtr& o) {
        // send the dependencies
        auto oo = machine()->dependencies(o);
        std::vector<std::string> ss;
        for (auto &o : oo) {
            auto s = machine()->disassemble(o);
            ss.push_back(unicode_to_string(s));
        }
        auto r = _connection->EgelDependencies(ss);

        if (!r.okay) {
            throw machine()->create_text("call failed");
        }

        // do the call
        auto s = machine()->serialize(o);
        r = _connection->EgelCall(unicode_to_string(s));

        if (!r.okay) {
            throw machine()->create_text("call failed");
        }

        auto q = machine()->deserialize(unicode_from_string(r.text));
        if (r.exception) {
            throw q;
        } else {
            return q;
        }
    }

    VMObjectPtr import_(const icu::UnicodeString& s) {
        auto r = _connection->EgelImport(unicode_to_string(s));

        if (!r.okay) {
            throw machine()->create_text("call failed");
        } else {
            return machine()->create_none();
        }
    }

private:
    std::string _address;
    EgelRpcConnection* _connection = nullptr;
};

class RpcServer : public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RpcServer, STRING_SYSTEM, "rpc_server");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = unicode_to_string(machine()->get_text(arg0));
            EgelRpcImpl erpc;
            erpc.run(machine(),s);
            return machine()->create_none();
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};


class RpcClient: public Monadic {
public:
    MONADIC_PREAMBLE(VM_SUB_EGO, RpcClient, STRING_SYSTEM, "rpc_client");

    VMObjectPtr apply(const VMObjectPtr& arg0) const override {
        if (machine()->is_text(arg0)) {
            auto s = unicode_to_string(machine()->get_text(arg0));
            return RpcConnection::create(machine(), s);
        } else {
            throw machine()->bad_args(this, arg0);
        }
    }
};

class RpcCall: public Binary {
public:
    BINARY_PREAMBLE(VM_SUB_EGO, RpcCall, STRING_SYSTEM, "rpc_call");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "System::rpc_connection") {
            auto c = RpcConnection::cast(arg0);
            return c->call(arg1);
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

class RpcImport: public Dyadic {
public:
    DYADIC_PREAMBLE(VM_SUB_EGO, RpcImport, STRING_SYSTEM, "rpc_import");

    VMObjectPtr apply(const VMObjectPtr& arg0, const VMObjectPtr& arg1) const override {
        auto m = machine();
        if (m->is_opaque(arg0) && m->symbol(arg0) == "System::rpc_connection" && m->is_text(arg1)) {
            auto c = RpcConnection::cast(arg0);
            return c->import_(m->get_text(arg1));
        } else {
            throw machine()->bad_args(this, arg0, arg1);
        }
    }
};

extern "C" std::vector<icu::UnicodeString> egel_imports() {
    return std::vector<icu::UnicodeString>();
};

extern "C" std::vector<VMObjectPtr> egel_exports(VM* vm) {
    std::vector<VMObjectPtr> oo;

    oo.push_back(RpcServer::create(vm));
    oo.push_back(RpcClient::create(vm));
    oo.push_back(RpcCall::create(vm));
    oo.push_back(RpcImport::create(vm));

    return oo;
};
