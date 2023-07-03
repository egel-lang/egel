#pragma once

#include <dlfcn.h>  // XXX: I wish I could get rid of this

#include <memory>
#include <vector>

#include "ast.hpp"
#include "builtin_eval.hpp"
#include "builtin_math.hpp"
#include "builtin_string.hpp"
#include "builtin_process.hpp"
#include "builtin_thread.hpp"
#include "builtin_async.hpp"
#include "builtin_runtime.hpp"
#include "builtin_system.hpp"
#include "constants.hpp"
#include "desugar.hpp"
#include "emit.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "lexical.hpp"
#include "lift.hpp"
#include "runtime.hpp"
#include "semantical.hpp"
#include "syntactical.hpp"
#include "lightning.hpp"

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

enum module_tag_t {
    MODULE_SOURCE,
    MODULE_INTERNAL,
    MODULE_DYNAMIC,
};

class Module {
public:
    Module(const module_tag_t t, const icu::UnicodeString &p,
           const icu::UnicodeString &fn, VM *m)
        : _tag(t), _path(p), _filename(fn), _machine(m) {
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

    icu::UnicodeString get_filename() const {
        return _filename;
    }

    module_tag_t tag() const {
        return _tag;
    }

    VM *machine() const {
        return _machine;
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
    virtual void syntactical(){};

    virtual void declarations(NamespacePtr &env){};

    virtual void semantical(NamespacePtr &env){};

    virtual void desugar(){};

    virtual void lift(){};

    virtual void datagen(VM *m){};

    virtual void codegen(VM *m){};

    virtual void jit(VM *m){};

private:
    module_tag_t _tag;
    OptionsPtr _options;
    icu::UnicodeString _path;
    icu::UnicodeString _filename;
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
    ModuleInternal(const icu::UnicodeString &fn, VM *m, const exports_t handle)
        : Module(MODULE_INTERNAL, fn, fn, m), _handle(handle) {
    }

    ModuleInternal(const ModuleInternal &m)
        : Module(MODULE_INTERNAL, m.get_path(), m.get_filename(), m.machine()),
          _handle(m._handle),
          _imports(m._imports),
          _exports(m._exports) {
    }

    static ModulePtr create(const icu::UnicodeString &fn, VM *m,
                            const exports_t handle) {
        return std::make_shared<ModuleInternal>(fn, m, handle);
    }

    void load() override {
        _imports = QualifiedStrings();
        _values = QualifiedStrings();
        _exports = (*_handle)(machine());
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
            vm->enter_data(o);
        }
    }

    void render(std::ostream &os) const override {
        os << "dynamic module: " << get_filename();
    }

    static bool filetype(const icu::UnicodeString &fn) {
        return fn.endsWith(".ego");
    }

private:
    exports_t _handle;
    QualifiedStrings _imports;
    QualifiedStrings _values;
    VMObjectPtrs _exports;
};

class ModuleDynamic : public Module {
public:
    ModuleDynamic(const icu::UnicodeString &p, const icu::UnicodeString &fn,
                  VM *m)
        : Module(MODULE_DYNAMIC, p, fn, m),
          _handle(0),
          _imports(0),
          _exports(0) {
    }

    ModuleDynamic(const ModuleDynamic &m)
        : Module(MODULE_DYNAMIC, m.get_path(), m.get_filename(), m.machine()),
          _handle(m._handle),
          _imports(m._imports),
          _exports(m._exports) {
        set_options(m.get_options());
    }

    static ModulePtr create(const icu::UnicodeString &p,
                            const icu::UnicodeString &fn, VM *m) {
        return std::make_shared<ModuleDynamic>(p, fn, m);
    }

    void load() override {
        char *error;

        dlerror();

        auto pth = VM::unicode_to_utf8_chars(get_path());  // XXX: leaks?
        _handle = dlopen(pth, RTLD_LAZY | RTLD_GLOBAL);
        if (!_handle) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            err += " on open(" + get_path() + ")";
            throw ErrorIO(err);
        }

        std::vector<icu::UnicodeString> (*egel_imports)();
        egel_imports = (std::vector<icu::UnicodeString>(*)())dlsym(
            _handle, "egel_imports");
        error = dlerror();
        if (error != NULL) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            throw ErrorIO(err);
        }

        std::vector<VMObjectPtr> (*egel_exports)(VM *);
        egel_exports =
            (std::vector<VMObjectPtr>(*)(VM *))dlsym(_handle, "egel_exports");
        error = dlerror();
        if (error != NULL) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            throw ErrorIO(err);
        }

        auto ss = (*egel_imports)();

        QualifiedStrings ii;
        Position p(get_path(), 1, 1);
        for (auto &s : ss) {
            ii.push_back(QualifiedString(p, s));
        }

        _imports = ii;
        _exports = (*egel_exports)(machine());
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
            vm->enter_data(o);
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
    QualifiedStrings _imports;
    QualifiedStrings _values;
    VMObjectPtrs _exports;
};

class ModuleSource : public Module {
public:
    ModuleSource(const icu::UnicodeString &path, const icu::UnicodeString &fn,
                 VM *m)
        : Module(MODULE_SOURCE, path, fn, m), _source(""), _ast(0) {
    }

    ModuleSource(const ModuleSource &m)
        : Module(MODULE_SOURCE, m.get_path(), m.get_filename(), m.machine()),
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
                auto [p, n, f] = AstDeclValue::split(a);
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
        TokenReaderPtr tt = tokenize_from_reader(r);

        if (get_options()->only_tokenize()) {
            while (tt->look().tag() != TOKEN_EOF) {
                std::cout << tt->look() << " ";
                tt->skip();
            };
            std::cout << std::endl;
            exit(EXIT_SUCCESS);
        };

        auto a = parse(tt);

        if (get_options()->only_unparse()) {
            std::cout << a << std::endl;
            exit(EXIT_SUCCESS);
        };

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

    void lift() override {
        _ast = egel::lift(_ast);

        if (get_options()->only_lift()) {
            std::cout << _ast << std::endl;
            exit(EXIT_SUCCESS);
        };
    }

    void datagen(VM *vm) override {
        auto oo = egel::emit_data(vm, _ast);
        for (auto &o:oo) {
            _combinators.push_back(o);
        }
    }

    void codegen(VM *vm) override {
        auto oo = egel::emit_code(vm, _ast);
        if (get_options()->only_bytecode()) {
            vm->render(std::cout);
            exit(EXIT_SUCCESS);
        };
        for (auto &o:oo) {
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
        // next code is slightly conceptually dirty, needs a better solution
        auto sys = ModuleInternal::create("internal", vm, &builtin_system);
        auto mth = ModuleInternal::create("internal", vm, &builtin_math);
        auto str = ModuleInternal::create("internal", vm, &builtin_string);
        auto run = ModuleInternal::create("internal", vm, &builtin_runtime);
        auto thd = ModuleInternal::create("internal", vm, &builtin_thread);
        auto prc = ModuleInternal::create("internal", vm, &builtin_process);
        auto evl = ModuleInternal::create("internal", vm, &builtin_eval);
        auto asy = ModuleInternal::create("internal", vm, &builtin_async);
        sys->load();
        mth->load();
        str->load();
        run->load();
        thd->load();
        prc->load();
        evl->load();
        asy->load();
        _loading.push_back(sys);
        _loading.push_back(mth);
        _loading.push_back(str);
        _loading.push_back(run);
        _loading.push_back(thd);
        _loading.push_back(prc);
        _loading.push_back(evl);
        _loading.push_back(asy);
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
            m->lift();
        }
        for (auto &m : _loading) {
            m->datagen(_machine);
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
