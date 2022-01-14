export module constants;

export {
    constexpr auto CHAR_BEL = '\a';
    constexpr auto CHAR_BS = '\b';
    constexpr auto CHAR_HT = '\t';
    constexpr auto CHAR_LF = '\n';
    constexpr auto CHAR_VT = '\v';
    constexpr auto CHAR_FF = '\f';
    constexpr auto CHAR_CR = '\n';
    constexpr auto CHAR_BB = '\\';
    constexpr auto CHAR_SQ = '\'';
    constexpr auto CHAR_DQ = '"';
    constexpr auto CHAR_BQ = '`';

    constexpr auto CHAR_SPACE = ' ';
    constexpr auto CHAR_BREAK = '\n';

    constexpr auto CHAR_SQUOTE = '\'';
    constexpr auto CHAR_DQUOTE = '"';

    // quotes

    constexpr auto SQUOTE = "'";
    constexpr auto DQUOTE = "\"";
    constexpr auto BQUOTE = "`";
    constexpr auto BACKSLASH = "\\";

    // commments

    constexpr auto COMMENT_LINE = "//";
    constexpr auto COMMENT_START = "/*";
    constexpr auto COMMENT_END = "*/";

    // seperators

    constexpr auto STRING_COLON = ":";
    constexpr auto STRING_DCOLON = "::";
    constexpr auto STRING_SEMICOLON = ";";
    constexpr auto STRING_DSEMICOLON = ";;";
    constexpr auto STRING_DOT = ".";
    constexpr auto STRING_COMMA = ",";
    constexpr auto STRING_BAR = "|";
    constexpr auto STRING_HASH = "#";
    constexpr auto STRING_UNDER = "_";

    constexpr auto STRING_LPAREN = "(";
    constexpr auto STRING_RPAREN = ")";
    constexpr auto STRING_LSQUARE = "[";
    constexpr auto STRING_RSQUARE = "]";
    constexpr auto STRING_LCURLY = "{";
    constexpr auto STRING_RCURLY = "}";

    // expressions

    constexpr auto STRING_EQUAL = "=";
    constexpr auto STRING_ASSIGN = "<-";
    constexpr auto STRING_LAMBDA = "\\";
    constexpr auto STRING_ARROW = "->";
    constexpr auto STRING_DARROW = "=>";
    constexpr auto STRING_STAR = "*";
    constexpr auto STRING_PLUS = "+";
    constexpr auto STRING_QUESTION = "?";
    constexpr auto STRING_BANG = "!";
    constexpr auto STRING_SLASH = "/";

    // namespaces

    constexpr auto STRING_IMPORT = "import";
    constexpr auto STRING_NAMESPACE = "namespace";
    constexpr auto STRING_USING = "using";

    // declarations

    constexpr auto STRING_DEF = "def";
    constexpr auto STRING_VAL = "val";
    constexpr auto STRING_DATA = "data";

    // expressions
    constexpr auto STRING_IF = "if";
    constexpr auto STRING_THEN = "then";
    constexpr auto STRING_ELSE = "else";
    constexpr auto STRING_THROW = "throw";
    constexpr auto STRING_TRY = "try";
    constexpr auto STRING_CATCH = "catch";
    constexpr auto STRING_LET = "let";
    constexpr auto STRING_IN = "in";

    // names, combinators, ..

    constexpr auto STRING_PART = "part";
    constexpr auto STRING_LOWERCASE = "lowercase";
    constexpr auto STRING_UPPERCASE = "uppercase";
    constexpr auto STRING_NAME = "name";
    constexpr auto STRING_VARIABLE = "variable";
    constexpr auto STRING_IDENTIFIER = "identifier";
    constexpr auto STRING_COMBINATOR = "combinator";
    constexpr auto STRING_OPERATOR = "operator";

    // basic types

    constexpr auto STRING_UNIT = "unit";
    constexpr auto STRING_NONE = "none";

    constexpr auto STRING_BOOL = "bool";
    constexpr auto STRING_TRUE = "true";
    constexpr auto STRING_FALSE = "false";

    constexpr auto STRING_INT = "int";
    constexpr auto STRING_FLOAT = "float";
    constexpr auto STRING_LONG = "long";
    constexpr auto STRING_CHAR = "char";
    constexpr auto STRING_STRING = "string";
    constexpr auto STRING_DOUBLE = "double";
    constexpr auto STRING_PTR = "ptr";

    constexpr auto STRING_TEXT = "text";

    constexpr auto STRING_LIST = "list";
    constexpr auto STRING_NIL = "nil";
    constexpr auto STRING_CONS = "cons";

    constexpr auto STRING_TUPLE = "tuple";
    constexpr auto STRING_CLASS = "class";
    constexpr auto STRING_OBJECT = "object";
    constexpr auto STRING_EXTEND = "extend";    // this is the combinator
    constexpr auto STRING_EXTENDS = "extends";  // this is the keyword
    constexpr auto STRING_WITH = "with";

    constexpr auto STRING_FAIL = "fail";

    // System

    constexpr auto STRING_SYSTEM = "System";
    constexpr auto STRING_K = "k";
    constexpr auto STRING_ID = "id";

    // main section

    constexpr auto STRING_MAIN = "main";

    // compiler phases

    constexpr auto STRING_INTERNAL = "internal";
    constexpr auto STRING_IO = "input/output";
    constexpr auto STRING_LEXICAL = "lexical";
    constexpr auto STRING_IDENTIFICATION = "identification";
    constexpr auto STRING_SYNTACTICAL = "syntactical";
    constexpr auto STRING_SEMANTICAL = "semantical";
    constexpr auto STRING_LINKER = "linker";
    constexpr auto STRING_CODEGEN = "code";

    // compiler errors

    constexpr auto STRING_ERROR_REDECLARE = "redeclaration";
    constexpr auto STRING_ERROR_UNDECLARED = "undeclared";
    constexpr auto STRING_ERROR_DECLARED = "declared";
    constexpr auto STRING_ERROR_NOPATTERN = "pattern expected";
    constexpr auto STRING_ERROR_NODEF = "definition expected";
    constexpr auto STRING_ERROR_NOEXPR = "expression expected";
    constexpr auto STRING_ERROR_NODECL = "declaration expected";

    // lambda
    constexpr auto STRING_CONST = "const";
    constexpr auto STRING_EMBED = "embed";
    constexpr auto STRING_CALL = "call";
    constexpr auto STRING_SELECT = "select";

    // readable text for token types
    constexpr auto STRING_EOF = "end of file";
    constexpr auto STRING_ERROR = "error";
    constexpr auto STRING_EQ = "=";
    constexpr auto STRING_INTEGER = "integer";
    constexpr auto STRING_HEXINTEGER = "hexadecimal integer";

    // human readable byte opcodes
    constexpr auto STRING_OP_ASSIGN = "assign";
    constexpr auto STRING_OP_INC = "inc";
    constexpr auto STRING_OP_ADD = "add";
    constexpr auto STRING_OP_SUB = "sub";
    constexpr auto STRING_OP_MUL = "mul";
    constexpr auto STRING_OP_DIV = "div";
    constexpr auto STRING_OP_MOD = "mod";
    constexpr auto STRING_OP_LEN = "len";
    constexpr auto STRING_OP_EQ = "eq";
    constexpr auto STRING_OP_LT = "lt";
    constexpr auto STRING_OP_LE = "le";
    constexpr auto STRING_OP_NIL = "nil";
    constexpr auto STRING_OP_MOV = "mov";
    constexpr auto STRING_OP_LOAD = "load";
    constexpr auto STRING_OP_DATA = "data";
    constexpr auto STRING_OP_TAKE = "take";
    constexpr auto STRING_OP_SPLIT = "split";
    constexpr auto STRING_OP_TAKEX = "takex";
    constexpr auto STRING_OP_SPLITX = "splitx";
    constexpr auto STRING_OP_COMBINE = "combine";
    constexpr auto STRING_OP_CONCAT = "concat";
    constexpr auto STRING_OP_CONCATX = "concatx";
    constexpr auto STRING_OP_TEST = "test";
    constexpr auto STRING_OP_TAG = "tag";
    constexpr auto STRING_OP_JMP = "jmp";
    constexpr auto STRING_OP_BRANCH = "branch";

    // local and a magic marker for local definitions
    constexpr auto STRING_LOCAL = "Local";
    constexpr auto STRING_MAGIC_START = "[[";
    constexpr auto STRING_MAGIC_END = "]]";
}
