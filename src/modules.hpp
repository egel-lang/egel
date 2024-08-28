#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <memory>
#include <vector>

#include "ast.hpp"
#include "builtin_async.hpp"
#include "builtin_dict.hpp"
#include "builtin_eval.hpp"
#include "builtin_ffi.hpp"
#include "builtin_math.hpp"
#include "builtin_process.hpp"
#include "builtin_regex.hpp"
#include "builtin_runtime.hpp"
#include "builtin_string.hpp"
#include "builtin_system.hpp"
#include "constants.hpp"
#include "desugar.hpp"
#include "emit.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "lexical.hpp"
#include "lift.hpp"
#include "lightning.hpp"
#include "runtime.hpp"
#include "semantical.hpp"
#include "syntactical.hpp"

namespace egel {
// convenience
inline icu::UnicodeString first(const icu::UnicodeString &s) {
    auto d = s;
    auto i = d.indexOf(STRING_DCOLON);
    return d.remove(i, d.length());
}

inline icu::UnicodeString second(const icu::UnicodeString &s) {
    auto d = s;
    auto i = d.indexOf(STRING_DCOLON);
    return d.remove(0, i + 2);
}

class Module;
using ModulePtr = std::shared_ptr<Module>;

class ModuleManager;
using ModuleManagerPtr = std::shared_ptr<ModuleManager>;

// modules may define imports or values
class QualifiedString {
public:
    QualifiedString() : _position(Position()), _string("") {
    }

    QualifiedString(const Position &p, const icu::UnicodeString &s)
        : _position(p), _string(s) {
    }

    /*
        QualifiedString(const QualifiedString& i):
            _position(i._position), _string(i._string) {
        }
    */

    Position position() const {
        return _position;
    }

    icu::UnicodeString string() const {
        return _string;
    }

private:
    Position _position;
    icu::UnicodeString _string;
};

using QualifiedStrings = std::vector<QualifiedString>;

class Module {
public:
    Module(const icu::UnicodeString &p,
           const icu::UnicodeString &fn, VM *m)
        : _path(p), _filename(fn), _machine(m) {
    }

    virtual ~Module() {  // keep the compiler happy
    }

    void set_options(const OptionsPtr &o) {
        _options = o;
    }

    OptionsPtr get_options() const {
        return _options;
    }

    icu::UnicodeString get_path() const {
        return _path;
    }

    void set_path(const icu::UnicodeString &p) {
        _path = p;
    }

    icu::UnicodeString get_filename() const {
        return _filename;
    }

    void set_filename(const icu::UnicodeString &fn) {
        _filename = fn;
    }

    VM *machine() const {
        return _machine;
    }

    icu::UnicodeString docstring() const {
        return _docstring;
    }

    void set_docstring(const icu::UnicodeString &doc) {
        _docstring = doc;
    }

    virtual void load() = 0;

    virtual void unload() = 0;

    virtual QualifiedStrings imports() = 0;

    virtual QualifiedStrings values() = 0;

    virtual VMObjectPtrs exports() = 0;

    virtual void render(std::ostream &os) const = 0;

    friend std::ostream &operator<<(std::ostream &os, const ModulePtr &m) {
        m->render(os);
        return os;
    }

public:
    // this is why OO is sometimes bad. but I am lazy at the moment.
    // pretend every module is equivalent to an Egel source file.
    virtual void syntactical() {};

    virtual void declarations(NamespacePtr &env) {};

    virtual void semantical(NamespacePtr &env) {};

    virtual void desugar() {};

    virtual void lift(VM *m) {};

    virtual void datagen(VM *m) {};

    virtual void codegen(VM *m) {};

    virtual void jit(VM *m) {};

private:
    OptionsPtr _options;
    icu::UnicodeString _path;
    icu::UnicodeString _filename;
    icu::UnicodeString _docstring;
    VM *_machine;
};

class VMModule;
using VMModulePtr = std::shared_ptr<VMModule>;

class VMModule : public Opaque {
public:
    VMModule(VM *vm, ModulePtr p)
        : Opaque(VM_SUB_MODULE, vm, STRING_SYSTEM, "module"), _value(p) {
    }

    VMModule(const VMModule &m) : VMModule(m.machine(), m.value()) {
    }

    ~VMModule() {
    }

    icu::UnicodeString docstring() const {
        return _value->docstring();
    }

    void set_docstring(const icu::UnicodeString &doc) {
        _value->set_docstring(doc);
    }

    static VMObjectPtr create(VM *vm, ModulePtr p) {
        return std::make_shared<VMModule>(vm, p);
    }

    ModulePtr value() const {
        return _value;
    }

    int compare(const VMObjectPtr &o) override {
        if (is_module(o)) {
            auto m = module_cast(o);
            auto n0 = value()->get_filename();
            auto n1 = m->value()->get_filename();
            if (n0 < n1) {
                return -1;
            } else if (n1 < n0) {
                return 1;
            } else {
                return 0;
            }
        } else {
            return -1;
        }
    }

    static bool is_module(const VMObjectPtr &o) {
        return (o->subtag() == VM_SUB_MODULE);
    }

    static VMModulePtr module_cast(const VMObjectPtr &o) {
        return std::static_pointer_cast<VMModule>(o);
    }

    VMObjectPtr name() {
        auto s = value()->get_filename();
        return machine()->create_text(s);
    }

    VMObjectPtr path() {
        auto s = value()->get_path();
        return machine()->create_text(s);
    }

    VMObjectPtr imports() {
        auto ii = value()->imports();
        VMObjectPtrs pp;
        for (auto &i : ii) {
            auto s = i.string();
            auto p = machine()->create_text(s);
            pp.push_back(p);
        }
        return machine()->to_list(pp);
    }

    VMObjectPtr exports() {
        auto ee = value()->exports();
        return machine()->to_list(ee);
    }

    VMObjectPtr values() {
        auto vv = value()->values();
        VMObjectPtrs pp;
        for (auto &v : vv) {
            auto s = v.string();
            auto p = machine()->create_text(s);
            pp.push_back(p);
        }
        return machine()->to_list(pp);
    }

private:
    ModulePtr _value;
};

using exports_t = std::vector<VMObjectPtr> (*)(VM *);

class ModuleInternal : public Module {
public:
    ModuleInternal(VM *m, std::shared_ptr<CModule> &cm)
        : Module("", "", m), _module(cm) {
    }

    ModuleInternal(const ModuleInternal &m)
        : Module(m.get_path(), m.get_filename(), m.machine()),
          _module(m._module),
          _exports(m._exports) {
    }

    static ModulePtr create(VM *m, std::shared_ptr<CModule> cm) {
        return std::make_shared<ModuleInternal>(m, cm);
    }

    void load() override {
        set_filename(_module->name());
        set_path(_module->path());
        set_docstring(_module->docstring());
        _imports = QualifiedStrings();
        _values = QualifiedStrings();
        _exports = _module->exports(machine()); //note: this also declares
    }

    void unload() override {
    }

    QualifiedStrings imports() override {
        return _imports;
    }

    QualifiedStrings values() override {
        return _values;
    }

    VMObjectPtrs exports() override {
        return _exports;
    }

    void declarations(NamespacePtr &env) override {
        for (auto &o : _exports) {
            if (machine()->is_combinator(o)) {
                auto sym = VMObjectCombinator::symbol(o);
                auto s = machine()->get_combinator_string(sym);

                UnicodeStrings nn;
                nn.push_back(first(s));
                auto n = second(s);
                egel::declare(env, nn, n, s);
            }
        }
    }

    void codegen(VM *vm) override {
        for (auto &o : _exports) {
            vm->define_data(o);
        }
    }

    void render(std::ostream &os) const override {
        os << "internal module: " << get_filename();
    }

    static bool filetype(const icu::UnicodeString &fn) {
        return false;
    }

private:
    std::shared_ptr<CModule> _module;
    QualifiedStrings _imports;
    QualifiedStrings _values;
    VMObjectPtrs _exports;
};

class ModuleDynamic : public Module {
public:
    ModuleDynamic(const icu::UnicodeString &p, const icu::UnicodeString &fn,
                  VM *m)
        : Module(p, fn, m), _module(nullptr) {
    }

    ModuleDynamic(const ModuleDynamic &m)
        : Module(m.get_path(), m.get_filename(), m.machine()),
          _module(m._module) {
        set_options(m.get_options());
    }

    static ModulePtr create(const icu::UnicodeString &p,
                            const icu::UnicodeString &fn, VM *m) {
        return std::make_shared<ModuleDynamic>(p, fn, m);
    }

    void load() override {

        // std::cout << "loading: " << get_path() << std::endl; // DEBUG

        auto pth = VM::unicode_to_utf8_chars(get_path());  // XXX: leaks?

#ifdef _WIN32
        _handle = LoadLibrary(pth);
#else
        _handle = dlopen(pth, RTLD_LAZY | RTLD_GLOBAL);
#endif
        if (!_handle) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            err += " on open(" + get_path() + ")";
            throw ErrorIO(err);
        }

#ifdef _WIN32
        auto fp = (CModule*(*)())GetProcAddress((HMODULE)_handle, "egel_module");
        if (!egel_module) {
            icu::UnicodeString err = "dynamic load error: ";
            err += std::to_string(GetLastError());
            throw ErrorIO(err);
        }
#else
        auto fp = (CModule*(*)())dlsym(_handle, "egel_module");
        const char* error = dlerror();
        if (error != NULL) {
            icu::UnicodeString err = "dynamic load error: ";
            err += error;
            throw ErrorIO(err);
        }
#endif
        _module = fp();

        set_filename(_module->name());
        set_docstring(_module->docstring());

        auto ss = _module->imports();
        QualifiedStrings ii;
        Position p(get_path(), 1, 1);
        for (auto &s : ss) {
            ii.push_back(QualifiedString(p, s));
        }

        _imports = ii;
        _exports = _module->exports(machine());
    }

    void unload() override {
        dlclose(_handle);
    }

    QualifiedStrings imports() override {
        return _imports;
    }

    QualifiedStrings values() override {
        return _values;
    }

    VMObjectPtrs exports() override {
        return _exports;
    }

    void declarations(NamespacePtr &env) override {
        for (auto &o : _exports) {
            if (machine()->is_combinator(o)) {
                auto sym = VMObjectCombinator::symbol(o);
                auto s = machine()->get_combinator_string(sym);

                UnicodeStrings nn;
                nn.push_back(first(s));
                auto n = second(s);
                egel::declare(env, nn, n, s);
            }
        }
    }

    void codegen(VM *vm) override {
        for (auto &o : _exports) {
            vm->define_data(o);
        }
    }

    void render(std::ostream &os) const override {
        os << "dynamic module: " << get_filename();
    }

    static bool filetype(const icu::UnicodeString &fn) {
        return fn.endsWith(".ego");
    }

private:
    void *_handle;
    CModule *_module;
    QualifiedStrings _imports;
    QualifiedStrings _values;
    VMObjectPtrs _exports;
};

class ModuleSource : public Module {
public:
    ModuleSource(const icu::UnicodeString &path, const icu::UnicodeString &fn,
                 VM *m)
        : Module(path, fn, m), _source(""), _ast(0) {
    }

    ModuleSource(const ModuleSource &m)
        : Module(m.get_path(), m.get_filename(), m.machine()),
          _source(m._source),
          _ast(m._ast) {
        set_options(m.get_options());
    }

    static ModulePtr create(const icu::UnicodeString &path,
                            const icu::UnicodeString &fn, VM *m) {
        return std::make_shared<ModuleSource>(path, fn, m);
    }

    void load() override {
        if (VM::file_exists(get_path())) {
            _source = VM::read_utf8_file(get_path());
        } else {
            throw ErrorIO("module " + get_path() + " not found");
        };
    }

    void unload() override {
    }

    QualifiedStrings imports() override {
        auto aa = egel::imports(_ast);
        auto ii = QualifiedStrings();
        for (auto a : aa) {
            if (a->tag() == AST_DIRECT_IMPORT) {
                auto [p, s] = AstDirectImport::split(a);
                ii.push_back(QualifiedString(p, VM::unicode_to_text(s)));
            }
        }
        return ii;
    }

    QualifiedStrings values() override {
        auto aa = egel::values(_ast);
        auto ii = QualifiedStrings();
        for (auto a : aa) {
            if (a->tag() == AST_DECL_VALUE) {
                auto [p, n, d, f] = AstDeclValue::split(a);
                ii.push_back(QualifiedString(p, n->to_text()));
            }
        }
        return ii;
    }

    VMObjectPtrs exports() override {
        return _combinators;
    }

    void syntactical() override {
        StringCharReader r = StringCharReader(get_filename(), _source);
        Tokens tt = tokenize_from_reader(r);
        sanitize(tt);

        if (get_options()->only_tokenize()) {
            while (tt.look().tag() != TOKEN_EOF) {
                std::cout << tt.look() << " ";
                tt.skip();
            };
            std::cout << std::endl;
            exit(EXIT_SUCCESS);
        };

        auto a = parse(tt);

        if (get_options()->only_unparse()) {
            std::cout << a << std::endl;
            exit(EXIT_SUCCESS);
        };

        auto doc = egel::visit_docstring(a);
        auto [p,d] = AstDocstring::split(doc);
        set_docstring(d);
        _source = "";
        _ast = a;
    }

    void declarations(NamespacePtr &env) override {
        declare(env, _ast);
    }

    void semantical(NamespacePtr &env) override {
        _ast = egel::identify(env, _ast);

        if (get_options()->only_semantical()) {
#ifdef DEBUG
            std::cout << env << std::endl;
#endif
            std::cout << _ast << std::endl;
            exit(EXIT_SUCCESS);
        };
    }

    void desugar() override {
        _ast = egel::desugar(_ast);
        if (get_options()->only_desugar()) {
            std::cout << _ast << std::endl;
            exit(EXIT_SUCCESS);
        };
    }

    void lift(VM *m) override {
        _ast = egel::lift(_ast, m);

        if (get_options()->only_lift()) {
            std::cout << _ast << std::endl;
            exit(EXIT_SUCCESS);
        };
    }

    void datagen(VM *vm) override {
        auto oo = egel::emit_data(vm, _ast);
        for (auto &o : oo) {
            _combinators.push_back(o);
        }
    }

    void codegen(VM *vm) override {
        auto oo = egel::emit_code(vm, _ast);
        if (get_options()->only_bytecode()) {
            vm->render(std::cout);
            exit(EXIT_SUCCESS);
        };
        for (auto &o : oo) {
            _combinators.push_back(o);
        }
    }

    void jit(VM *vm) override {
        emit_jit(vm, _combinators);
    }

    void render(std::ostream &os) const override {
        os << "source module " << get_filename();
    }

    static bool filetype(const icu::UnicodeString &fn) {
        return fn.endsWith(".eg");
    }

private:
    icu::UnicodeString _source;
    ptr<Ast> _ast;
    std::vector<VMObjectPtr> _combinators;
};

class ModuleEgg : public Module {
public:
    ModuleEgg(const icu::UnicodeString &path, const icu::UnicodeString &fn,
              VM *m)
        : Module(path, fn, m), _source(""), _ast(0) {
    }

    ModuleEgg(const ModuleEgg &m)
        : Module(m.get_path(), m.get_filename(), m.machine()),
          _source(m._source),
          _ast(m._ast) {
        set_options(m.get_options());
    }

    static ModulePtr create(const icu::UnicodeString &path,
                            const icu::UnicodeString &fn, VM *m) {
        return std::make_shared<ModuleEgg>(path, fn, m);
    }

    void load() override {
        if (VM::file_exists(get_path())) {
            _source = VM::read_utf8_file(get_path());
        } else {
            throw ErrorIO("egg " + get_path() + " not found");
        };
    }

    void unload() override {
    }

    QualifiedStrings imports() override {
        auto aa = egel::imports(_ast);
        auto ii = QualifiedStrings();
        for (auto a : aa) {
            if (a->tag() == AST_DIRECT_IMPORT) {
                auto [p, s] = AstDirectImport::split(a);
                ii.push_back(QualifiedString(p, VM::unicode_to_text(s)));
            }
        }
        return ii;
    }

    QualifiedStrings values() override {
        auto aa = egel::values(_ast);
        auto ii = QualifiedStrings();
        for (auto a : aa) {
            if (a->tag() == AST_DECL_VALUE) {
                auto [p, n, d, f] = AstDeclValue::split(a);
                ii.push_back(QualifiedString(p, n->to_text()));
            }
        }
        return ii;
    }

    VMObjectPtrs exports() override {
        return _combinators;
    }

    void syntactical() override {
        StringCharReader r = StringCharReader(get_filename(), _source);
        Tokens tt = tokenize_from_egg_reader(r);
        sanitize(tt);

        if (get_options()->only_tokenize()) {
            while (tt.look().tag() != TOKEN_EOF) {
                std::cout << tt.look() << " ";
                tt.skip();
            };
            std::cout << std::endl;
            exit(EXIT_SUCCESS);
        };

        auto a = parse(tt);

        if (get_options()->only_unparse()) {
            std::cout << a << std::endl;
            exit(EXIT_SUCCESS);
        };

        auto doc = egel::visit_docstring(a);
        auto [p,d] = AstDocstring::split(doc);
        set_docstring(d);
        _source = "";
        _ast = a;
    }

    void declarations(NamespacePtr &env) override {
        declare(env, _ast);
    }

    void semantical(NamespacePtr &env) override {
        _ast = egel::identify(env, _ast);

        if (get_options()->only_semantical()) {
#ifdef DEBUG
            std::cout << env << std::endl;
#endif
            std::cout << _ast << std::endl;
            exit(EXIT_SUCCESS);
        };
    }

    void desugar() override {
        _ast = egel::desugar(_ast);
        if (get_options()->only_desugar()) {
            std::cout << _ast << std::endl;
            exit(EXIT_SUCCESS);
        };
    }

    void lift(VM *m) override {
        _ast = egel::lift(_ast, m);

        if (get_options()->only_lift()) {
            std::cout << _ast << std::endl;
            exit(EXIT_SUCCESS);
        };
    }

    void datagen(VM *vm) override {
        auto oo = egel::emit_data(vm, _ast);
        for (auto &o : oo) {
            _combinators.push_back(o);
        }
    }

    void codegen(VM *vm) override {
        auto oo = egel::emit_code(vm, _ast);
        if (get_options()->only_bytecode()) {
            vm->render(std::cout);
            exit(EXIT_SUCCESS);
        };
        for (auto &o : oo) {
            _combinators.push_back(o);
        }
    }

    void jit(VM *vm) override {
        emit_jit(vm, _combinators);
    }

    void render(std::ostream &os) const override {
        os << "source module " << get_filename();
    }

    static bool filetype(const icu::UnicodeString &fn) {
        return fn.endsWith(".egg");
    }

private:
    icu::UnicodeString _source;
    ptr<Ast> _ast;
    std::vector<VMObjectPtr> _combinators;
};

using ModulePtrs = std::vector<ModulePtr>;

class ModuleManager {
public:
    ModuleManager() {
    }

    ModuleManager(const ModuleManager &mm)
        : _options(mm._options),
          _machine(mm._machine),
          _environment(mm._environment),
          _modules(mm._modules),
          _loading(mm._loading) {
    }

    static ModuleManagerPtr create() {
        return std::make_shared<ModuleManager>();
    }

    void init(const OptionsPtr &oo, VM *vm) {
        NamespacePtr env = Namespace::create();
        set_options(oo);
        set_machine(vm);
        set_environment(env);
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<TimeModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<SystemModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<MathModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<StringModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<RuntimeModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<ProcessModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<EvalModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<AsyncModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<DictModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<RegexModule>()));
        _loading.push_back(
            ModuleInternal::create(vm, std::make_shared<FFIModule>()));
        for (const auto &m : _loading) {
            m->load();
        }
        process();
        flush();
    }

    void set_options(const OptionsPtr &oo) {
        _options = oo;
    }

    OptionsPtr get_options() const {
        return _options;
    }

    void set_machine(VM *vm) {
        _machine = vm;
    }

    VM *machine() const {
        return _machine;
    }

    void set_environment(const NamespacePtr &env) {
        _environment = env;
    }

    NamespacePtr get_environment() const {
        return _environment;
    }

    ModulePtrs get_modules() const {
        return _modules;
    }

    // implements incremental loading for interactive mode
    void load(const Position &p, const icu::UnicodeString &fn) {
        preload(p, fn);
        _loading[0]->set_options(_options);
        transitive_closure();
        reverse();  // XXX: why was this again?
        process();
        flush();
    }

    QualifiedStrings values() {
        QualifiedStrings ss;
        for (auto &m : _modules) {
            auto vv = m->values();
            for (auto &s : vv) {
                ss.push_back(s);
            }
        }
        return ss;
    }

    friend std::ostream &operator<<(std::ostream &os, const ModuleManager &mm) {
        for (auto m : mm._modules) {
            os << m << std::endl;
        }
        return os;
    }

protected:
    icu::UnicodeString search(const UnicodeStrings &path,
                              const icu::UnicodeString &fn) {
        auto fn_here =
            VM::path_absolute(fn);  // XXX: shouldn't this be in path?
        if (VM::file_exists(fn_here)) return fn_here;
        for (auto p : path) {
            auto fn0 = VM::path_combine(p, fn);
            auto fn1 = VM::path_absolute(fn0);
            if (VM::file_exists(fn1)) return fn1;
        };
        return "";
    }

    bool already_loaded(const icu::UnicodeString &fn) {
        for (auto &m : _modules) {
            if (m->get_path().compare(fn) == 0) return true;
        }
        for (auto &m : _loading) {
            if (m->get_path().compare(fn) == 0) return true;
        }
        return false;
    }

    void preload(const Position &p, const icu::UnicodeString &fn) {
        if (fn[0] == '!') {
            // not implemented yet
        } else {
            auto find = search(get_options()->get_include_path(), fn);

            if (find.compare("") == 0) {
                throw ErrorIO(p, "file \"" + fn + "\" not found");
            }
            if (!already_loaded(find)) {
                ModulePtr m = nullptr;
                if (ModuleSource::filetype(fn)) {
                    m = ModuleSource::create(find, fn, _machine);
                } else if (ModuleEgg::filetype(fn)) {
                    m = ModuleEgg::create(find, fn, _machine);
                } else if (ModuleDynamic::filetype(fn)) {
                    m = ModuleDynamic::create(find, fn, _machine);
                } else {
                    throw ErrorIO(p, "file \"" + fn + "\" has wrong extension");
                }
                m->set_options(Options::create());  // XXX check this
                try {
                    m->load();
                } catch (ErrorIO &e) {
                    throw ErrorIO(p, e.message());
                }
                _loading.push_back(m);
            }
        }
    }

    void transitive_closure() {
        size_t n = 0;
        while (n < _loading.size()) {
            auto m = _loading[n];
            m->syntactical();
            auto ii = m->imports();
            for (auto &i : ii) {
                preload(i.position(), i.string());
            }
            n++;
        }
    }

    void reverse() {
        ModulePtrs ll;
        for (int i = _loading.size() - 1; i >= 0; i--) {
            ll.push_back(_loading[i]);
        }
        _loading = ll;
    }

    void process() {
        // DEBUG
        /*
                for (auto &m : _loading) {
                    std::cout << "loading loop: " << m->get_path() << std::endl;
                }
        */
        for (auto &m : _loading) {
            m->declarations(_environment);
        }
        for (auto &m : _loading) {
            m->semantical(_environment);
        }
        for (auto &m : _loading) {
            m->desugar();
        }
        for (auto &m : _loading) {
            m->datagen(_machine);
        }
        for (auto &m : _loading) {
            m->lift(_machine);
        }
        for (auto &m : _loading) {
            m->codegen(_machine);
        }
        for (auto &m : _loading) {
            m->jit(_machine);
        }
    }

    void flush() {
        for (auto &m : _loading) {
            _modules.push_back(m);
        }
        ModulePtrs empty;
        _loading = empty;
    }

private:
    OptionsPtr _options;
    VM *_machine;
    NamespacePtr _environment;
    ModulePtrs _modules;
    ModulePtrs _loading;
};
};  // namespace egel
