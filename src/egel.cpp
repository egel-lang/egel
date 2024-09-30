#include "machine.hpp"
#include "runtime.hpp"

using namespace egel;

#define EXECUTABLE_NAME "egel"

#define EXECUTABLE_VERSION_MAJOR "0"
#define EXECUTABLE_VERSION_MINOR "1"
#define EXECUTABLE_VERSION_PATCH "14"

#define EXECUTABLE_VERSION                                \
    EXECUTABLE_VERSION_MAJOR "." EXECUTABLE_VERSION_MINOR \
                             "." EXECUTABLE_VERSION_PATCH

#define EXECUTABLE_COPYRIGHT "Copyright (C) 2017"
#define EXECUTABLE_AUTHORS "M.C.A. (Marco) Devillers"

#ifdef _WIN32
#define EGEL_PATH "."
#else
#define EGEL_PATH ".:/usr/local/lib/egel:/usr/lib/egel"
#endif

#if defined(_WIN32) || defined(_WIN64)
#define EXECUTABLE_OS "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
#define EXECUTABLE_OS "MacOS"
#elif defined(__linux__)
#define EXECUTABLE_OS "Linux"
#elif defined(__unix__) || defined(__unix)
#define EXECUTABLE_OS "Linux"
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__DragonFly__)
#define EXECUTABLE_OS "BSD"
#endif
#else
#error "Unknown operating system"
#endif

enum arg_t {
    OPTION_NONE,
    OPTION_FILE,
    OPTION_DIR,
    OPTION_NUMBER,
    OPTION_TEXT,
};

struct option_t {
    char const *shortname;
    char const *longname;
    arg_t argument;
    char const *description;
};

static option_t options[] = {
    {
        "-h",
        "--help",
        OPTION_NONE,
        "display usage",
    },
    {
        "-v",
        "--version",
        OPTION_NONE,
        "display version",
    },
    {
        "-",
        "--interact",
        OPTION_NONE,
        "interactive mode (default)",
    },
    {
        "-I",
        "--include",
        OPTION_DIR,
        "add include directory",
    },
    {
        "-e",
        "--eval",
        OPTION_TEXT,
        "evaluate command",
    },
    {
        "-b",
        "--blank",
        OPTION_NONE,
        "blank slate mode",
    },
    {
        "-T",
        "--tokens",
        OPTION_NONE,
        "output all tokens (debug)",
    },
    {
        "-U",
        "--unparse",
        OPTION_NONE,
        "output the parse tree (debug)",
    },
    {
        "-X",
        "--check",
        OPTION_NONE,
        "output analyzed tree (debug)",
    },
    {
        "-D",
        "--desugar",
        OPTION_NONE,
        "output desugared tree (debug)",
    },
    {
        "-C",
        "--lift",
        OPTION_NONE,
        "output combinator lifted tree (debug)",
    },
    {
        "-B",
        "--bytes",
        OPTION_NONE,
        "output bytecode (debug)",
    },
};

using StringPairs =
    std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>>;

StringPairs parse_options(int argc, char *argv[]) {
    StringPairs pp;
    for (int a = 1; a < argc; a++) {
        size_t pp_size = pp.size();
        for (auto &o : options) {
            if ((strncmp(argv[a], o.shortname, 32) == 0) ||
                (strncmp(argv[a], o.longname, 32) == 0)) {
                switch (o.argument) {
                    case OPTION_NONE:
                        pp.push_back(
                            std::make_pair(icu::UnicodeString(o.shortname),
                                           icu::UnicodeString("")));
                        break;
                    case OPTION_FILE:
                        if (a == argc - 1) goto options_error;
                        pp.push_back(
                            std::make_pair(icu::UnicodeString(o.shortname),
                                           icu::UnicodeString(argv[a + 1])));
                        a++;
                        break;
                    case OPTION_DIR:
                        if (a == argc - 1) goto options_error;
                        pp.push_back(
                            std::make_pair(icu::UnicodeString(o.shortname),
                                           icu::UnicodeString(argv[a + 1])));
                        a++;
                        break;
                    case OPTION_NUMBER:
                        if (a == argc - 1) goto options_error;
                        pp.push_back(
                            std::make_pair(icu::UnicodeString(o.shortname),
                                           icu::UnicodeString(argv[a + 1])));
                        a++;
                        break;
                    case OPTION_TEXT:
                        if (a == argc - 1) goto options_error;
                        pp.push_back(
                            std::make_pair(icu::UnicodeString(o.shortname),
                                           icu::UnicodeString(argv[a + 1])));
                        a++;
                        break;
                };
            };
        }

        if (pp_size == pp.size()) {  // XXX: didn't add an option - not sure
                                     // this always works
            pp.push_back(std::make_pair(icu::UnicodeString("--"),
                                        icu::UnicodeString(argv[a])));
        }
    }
    return pp;

options_error:
    throw 0;
}

void display_usage() {
    std::cout << "Usage: " << EXECUTABLE_NAME << " [options] [filename]"
              << std::endl;
    std::cout << "Options:" << std::endl;

    for (auto &o : options) {
        std::cout << "\t[" << o.shortname << "|" << o.longname << "] ";
        switch (o.argument) {
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
        std::cout << "\t" << o.description << std::endl;
    };
}

void display_version(const OptionsPtr &oo) {
    std::cout << EXECUTABLE_NAME << ' ' << EXECUTABLE_VERSION << std::endl;
    std::cout << "    EGEL_PATH: ";
    for (const auto &p : oo->get_include_path()) {
        std::cout << "\"" << p << "\" ";
    }
    std::cout << std::endl;
    std::cout << "     EGEL_PS0: \"" << VM::env_egel_ps0() << "\"" << std::endl;
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

    // options
    OptionsPtr oo = Options::create();

    // add include path from environment
    auto path = VM::env_egel_path();
    if (path != "") {
        oo->set_include_path(path);
    } else {
        oo->set_include_path(EGEL_PATH);
    }

    // add include paths from prompt
    for (auto &p : pp) {
        if (p.first == ("-I")) {
            oo->add_include_path(p.second);
        };
    };

    // check for -h or -v
    for (auto &p : pp) {
        if (p.first == ("-h")) {
            display_usage();
            return (EXIT_SUCCESS);
        };
        if (p.first == ("-v")) {
            display_version(oo);
            return (EXIT_SUCCESS);
        };
    };

    bool blank_state = false;
    // check for flags
    for (auto &p : pp) {
        if (p.first == ("-")) {
            oo->set_interactive(true);
        };
        if (p.first == ("-b")) {
            blank_state = true;
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
    for (auto &p : pp) {
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
    for (auto &p : pp) {
        if (p.first == ("-e")) {
            command = true;
            e = p.second;
        };
    };

    // create a machine
    VMPtr m = Machine::create();

    // initialize (rebinding exceptions need to be caught)
    try {
        m->initialize(oo);
    } catch (Error &e) {
        std::cerr << e << std::endl;
        return (EXIT_FAILURE);
    }

    // set the application arguments
    application_argc = argc;
    application_argv = argv;
    application_version =
        icu::UnicodeString("egel ") + EXECUTABLE_VERSION + " " + EXECUTABLE_OS;

    // load the file
    if (fn != "") {
        try {
            m->eval_module(fn);
        } catch (Error &e) {
            std::cerr << e << std::endl;
            return (EXIT_FAILURE);
        }
    }

    // start either interactive or batch mode
    icu::UnicodeString populate =
        "import \"prelude.eg\";;using System;;using List";
    if (command) {
        try {
            if (!blank_state) m->eval_command(icu::UnicodeString(populate));
            m->eval_command(e);
        } catch (Error &e) {
            std::cerr << e << std::endl;
            return (EXIT_FAILURE);
        }
    } else if ((fn == "") || oo->interactive()) {
        if (!blank_state) m->eval_command(icu::UnicodeString(populate));
        m->eval_interactive();
    } else {
        m->eval_main();
    }

    return EXIT_SUCCESS;
}
