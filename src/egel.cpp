#include <string.h>
#include <vector>

#include "utils.hpp"
#include "position.hpp"
#include "reader.hpp"
#include "lexical.hpp"
#include "syntactical.hpp"
#include "runtime.hpp"
#include "machine.hpp"
#include "eval.hpp"
#include "modules.hpp"

#define EXECUTABLE_NAME "egel"

#define EXECUTABLE_VERSION_MAJOR   "0"
#define EXECUTABLE_VERSION_MINOR   "1"
#define EXECUTABLE_VERSION_PATCH   "1"

#define EXECUTABLE_VERSION \
    EXECUTABLE_VERSION_MAJOR "." \
    EXECUTABLE_VERSION_MINOR "." \
    EXECUTABLE_VERSION_PATCH

#define EXECUTABLE_COPYRIGHT \
    "Copyright (C) 2017"
#define EXECUTABLE_AUTHORS \
    "M.C.A. (Marco) Devillers"

#define INCLUDE_PATH "/usr/local/lib64/egel"

typedef enum {
    OPTION_NONE,
    OPTION_FILE,
    OPTION_DIR,
    OPTION_NUMBER,
    OPTION_TEXT,
} arg_t;

typedef struct {
    char const*     shortname;
    char const*     longname;
    arg_t           argument;
    char const*     description;
} option_t;

static option_t options[] = {
    { "-h", "--help",       OPTION_NONE, "display usage", },
    { "-v", "--version",    OPTION_NONE, "display version", },
    { "-",  "--interact",   OPTION_NONE, "interactive mode (default)", },
    { "-I", "--include",    OPTION_DIR,  "add include directory", },
    { "-e", "--eval",       OPTION_TEXT, "evaluate command", },
    { "-T", "--tokens",     OPTION_NONE, "output all tokens (debug)", },
    { "-U", "--unparse",    OPTION_NONE, "output the parse tree (debug)", },
    { "-X", "--check",      OPTION_NONE, "output analyzed tree (debug)", },
    { "-D", "--desugar",    OPTION_NONE, "output desugared tree (debug)", },
    { "-C", "--lift",       OPTION_NONE, "output combinator lifted tree (debug)", },
    { "-B", "--bytes",      OPTION_NONE, "output bytecode (debug)", },
};

#define OPTIONS_SIZE    (sizeof(options)/sizeof(option_t))

typedef std::vector<std::pair<icu::UnicodeString, icu::UnicodeString> > StringPairs;

StringPairs parse_options(int argc, char *argv[]) {
    StringPairs pp;
    for (int a = 1; a < argc; a++) {
        uint_t sz = pp.size();

        for(uint_t i = 0; i < OPTIONS_SIZE; i++) {
            
            if ( (strncmp(argv[a], options[i].shortname, 32) == 0) ||
                 (strncmp(argv[a], options[i].longname, 32) == 0) ) {
                
                switch (options[i].argument) {
                case OPTION_NONE:
                    pp.push_back(std::make_pair(icu::UnicodeString(options[i].shortname), icu::UnicodeString("")));
                    break;
                case OPTION_FILE:
                    if (a == argc - 1) goto options_error;
                    pp.push_back(std::make_pair(icu::UnicodeString(options[i].shortname), icu::UnicodeString(argv[a+1])));
                    a++;
                    break;
                case OPTION_DIR:
                    if (a == argc - 1) goto options_error;
                    pp.push_back(std::make_pair(icu::UnicodeString(options[i].shortname), icu::UnicodeString(argv[a+1])));
                    a++;
                    break;
                case OPTION_NUMBER:
                    if (a == argc - 1) goto options_error;
                    pp.push_back(std::make_pair(icu::UnicodeString(options[i].shortname), icu::UnicodeString(argv[a+1])));
                    a++;
                    break;
                case OPTION_TEXT:
                    if (a == argc - 1) goto options_error;
                    pp.push_back(std::make_pair(icu::UnicodeString(options[i].shortname), icu::UnicodeString(argv[a+1])));
                    a++;
                    break;
                };
            };

        }

        if (sz == pp.size()) {
            pp.push_back(std::make_pair(icu::UnicodeString("--"), icu::UnicodeString(argv[a])));
        }
    }
    return pp;

options_error:
    throw 0;
}

void display_usage () {
    std::cout << "Usage: " << EXECUTABLE_NAME << " [options] [filename]" << std::endl;
    std::cout << "Options:" << std::endl;

    for (uint_t i = 0; i < OPTIONS_SIZE; i++) {
        std::cout << "\t[" << options[i].shortname << "|" << options[i].longname << "] ";
        switch (options[i].argument) {
        case OPTION_NONE:
            std::cout << "      "; 
            break;
        case OPTION_FILE:
            std::cout << "<file>"; 
            break;
        case OPTION_DIR:
            std::cout << "<dir>"; 
            break;
        case OPTION_NUMBER:
            std::cout << "<num>"; 
            break;
        case OPTION_TEXT:
            std::cout << "<text>"; 
            break;
        };
        std::cout << "\t" << options[i].description << std::endl;
    };
}

void display_version () {
    std::cout << EXECUTABLE_NAME << ' ' << EXECUTABLE_VERSION << std::endl;
    std::cout << EXECUTABLE_COPYRIGHT << ' ' << EXECUTABLE_AUTHORS << std::endl;
}

int main(int argc, char *argv[]) {
    // parse the options (quick and dirty)
    StringPairs pp;
    try {
        pp = parse_options(argc, argv);
    } catch (int ex) {
        std::cerr << "options error, try -h." << std::endl;
        return (EXIT_FAILURE);
    };
    
    // check for -h or -v
    for (auto& p : pp) {
        if (p.first == ("-h")) {
            display_usage();
            return (EXIT_SUCCESS);
        };
        if (p.first == ("-v")) {
            display_version();
            return (EXIT_SUCCESS);
        };
    };

    // options
    OptionsPtr oo = Options().clone();


    // check for include paths
    bool hasI = false;
    for (auto& p : pp) {
        if (p.first == ("-I")) {
            oo->add_include_path(p.second);
            hasI = true;
        };
    };

    // add local directory to the search path if no other where given
    if (!hasI) oo->add_include_path(icu::UnicodeString("./"));

    // add include path from environment
    auto istr = getenv("EGEL_INCLUDE");
    if (istr != nullptr) {
        oo->add_include_path(icu::UnicodeString(istr, -1, US_INV));
    } else {
        oo->add_include_path(INCLUDE_PATH);
    }

    // check for flags
    for (auto& p : pp) {
        if (p.first == ("-")) {
            oo->set_interactive(true);
        };
        if (p.first == ("-T")) {
            oo->set_tokenize(true);
        };
        if (p.first == ("-U")) {
            oo->set_unparse(true);
        };
        if (p.first == ("-X")) {
            oo->set_semantical(true);
        };
        if (p.first == ("-D")) {
            oo->set_desugar(true);
        };
        if (p.first == ("-C")) {
            oo->set_lift(true);
        };
        if (p.first == ("-B")) {
            oo->set_bytecode(true);
        };
    };

    // check for unique --/fn
    icu::UnicodeString fn;
    std::vector<icu::UnicodeString> aa;
    for (auto& p : pp) {
        if (p.first == ("--")) {
            if (fn == "") {
                fn = p.second;
            } else {
                aa.push_back(p.second);
            }
        }
    };

    // check for command
    bool command = false;
    icu::UnicodeString e;
    for (auto& p : pp) {
        if (p.first == ("-e")) {
            command = true;
            e = p.second;
        };
    };

    // start up the module system
    ModuleManagerPtr mm = ModuleManager().clone();
    Machine m;
    NamespacePtr env = Namespace().clone();

    // initialize (rebinding exceptions need to be caught)
    try {
        mm->init(oo, &m, env);
    } catch (Error &e) {
        std::cerr << e << std::endl;
        return (EXIT_FAILURE);
    }

    // fire up the evaluator
    Eval eval(mm);
    // make it possible for the low level machine to peek upward
    m.set_context((void*) &eval);

    // load the file
    if (fn != "") {
        try {
            eval.eval_load(fn);
        } catch (Error &e) {
            std::cerr << e << std::endl;
            return (EXIT_FAILURE);
        }
    }

    // set the application arguments
    application_argc = argc;
    application_argv = argv;

    // evaluate all values
    eval.eval_values(); //XXX: handle exceptions once

    // start either interactive or batch mode
    if (command) {
        try {
            eval.eval_command(icu::UnicodeString("using System"));
            eval.eval_command(e);
        } catch (Error &e) {
            std::cerr << e << std::endl;
            return (EXIT_FAILURE);
        }
    } else if ((fn == "") || oo->interactive()) {
        eval.eval_interactive();
    } else {
        eval.eval_main();
    }

    return EXIT_SUCCESS;
}
