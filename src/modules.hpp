#ifndef MODULES_HPP
#define MODULES_HPP

#include <vector>

#include "constants.hpp"

#include "error.hpp"
#include "ast.hpp"
#include "environment.hpp"
#include "lexical.hpp"
#include "syntactical.hpp"
#include "semantical.hpp"
#include "desugar.hpp"
#include "lift.hpp"
#include "runtime.hpp"
#include "emit.hpp"

#include "builtin_system.hpp"
#include "builtin_math.hpp"
#include "builtin_string.hpp"
#include "builtin_runtime.hpp"
#include "builtin_thread.hpp"
#include "builtin_process.hpp"
#include "builtin_eval.hpp"


extern std::vector<VMObjectPtr> builtin_eval(VM* vm); // XXX: forward declaration

// convenience
inline icu::UnicodeString first(const icu::UnicodeString& s) {
    auto d = s;
    auto i = d.indexOf(STRING_DCOLON);
    return d.remove(i, d.length());
}

inline icu::UnicodeString second(const icu::UnicodeString& s) {
    auto d = s;
    auto i = d.indexOf(STRING_DCOLON);
    return d.remove(0, i+2);
}

class Module;
typedef std::shared_ptr<Module> ModulePtr;

class ModuleManager;
typedef std::shared_ptr<ModuleManager> ModuleManagerPtr;


// modules may define imports or values
class QualifiedString {
public:
    QualifiedString():
        _position(Position()), _string("") {
    }

    QualifiedString(const Position& p, const icu::UnicodeString& s):
        _position(p), _string(s) {
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
    Position            _position;
    icu::UnicodeString  _string;
};

typedef std::vector<QualifiedString> QualifiedStrings;

typedef enum {
    MODULE_SOURCE,
    MODULE_INTERNAL,
    MODULE_DYNAMIC,
} module_tag_t;

class Module {
public:
    Module(const module_tag_t t, const icu::UnicodeString& p, const icu::UnicodeString& fn, VM* m):
        _tag(t), _path(p), _filename(fn), _machine(m) {
    }

    virtual ~Module() { // keep the compiler happy
    }

    virtual ModulePtr clone() const = 0;

    void set_options(const OptionsPtr& o) {
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

    VM* machine() const {
        return _machine;
    }

    virtual void load() = 0;

    virtual void unload() = 0;

    virtual QualifiedStrings imports() = 0;

    virtual QualifiedStrings values() = 0;

    virtual VMObjectPtrs exports() = 0;

    virtual void render(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const ModulePtr& m) {
        m->render(os);
        return os;
    }

public:
    // this is why OO is sometimes bad. but I am lazy at the moment.
    // pretend every module is equivalent to an Egel source file.
    virtual void syntactical() {};

    virtual void declarations(NamespacePtr& env) {};

    virtual void semantical(NamespacePtr& env) {};

    virtual void desugar() {};

    virtual void lift() {};

    virtual void datagen(VM* m) {};

    virtual void codegen(VM* m) {};

private:
    module_tag_t        _tag;
    OptionsPtr          _options;
    icu::UnicodeString  _path;
    icu::UnicodeString  _filename;
    VM*                 _machine;
};

class VMModule;
typedef std::shared_ptr<VMModule>  VMModulePtr;

class VMModule: public Opaque {
public:
    VMModule(VM* vm, ModulePtr p)
        : Opaque(VM_SUB_MODULE, vm, STRING_SYSTEM, "module"), _value(p) {
    }

    VMModule(const VMModule& m)
        : VMModule(m.machine(), m.value()) {
    }

    ~VMModule() {
    }

    VMObjectPtr clone() const override {
        return VMObjectPtr(new VMModule(*this));
    }

    ModulePtr value() const {
        return _value;
    }

    int compare(const VMObjectPtr& o) override {
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
    
    static bool is_module(const VMObjectPtr& o) {
        return ((o->tag() == VM_OBJECT_OPAQUE) && (o->subtag() == VM_SUB_MODULE));
    }

    static VMModulePtr module_cast(const VMObjectPtr& o) {
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
        for (auto& i: ii) {
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
        for (auto& v: vv) {
            auto s = v.string();
            auto p = machine()->create_text(s);
            pp.push_back(p);
        }
        return machine()->to_list(pp);
    }

private:
    ModulePtr _value;
};

typedef std::vector<VMObjectPtr> (*exports_t)(VM*);

class ModuleInternal: public Module {
public:
    ModuleInternal(const icu::UnicodeString& fn, VM* m, const exports_t handle):
        Module(MODULE_INTERNAL, fn, fn, m),
            _handle(handle) {
    }

    ModuleInternal(const ModuleInternal& m):
        Module(MODULE_INTERNAL, m.get_path(), m.get_filename(), m.machine()),
            _handle(m._handle), _imports(m._imports), _exports(m._exports) {
    }

    ModulePtr clone() const override {
        return ModulePtr(new ModuleInternal(*this));
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

    void declarations(NamespacePtr& env) override {
        for (auto& o:_exports) {
            if (o->tag() == VM_OBJECT_COMBINATOR) {
                auto sym = VM_OBJECT_COMBINATOR_SYMBOL(o);
                auto s   = machine()->get_combinator_string(sym);

                UnicodeStrings nn;
                nn.push_back(first(s));
                auto n = second(s);
                ::declare(env, nn, n, s);
            }
        }
    }

    void codegen(VM* vm) override {
        for (auto& o:_exports) {
            vm->enter_data(o);
        }
    }

    void render(std::ostream& os) const override {
        os << "dynamic module: " << get_filename();
    }

    static bool filetype(const icu::UnicodeString& fn) {
        return unicode_endswith(fn, ".ego");
    }

private:
    exports_t           _handle;
    QualifiedStrings    _imports;
    QualifiedStrings    _values;
    VMObjectPtrs        _exports;
};

#define LINUX
#ifdef LINUX

#include <dlfcn.h>

class ModuleDynamic: public Module {
public:
    ModuleDynamic(const icu::UnicodeString& p, const icu::UnicodeString& fn, VM* m):
        Module(MODULE_DYNAMIC, p, fn, m),
            _handle(0), _imports(0), _exports(0) {
    }

    ModuleDynamic(const ModuleDynamic& m):
        Module(MODULE_DYNAMIC, m.get_path(), m.get_filename(), m.machine()),
        _handle(m._handle), _imports(m._imports), _exports(m._exports) {
        set_options(m.get_options());
    }

    ModulePtr clone() const override {
        return ModulePtr(new ModuleDynamic(*this));
    }

    void load() override {
        char* error;

        dlerror();

        _handle = dlopen(unicode_to_char(get_path()), RTLD_LAZY | RTLD_GLOBAL); // XXX: leaks?
        if (!_handle) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            err += " on open(" + get_path() + ")";
            throw ErrorIO(err);
        }

        std::vector<icu::UnicodeString> (*egel_imports)();
        egel_imports = (std::vector<icu::UnicodeString> (*)())
                            dlsym(_handle, "egel_imports");
        error = dlerror();
        if (error != NULL) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            throw ErrorIO(err);
        }

        std::vector<VMObjectPtr>   (*egel_exports)(VM*);
        egel_exports = (std::vector<VMObjectPtr> (*)(VM*))
                            dlsym(_handle, "egel_exports");
        error = dlerror();
        if (error != NULL) {
            icu::UnicodeString err = "dynamic load error: ";
            err += dlerror();
            throw ErrorIO(err);
        }

        auto ss = (*egel_imports)();

        QualifiedStrings ii;
        Position p(get_path(), 1, 1);
        for (auto& s:ss) {
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

    void declarations(NamespacePtr& env) override {
        for (auto& o:_exports) {
            if (o->tag() == VM_OBJECT_COMBINATOR) {
                auto sym = VM_OBJECT_COMBINATOR_SYMBOL(o);
                auto s   = machine()->get_combinator_string(sym);

                UnicodeStrings nn;
                nn.push_back(first(s));
                auto n = second(s);
                ::declare(env, nn, n, s);
            }
        }
    }

    void codegen(VM* vm) override {
        for (auto& o:_exports) {
            vm->enter_data(o);
        }
    }

    void render(std::ostream& os) const override {
        os << "dynamic module: " << get_filename();
    }

    static bool filetype(const icu::UnicodeString& fn) {
        return unicode_endswith(fn, ".ego");
    }

private:
    void*               _handle;
    QualifiedStrings    _imports;
    QualifiedStrings    _values;
    VMObjectPtrs        _exports;
};
#endif

class ModuleSource : public Module {
public:
    ModuleSource(const icu::UnicodeString& path, const icu::UnicodeString& fn, VM* m):
        Module(MODULE_SOURCE, path, fn, m),
        _source(""), _ast(0) {
    }

    ModuleSource(const ModuleSource& m):
        Module(MODULE_SOURCE, m.get_path(), m.get_filename(), m.machine()),
        _source(m._source), _ast(m._ast) {
        set_options(m.get_options());
    }

    ModulePtr clone() const override {
        return ModulePtr(new ModuleSource(*this));
    }

    void load() override {
        if (file_exists(get_path())) {
            _source = file_read(get_path());
        } else {
            throw ErrorIO("module " + get_path() + " not found");
        };
    }

    void unload() override {
    }

    QualifiedStrings imports() override {
        auto aa = ::imports(_ast);
        auto ii = QualifiedStrings();
        for (auto a:aa) {
            if (a->tag() == AST_DIRECT_IMPORT) {
                AST_DIRECT_IMPORT_SPLIT(a, p, s);
                ii.push_back(QualifiedString(p, unicode_strip_quotes(s)));
            }
        }
        return ii;        
    }

    QualifiedStrings values() override {
        auto aa = ::values(_ast);
        auto ii = QualifiedStrings();
        for (auto a:aa) {
            if (a->tag() == AST_DECL_VALUE) {
                AST_DECL_VALUE_SPLIT(a, p, n, f);
                ii.push_back(QualifiedString(p, n->to_text()));
            }
        }
        return ii;        
    }

    VMObjectPtrs exports() override {
        return VMObjectPtrs(); // XXX XXX XXX: implement this
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
            exit (EXIT_SUCCESS);
        };

        AstPtr a = parse(tt);

        if (get_options()->only_unparse()) {
            std::cout << a << std::endl;
            exit (EXIT_SUCCESS);
        };

        _source = "";
        _ast = a;
	}

    void declarations(NamespacePtr& env) override {
        declare(env, _ast);
	}

    void semantical(NamespacePtr& env) override {
        _ast = ::identify(env, _ast);

        if (get_options()->only_semantical()) {
#ifdef DEBUG
            std::cout << env << std::endl;
#endif
            std::cout << _ast << std::endl;
            exit (EXIT_SUCCESS);
        };
	}

    void desugar() override {
         _ast = ::desugar(_ast);
        if (get_options()->only_desugar()) {
            std::cout << _ast << std::endl;
            exit (EXIT_SUCCESS);
        };

	}

    void lift() override {
         _ast = ::lift(_ast);

        if (get_options()->only_lift()) {
            std::cout << _ast << std::endl;
            exit (EXIT_SUCCESS);
        };
	}

    void datagen(VM* vm) override {
        ::emit_data(vm, _ast);
	}

    void codegen(VM* vm) override {
        ::emit_code(vm, _ast);
        if (get_options()->only_bytecode()) {
            vm->render(std::cout);
            exit (EXIT_SUCCESS);
        };
	}

    void render(std::ostream& os) const override {
        os << "source module " << get_filename();
    }

    static bool filetype(const icu::UnicodeString& fn) {
        return unicode_endswith(fn, ".eg");
    }

private:
    icu::UnicodeString   _source;
    AstPtr          _ast;
};

typedef std::vector<ModulePtr> ModulePtrs;

class ModuleManager {
public:
    ModuleManager() {
    }

    ModuleManager(const ModuleManager& mm):
        _options(mm._options), _machine(mm._machine), _environment(mm._environment),
        _modules(mm._modules), _loading(mm._loading) {
    }

    ModuleManagerPtr clone() {
        return ModuleManagerPtr(new ModuleManager(*this));
    }

    void init(const OptionsPtr& oo, VM* vm) {
        NamespacePtr env = Namespace().clone();
        set_options(oo);
        set_machine(vm);
        set_environment(env);
        // next code is slightly conceptually dirty, needs a better solution
        auto sys = ModuleInternal("internal", vm, &builtin_system).clone();
        auto mth = ModuleInternal("internal", vm, &builtin_math).clone();
        auto str = ModuleInternal("internal", vm, &builtin_string).clone();
        auto run = ModuleInternal("internal", vm, &builtin_runtime).clone();
        auto thd = ModuleInternal("internal", vm, &builtin_thread).clone();
        auto prc = ModuleInternal("internal", vm, &builtin_process).clone();
        auto evl = ModuleInternal("internal", vm, &builtin_eval).clone();
        sys->load(); mth->load(); str->load(); run->load(); thd->load(); prc->load(); evl->load();
        _loading.push_back(sys);
        _loading.push_back(mth);
        _loading.push_back(str);
        _loading.push_back(run);
        _loading.push_back(thd);
        _loading.push_back(prc);
        _loading.push_back(evl);
        process();
        flush();
    }

    void set_options(const OptionsPtr& oo) {
        _options = oo;
    }

    OptionsPtr get_options() const {
        return _options;
    }

    void set_machine(VM* vm) {
        _machine = vm;
    }

    VM* get_machine() const {
        return _machine;
    }

    void set_environment(const NamespacePtr& env) {
        _environment = env;
    }

    NamespacePtr get_environment() const {
        return _environment;
    }

    ModulePtrs get_modules() const {
        return _modules;
    }

    // implements incremental loading for interactive mode
    void load(const Position& p, const icu::UnicodeString& fn) {
        preload(p, fn);
        _loading[0]->set_options(_options);
        transitive_closure();
         reverse(); // XXX: why was this again?
        process();
        flush();
    }

    QualifiedStrings values() {
        QualifiedStrings ss;
        for(auto& m:_modules) {
            auto vv = m->values();
            for (auto& s:vv) {
                ss.push_back(s);
            }
        }
        return ss;
    }

    friend std::ostream& operator<<(std::ostream& os, const ModuleManager& mm) {
        for (auto m:mm._modules) {
            os << m << std::endl;
        }
        return os;
    }

protected:
    icu::UnicodeString search(const UnicodeStrings& path, const icu::UnicodeString& fn) {
        auto fn_here = path_absolute(fn); // XXX: shouldn't this be in path?
        if (file_exists(fn_here)) return fn_here;
        for (auto p:path) {
            auto fn0 = path_combine(p, fn);
            auto fn1 = path_absolute(fn0);
            if (file_exists(fn1)) return fn1;
        };
        return "";
    }


    bool already_loaded(const icu::UnicodeString& fn) {
        for (auto& m:_modules) {
            if (m->get_path().compare(fn) == 0) return true;
        }
        for (auto& m:_loading) {
            if (m->get_path().compare(fn) == 0) return true;
        }
        return false;
    }

    void preload(const Position& p, const icu::UnicodeString& fn) {
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
                    m = ModuleSource(find, fn, _machine).clone();
                } else if (ModuleDynamic::filetype(fn)) {
                    m = ModuleDynamic(find, fn, _machine).clone();
                } else {
                    throw ErrorIO(p, "file \"" + fn + "\" has wrong extension");
                }
                m->set_options(Options().clone());//XXX check this
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
        uint_t n = 0;
        while (n < _loading.size()) {
            auto m = _loading[n];
            m->syntactical();
            auto ii = m->imports();
            for (auto& i:ii) {
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
        for (auto& m:_loading) {
            m->declarations(_environment);
        }
        for (auto& m:_loading) {
            m->semantical(_environment);
        }
        for (auto& m:_loading) {
            m->desugar();
        }
        for (auto& m:_loading) {
            m->lift();
        }
        for (auto& m:_loading) {
            m->datagen(_machine);
        }
        for (auto& m:_loading) {
            m->codegen(_machine);
        }
    }

    void flush() {
        for(auto& m:_loading) {
            _modules.push_back(m);
        }
        ModulePtrs empty;
        _loading = empty;
    }

private:
    OptionsPtr          _options;
    VM*                 _machine;
    NamespacePtr        _environment;
    ModulePtrs          _modules;
    ModulePtrs          _loading;
};

#endif
