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
#include "builtin.hpp"

// convenience
inline UnicodeString first(const UnicodeString& s) {
    auto d = s;
    auto i = d.indexOf('.');
    return d.remove(i, d.length());
}

inline UnicodeString second(const UnicodeString& s) {
    auto d = s;
    auto i = d.indexOf('.');
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
    
    void add_include_path(const UnicodeString& p) {
        _include_path.push_back(p);
    }
    
    void add_library_path(const UnicodeString& p) {
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


typedef enum {
    MODULE_SOURCE,
    MODULE_DYNAMIC,
    MODULE_INTERACTIVE,
} module_tag_t;

class Module {
public:
    Module(const module_tag_t t, const UnicodeString& p, const UnicodeString& fn):
        _tag(t), _path(p), _filename(fn) {
    }

    virtual ModulePtr clone() const = 0;

    void set_options(const OptionsPtr& o) {
        _options = o;
    }

    OptionsPtr get_options() const {
        return _options;
    }

    UnicodeString get_path() const {
        return _path;
    }

    UnicodeString get_filename() const {
        return _filename;
    }

    module_tag_t tag() const {
        return _tag;
    }

    virtual void load(VM* vm) = 0;

    virtual void unload() = 0;

    virtual AstPtrs imports() = 0;

    // this is why OO is sometimes bad. but I am lazy at the moment. 
    virtual void syntactical() {};

    virtual void declarations(NamespacePtr& env) = 0;

    virtual void semantical(NamespacePtr& env) {};

    virtual void desugar() {};

    virtual void lift() {};

    virtual void datagen(VM* vm) {};

    virtual void codegen(VM* vm) = 0;

    virtual void render(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const ModulePtr& m) {
        m->render(os);
        return os;
    }

private:
    module_tag_t    _tag;
    OptionsPtr      _options;
    UnicodeString   _path;
    UnicodeString   _filename;
};

#define LINUX
#ifdef LINUX

#include <dlfcn.h>

class ModuleDynamic: public Module {
public:
    ModuleDynamic(const UnicodeString& p, const UnicodeString& fn): 
        Module(MODULE_DYNAMIC, p, fn),
            _handle(0), _imports(0), _exports(0), _machine(0) {
    }

    ModuleDynamic(const ModuleDynamic& m):
        Module(MODULE_DYNAMIC, m.get_path(), m.get_filename()), 
        _handle(m._handle), _imports(m._imports), _exports(m._exports), _machine(m._machine) {
        set_options(m.get_options());
    }

    ModulePtr clone() const override {
        return ModulePtr(new ModuleDynamic(*this));
    }

    void load(VM* vm) override {
        _machine = vm;
        char* error;

        dlerror();

        _handle = dlopen(unicode_to_char(get_path()), RTLD_LAZY);
        if (!_handle) {
            UnicodeString err = "dynamic load error: ";
            err += dlerror();
            throw ErrorIO(err);
        }
        
        std::vector<UnicodeString> (*egel_imports)();
        egel_imports = (std::vector<UnicodeString> (*)()) 
                            dlsym(_handle, "egel_imports");
        error = dlerror();
        if (error != NULL) {
            UnicodeString err = "dynamic load error: ";
            err += dlerror();
            throw ErrorIO(err);
        }

        std::vector<VMObjectPtr>   (*egel_exports)(VM*);
        egel_exports = (std::vector<VMObjectPtr> (*)(VM*)) 
                            dlsym(_handle, "egel_exports");
        error = dlerror();
        if (error != NULL) {
            UnicodeString err = "dynamic load error: ";
            err += dlerror();
            throw ErrorIO(err);
        }

        _imports = (*egel_imports)();
        _exports = (*egel_exports)(vm);
    }

    void unload() override {
        dlclose(_handle);
    }

    AstPtrs imports() override {
        AstPtrs ii;
        Position p(get_path(), 1, 1);
        for (auto& s:_imports) {
            ii.push_back(AstDirectImport(p, s).clone());
        }
        return ii;
    }

    void declarations(NamespacePtr& env) override {
        for (auto& o:_exports) {
            if (o->tag() == VM_OBJECT_COMBINATOR) {
                auto sym = VM_OBJECT_COMBINATOR_SYMBOL(o);
                auto s   = _machine->get_symbol(sym);

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

    static bool filetype(const UnicodeString& fn) {
        return unicode_endswith(fn, ".ego");
    }

private:
    void*           _handle;
    UnicodeStrings  _imports;
    VMObjectPtrs    _exports;
    VM*             _machine;
};
#endif

class ModuleSource : public Module {
public:
    ModuleSource(const UnicodeString& path, const UnicodeString& fn): 
        Module(MODULE_SOURCE, path, fn),
        _source(""), _ast(0), _qualified(false) {
    }

    ModuleSource(const ModuleSource& m):
        Module(MODULE_SOURCE, m.get_path(), m.get_filename()),
        _source(m._source), _ast(m._ast), _qualified(m._qualified) {
        set_options(m.get_options());
    }

    ModulePtr clone() const override {
        return ModulePtr(new ModuleSource(*this));
    }

    void load(VM* vm) override {
        if (file_exists(get_path())) {
            _source = file_read(get_path());
        } else {
            throw ErrorIO("module " + get_path() + " not found");
        };
    }

    void unload() override {
    }

    AstPtrs imports() override {
        return ::imports(_ast);
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

    static bool filetype(const UnicodeString& fn) {
        return unicode_endswith(fn, ".eg");
    }

private:
    UnicodeString   _source;
    AstPtr          _ast;
    bool            _qualified;
};

typedef std::vector<ModulePtr> ModulePtrs;

class ModuleManager {
public:
    ModuleManager() {
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
    void load(const Position& p, const UnicodeString& fn) {
        preload(p, fn);
        _loading[0]->set_options(_options);
        transitive_closure();
         reverse(); // why was this again?
        process();
        flush();
    }

    void builtin() {
        // XXX: move this to a module
        auto oo = vm_export(_machine);

        for (auto& o:oo) {
            if (o->tag() == VM_OBJECT_COMBINATOR) {
                auto sym = VM_OBJECT_COMBINATOR_SYMBOL(o);
                auto s   = _machine->get_symbol(sym);

                UnicodeStrings nn;
                nn.push_back(first(s));
                auto n = second(s);
                ::declare(_environment, nn, n, s);
            }
        }

        for (auto& o:oo) {
            _machine->enter_data(o);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const ModuleManager& mm) {
        for (auto m:mm._modules) {
            os << m << std::endl;
        }
        return os;
    }

protected:
    UnicodeString search(const UnicodeStrings& path, const UnicodeString& fn) {
        if (file_exists(fn)) return fn;
        for (auto p:path) {
            UnicodeString fn0 = path_combine(p, fn);
            if (file_exists(fn0)) return fn0;
        };
        return "";
    }


    bool already_loaded(const UnicodeString& fn) {
        for (auto& m:_modules) {
            if (m->get_path().compare(fn) == 0) return true;
        }
        for (auto& m:_loading) {
            if (m->get_path().compare(fn) == 0) return true;
        }
        return false;
    }

    void preload(const Position& p, const UnicodeString& fn) {
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
                    m = ModuleSource(find, fn).clone();
                } else if (ModuleDynamic::filetype(fn)) {
                    m = ModuleDynamic(find, fn).clone();
                } else {
                    throw ErrorIO(p, "file \"" + fn + "\" has wrong extension");
                }
                m->set_options(Options().clone());//XXX check this
                try {
                    m->load(_machine);
                } catch (ErrorIO e) {
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
                AST_DIRECT_IMPORT_SPLIT(i, p, fn);
                preload(p, unicode_strip_quotes(fn));
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
    ModulePtrs          _modules;
    ModulePtrs          _loading;
    OptionsPtr          _options;
    NamespacePtr        _environment;
    VM*                 _machine;
};

#endif
