#ifndef MODULES_HPP
#define MODULES_HPP

#include <vector>

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
#include "builtin_thread.hpp"
#include "builtin_process.hpp"
#include "builtin_eval.hpp"

extern std::vector<VMObjectPtr> builtin_eval(VM* vm); // XXX: forward declaration

// convenience
inline icu::UnicodeString first(const icu::UnicodeString& s) {
    auto d = s;
    auto i = d.indexOf(':');
    return d.remove(i, d.length());
}

inline icu::UnicodeString second(const icu::UnicodeString& s) {
    auto d = s;
    auto i = d.indexOf(':');
    return d.remove(0, i+1);
}

class Options;
typedef std::shared_ptr<Options> OptionsPtr;

class Options {
public:
    Options():
        _interactive_flag(false),
        _tokenize_flag(false), _unparse_flag(false),
        _semantical_flag(false), _desugar_flag(false),
        _lift_flag(false), _bytecode_flag(false) {
        _include_path = UnicodeStrings();
        _library_path = UnicodeStrings();
    }

    Options(bool i, bool t, bool u, bool s, bool d, bool l, bool b,
            const UnicodeStrings& ii, const UnicodeStrings& ll):
        _interactive_flag(i),
        _tokenize_flag(t), _unparse_flag(u), _semantical_flag(s),
        _desugar_flag(d), _lift_flag(l), _bytecode_flag(b),
        _include_path(ii), _library_path(ll) {
    }

    Options(const Options& o):
        _interactive_flag(o._interactive_flag),
        _tokenize_flag(o._tokenize_flag), _unparse_flag(o._unparse_flag),
        _semantical_flag(o._semantical_flag), _desugar_flag(o._desugar_flag),
        _lift_flag(o._lift_flag), _bytecode_flag(o._bytecode_flag),
        _include_path(o._include_path),
        _library_path(o._library_path) {
    }

    OptionsPtr clone() {
        return OptionsPtr(new Options(*this));
    }

    void set_include_path(const UnicodeStrings& dd) {
        _include_path = dd;
    }

    UnicodeStrings get_include_path() const {
        return _include_path;
    }

    void set_library_path(const UnicodeStrings& dd) {
        _library_path = dd;
    }

    UnicodeStrings get_library_path() const {
        return _library_path;
    }

    void add_include_path(const icu::UnicodeString& p) {
        _include_path.push_back(p);
    }

    void add_library_path(const icu::UnicodeString& p) {
        _library_path.push_back(p);
    }

    void set_interactive(bool f) {
        _interactive_flag = f;
    }

    bool interactive() const {
        return _interactive_flag;
    }

    void set_tokenize(bool f) {
        _tokenize_flag = f;
    }

    bool only_tokenize() const {
        return _tokenize_flag;
    }

    void set_desugar(bool f) {
        _desugar_flag = f;
    }

    bool only_desugar() const {
        return _desugar_flag;
    }

    void set_unparse(bool f) {
        _unparse_flag = f;
    }

    bool only_unparse() const {
        return _unparse_flag;
    }

    void set_semantical(bool f) {
        _semantical_flag = f;
    }

    bool only_semantical() const {
        return _semantical_flag;
    }

    void set_lift(bool f) {
        _lift_flag = f;
    }

    bool only_lift() const {
        return _lift_flag;
    }

    void set_bytecode(bool f) {
        _bytecode_flag = f;
    }

    bool only_bytecode() const {
        return _bytecode_flag;
    }

    void render(std::ostream& os) const {
        os << "interactive:" << _interactive_flag << std::endl;
        os << "tokenize:   " << _tokenize_flag << std::endl;
        os << "unparse:    " << _unparse_flag << std::endl;
        os << "semantical: " << _semantical_flag << std::endl;
        os << "desugar:    " << _desugar_flag << std::endl;
        os << "lift:       " << _lift_flag << std::endl;
        os << "bytecode:   " << _bytecode_flag << std::endl;
        os << "include:    ";
        for (auto& i:_include_path) {
            os << i << ":";
        }
        os << std::endl;
        os << "library:    ";
        for (auto& i:_library_path) {
            os << i << ":";
        }
        os << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& os, const OptionsPtr& m) {
        m->render(os);
        return os;
    }
private:
    bool    _interactive_flag;
    bool    _tokenize_flag;
    bool    _unparse_flag;
    bool    _semantical_flag;
    bool    _desugar_flag;
    bool    _lift_flag;
    bool    _bytecode_flag;
    UnicodeStrings  _include_path;
    UnicodeStrings  _library_path;
};

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

    QualifiedString(const QualifiedString& i):
        _position(i._position), _string(i._string) {
    }

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
    module_tag_t    _tag;
    OptionsPtr      _options;
    icu::UnicodeString   _path;
    icu::UnicodeString   _filename;
    VM*             _machine;
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
                auto s   = machine()->get_symbol(sym);

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

        _handle = dlopen(unicode_to_char(get_path()), RTLD_LAZY); // XXX: leaks?
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
                auto s   = machine()->get_symbol(sym);

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

    void init(const OptionsPtr& oo, VM* vm, const NamespacePtr& env) {
        set_options(oo);
        set_machine(vm);
        set_environment(env);
        // next code is slightly conceptually dirty, needs a better solution
        auto sys = ModuleInternal("internal", vm, &builtin_system).clone();
        auto mth = ModuleInternal("internal", vm, &builtin_math).clone();
        auto str = ModuleInternal("internal", vm, &builtin_string).clone();
        auto thd = ModuleInternal("internal", vm, &builtin_thread).clone();
        auto prc = ModuleInternal("internal", vm, &builtin_process).clone();
        auto evl = ModuleInternal("internal", vm, &builtin_eval).clone();
        sys->load(); mth->load(); str->load(); thd->load(); prc->load(); evl->load();
        _loading.push_back(sys);
        _loading.push_back(mth);
        _loading.push_back(str);
        _loading.push_back(thd);
        _loading.push_back(prc);
        _loading.push_back(evl);
        process();
        flush();
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

    void set_options(const OptionsPtr& oo) {
        _options = oo;
    }

    OptionsPtr get_options() const {
        return _options;
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
