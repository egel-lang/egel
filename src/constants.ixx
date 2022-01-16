export module constants;

// work around unimplemented feature
export class Constants {
public:
     static constexpr auto CHAR_BEL = '\a';
     static constexpr auto CHAR_BS = '\b';
     static constexpr auto CHAR_HT = '\t';
     static constexpr auto CHAR_LF = '\n';
     static constexpr auto CHAR_VT = '\v';
     static constexpr auto CHAR_FF = '\f';
     static constexpr auto CHAR_CR = '\n';
     static constexpr auto CHAR_BB = '\\';
     static constexpr auto CHAR_SQ = '\'';
     static constexpr auto CHAR_DQ = '"';
     static constexpr auto CHAR_BQ = '`';

     static constexpr auto CHAR_SPACE = ' ';
     static constexpr auto CHAR_BREAK = '\n';

     static constexpr auto CHAR_SQUOTE = '\'';
     static constexpr auto CHAR_DQUOTE = '"';

    // quotes

     static constexpr auto SQUOTE = "'";
     static constexpr auto DQUOTE = "\"";
     static constexpr auto BQUOTE = "`";
     static constexpr auto BACKSLASH = "\\";

    // commments

     static constexpr auto COMMENT_LINE = "//";
     static constexpr auto COMMENT_START = "/*";
     static constexpr auto COMMENT_END = "*/";

    // seperators

     static constexpr auto STRING_COLON = ":";
     static constexpr auto STRING_DCOLON = "::";
     static constexpr auto STRING_SEMICOLON = ";";
     static constexpr auto STRING_DSEMICOLON = ";;";
     static constexpr auto STRING_DOT = ".";
     static constexpr auto STRING_COMMA = ",";
     static constexpr auto STRING_BAR = "|";
     static constexpr auto STRING_HASH = "#";
     static constexpr auto STRING_UNDER = "_";

     static constexpr auto STRING_LPAREN = "(";
     static constexpr auto STRING_RPAREN = ")";
     static constexpr auto STRING_LSQUARE = "[";
     static constexpr auto STRING_RSQUARE = "]";
     static constexpr auto STRING_LCURLY = "{";
     static constexpr auto STRING_RCURLY = "}";

    // expressions

     static constexpr auto STRING_EQUAL = "=";
     static constexpr auto STRING_ASSIGN = "<-";
     static constexpr auto STRING_LAMBDA = "\\";
     static constexpr auto STRING_ARROW = "->";
     static constexpr auto STRING_DARROW = "=>";
     static constexpr auto STRING_STAR = "*";
     static constexpr auto STRING_PLUS = "+";
     static constexpr auto STRING_QUESTION = "?";
     static constexpr auto STRING_BANG = "!";
     static constexpr auto STRING_SLASH = "/";

    // namespaces

     static constexpr auto STRING_IMPORT = "import";
     static constexpr auto STRING_NAMESPACE = "namespace";
     static constexpr auto STRING_USING = "using";

    // declarations

     static constexpr auto STRING_DEF = "def";
     static constexpr auto STRING_VAL = "val";
     static constexpr auto STRING_DATA = "data";

    // expressions

     static constexpr auto STRING_IF = "if";
     static constexpr auto STRING_THEN = "then";
     static constexpr auto STRING_ELSE = "else";
     static constexpr auto STRING_THROW = "throw";
     static constexpr auto STRING_TRY = "try";
     static constexpr auto STRING_CATCH = "catch";
     static constexpr auto STRING_LET = "let";
     static constexpr auto STRING_IN = "in";

    // names, combinators, ..

     static constexpr auto STRING_PART = "part";
     static constexpr auto STRING_LOWERCASE = "lowercase";
     static constexpr auto STRING_UPPERCASE = "uppercase";
     static constexpr auto STRING_NAME = "name";
     static constexpr auto STRING_VARIABLE = "variable";
     static constexpr auto STRING_IDENTIFIER = "identifier";
     static constexpr auto STRING_COMBINATOR = "combinator";
     static constexpr auto STRING_OPERATOR = "operator";

    // basic types

     static constexpr auto STRING_UNIT = "unit";
     static constexpr auto STRING_NONE = "none";

     static constexpr auto STRING_BOOL = "bool";
     static constexpr auto STRING_TRUE = "true";
     static constexpr auto STRING_FALSE = "false";

     static constexpr auto STRING_INT = "int";
     static constexpr auto STRING_FLOAT = "float";
     static constexpr auto STRING_LONG = "long";
     static constexpr auto STRING_CHAR = "char";
     static constexpr auto STRING_STRING = "string";
     static constexpr auto STRING_DOUBLE = "double";
     static constexpr auto STRING_PTR = "ptr";

     static constexpr auto STRING_TEXT = "text";

     static constexpr auto STRING_LIST = "list";
     static constexpr auto STRING_NIL = "nil";
     static constexpr auto STRING_CONS = "cons";

     static constexpr auto STRING_TUPLE = "tuple";
     static constexpr auto STRING_CLASS = "class";
     static constexpr auto STRING_OBJECT = "object";
     static constexpr auto STRING_EXTEND = "extend";    // this is the combinator
     static constexpr auto STRING_EXTENDS = "extends";  // this is the keyword
     static constexpr auto STRING_WITH = "with";

     static constexpr auto STRING_FAIL = "fail";

    // System

     static constexpr auto STRING_SYSTEM = "System";
     static constexpr auto STRING_K = "k";
     static constexpr auto STRING_ID = "id";

    // main section

     static constexpr auto STRING_MAIN = "main";

    // compiler phases

     static constexpr auto STRING_INTERNAL = "internal";
     static constexpr auto STRING_IO = "input/output";
     static constexpr auto STRING_LEXICAL = "lexical";
     static constexpr auto STRING_IDENTIFICATION = "identification";
     static constexpr auto STRING_SYNTACTICAL = "syntactical";
     static constexpr auto STRING_SEMANTICAL = "semantical";
     static constexpr auto STRING_LINKER = "linker";
     static constexpr auto STRING_CODEGEN = "code";

    // compiler errors

     static constexpr auto STRING_ERROR_REDECLARE = "redeclaration";
     static constexpr auto STRING_ERROR_UNDECLARED = "undeclared";
     static constexpr auto STRING_ERROR_DECLARED = "declared";
     static constexpr auto STRING_ERROR_NOPATTERN = "pattern expected";
     static constexpr auto STRING_ERROR_NODEF = "definition expected";
     static constexpr auto STRING_ERROR_NOEXPR = "expression expected";
     static constexpr auto STRING_ERROR_NODECL = "declaration expected";

    // lambda

     static constexpr auto STRING_CONST = "";
     static constexpr auto STRING_EMBED = "embed";
     static constexpr auto STRING_CALL = "call";
     static constexpr auto STRING_SELECT = "select";

    // readable text for token types

     static constexpr auto STRING_EOF = "end of file";
     static constexpr auto STRING_ERROR = "error";
     static constexpr auto STRING_EQ = "=";
     static constexpr auto STRING_INTEGER = "integer";
     static constexpr auto STRING_HEXINTEGER = "hexadecimal integer";

    // human readable byte opcodes

     static constexpr auto STRING_OP_ASSIGN = "assign";
     static constexpr auto STRING_OP_INC = "inc";
     static constexpr auto STRING_OP_ADD = "add";
     static constexpr auto STRING_OP_SUB = "sub";
     static constexpr auto STRING_OP_MUL = "mul";
     static constexpr auto STRING_OP_DIV = "div";
     static constexpr auto STRING_OP_MOD = "mod";
     static constexpr auto STRING_OP_LEN = "len";
     static constexpr auto STRING_OP_EQ = "eq";
     static constexpr auto STRING_OP_LT = "lt";
     static constexpr auto STRING_OP_LE = "le";
     static constexpr auto STRING_OP_NIL = "nil";
     static constexpr auto STRING_OP_MOV = "mov";
     static constexpr auto STRING_OP_LOAD = "load";
     static constexpr auto STRING_OP_DATA = "data";
     static constexpr auto STRING_OP_TAKE = "take";
     static constexpr auto STRING_OP_SPLIT = "split";
     static constexpr auto STRING_OP_TAKEX = "takex";
     static constexpr auto STRING_OP_SPLITX = "splitx";
     static constexpr auto STRING_OP_COMBINE = "combine";
     static constexpr auto STRING_OP_CONCAT = "concat";
     static constexpr auto STRING_OP_CONCATX = "concatx";
     static constexpr auto STRING_OP_TEST = "test";
     static constexpr auto STRING_OP_TAG = "tag";
     static constexpr auto STRING_OP_JMP = "jmp";
     static constexpr auto STRING_OP_BRANCH = "branch";

    // local and a magic marker for local definitions

     static constexpr auto STRING_LOCAL = "Local";
     static constexpr auto STRING_MAGIC_START = "[[";
     static constexpr auto STRING_MAGIC_END = "]]";
};
