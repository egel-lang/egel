#pragma once

namespace egel {

inline constexpr auto CHAR_BEL = '\a';
inline constexpr auto CHAR_BS = '\b';
inline constexpr auto CHAR_HT = '\t';
inline constexpr auto CHAR_LF = '\n';
inline constexpr auto CHAR_VT = '\v';
inline constexpr auto CHAR_FF = '\f';
inline constexpr auto CHAR_CR = '\n';
inline constexpr auto CHAR_BB = '\\';
inline constexpr auto CHAR_SQ = '\'';
inline constexpr auto CHAR_DQ = '"';
inline constexpr auto CHAR_BQ = '`';

inline constexpr auto CHAR_SPACE = ' ';
inline constexpr auto CHAR_BREAK = '\n';

inline constexpr auto CHAR_SQUOTE = '\'';
inline constexpr auto CHAR_DQUOTE = '"';

// quotes

inline constexpr auto SQUOTE = "'";
inline constexpr auto DQUOTE = "\"";
inline constexpr auto BQUOTE = "`";
inline constexpr auto BACKSLASH = "\\";

// commments

inline constexpr auto COMMENT_LINE = "//";
inline constexpr auto COMMENT_START = "/*";
inline constexpr auto COMMENT_END = "*/";

// seperators

inline constexpr auto STRING_COLON = ":";
inline constexpr auto STRING_DCOLON = "::";
inline constexpr auto STRING_SEMICOLON = ";";
inline constexpr auto STRING_DSEMICOLON = ";;";
inline constexpr auto STRING_DOT = ".";
inline constexpr auto STRING_COMMA = ",";
inline constexpr auto STRING_BAR = "|";
inline constexpr auto STRING_HASH = "#";
inline constexpr auto STRING_UNDER = "_";

inline constexpr auto STRING_LPAREN = "(";
inline constexpr auto STRING_RPAREN = ")";
inline constexpr auto STRING_LSQUARE = "[";
inline constexpr auto STRING_RSQUARE = "]";
inline constexpr auto STRING_LCURLY = "{";
inline constexpr auto STRING_RCURLY = "}";

// expressions

inline constexpr auto STRING_EQUAL = "=";
inline constexpr auto STRING_ASSIGN = "<-";
inline constexpr auto STRING_LAMBDA = "\\";
inline constexpr auto STRING_ARROW = "->";
inline constexpr auto STRING_DARROW = "=>";
inline constexpr auto STRING_STAR = "*";
inline constexpr auto STRING_PLUS = "+";
inline constexpr auto STRING_QUESTION = "?";
inline constexpr auto STRING_BANG = "!";
inline constexpr auto STRING_SLASH = "/";

// namespaces

inline constexpr auto STRING_IMPORT = "import";
inline constexpr auto STRING_NAMESPACE = "namespace";
inline constexpr auto STRING_USING = "using";

// declarations

inline constexpr auto STRING_DEF = "def";
inline constexpr auto STRING_VAL = "val";
inline constexpr auto STRING_DATA = "data";

// expressions

inline constexpr auto STRING_IF = "if";
inline constexpr auto STRING_THEN = "then";
inline constexpr auto STRING_ELSE = "else";
inline constexpr auto STRING_THROW = "throw";
inline constexpr auto STRING_TRY = "try";
inline constexpr auto STRING_CATCH = "catch";
inline constexpr auto STRING_LET = "let";
inline constexpr auto STRING_IN = "in";

inline constexpr auto STRING_HANDLE = "handle";
// do sugar

inline constexpr auto STRING_DO = "do";

// names, combinators, ..

inline constexpr auto STRING_PART = "part";
inline constexpr auto STRING_LOWERCASE = "lowercase";
inline constexpr auto STRING_UPPERCASE = "uppercase";
inline constexpr auto STRING_NAME = "name";
inline constexpr auto STRING_VARIABLE = "variable";
inline constexpr auto STRING_IDENTIFIER = "identifier";
inline constexpr auto STRING_COMBINATOR = "combinator";
inline constexpr auto STRING_OPERATOR = "operator";

// basic types

inline constexpr auto STRING_UNIT = "unit";
inline constexpr auto STRING_NONE = "none";

inline constexpr auto STRING_BOOL = "bool";
inline constexpr auto STRING_TRUE = "true";
inline constexpr auto STRING_FALSE = "false";

inline constexpr auto STRING_INT = "int";
inline constexpr auto STRING_FLOAT = "float";
inline constexpr auto STRING_LONG = "long";
inline constexpr auto STRING_CHAR = "char";
inline constexpr auto STRING_STRING = "string";
inline constexpr auto STRING_DOUBLE = "double";
inline constexpr auto STRING_PTR = "ptr";

inline constexpr auto STRING_TEXT = "text";

inline constexpr auto STRING_ARRAY = "array";

inline constexpr auto STRING_LIST = "list";
inline constexpr auto STRING_NIL = "nil";
inline constexpr auto STRING_CONS = "cons";

inline constexpr auto STRING_TUPLE = "tuple";
inline constexpr auto STRING_CLASS = "class";
inline constexpr auto STRING_OBJECT = "object";
inline constexpr auto STRING_EXTEND = "extend";    // this is the combinator
inline constexpr auto STRING_EXTENDS = "extends";  // this is the keyword
inline constexpr auto STRING_WITH = "with";

inline constexpr auto STRING_FAIL = "fail";

// System

inline constexpr auto STRING_SYSTEM = "System";
inline constexpr auto STRING_TIME = "Time";
inline constexpr auto STRING_K = "k";
inline constexpr auto STRING_ID = "id";

// main section

inline constexpr auto STRING_MAIN = "main";

// compiler phases

inline constexpr auto STRING_INTERNAL = "internal";
inline constexpr auto STRING_IO = "input/output";
inline constexpr auto STRING_LEXICAL = "lexical";
inline constexpr auto STRING_IDENTIFICATION = "identification";
inline constexpr auto STRING_SYNTACTICAL = "syntactical";
inline constexpr auto STRING_SEMANTICAL = "semantical";
inline constexpr auto STRING_LINKER = "linker";
inline constexpr auto STRING_CODEGEN = "code";

// compiler errors

inline constexpr auto STRING_ERROR_REDECLARE = "redeclaration";
inline constexpr auto STRING_ERROR_UNDECLARED = "undeclared";
inline constexpr auto STRING_ERROR_DECLARED = "declared";
inline constexpr auto STRING_ERROR_NOPATTERN = "pattern expected";
inline constexpr auto STRING_ERROR_NODEF = "definition expected";
inline constexpr auto STRING_ERROR_NOEXPR = "expression expected";
inline constexpr auto STRING_ERROR_NODECL = "declaration expected";

// lambda

inline constexpr auto STRING_CONST = "";
inline constexpr auto STRING_EMBED = "embed";
inline constexpr auto STRING_CALL = "call";
inline constexpr auto STRING_SELECT = "select";

// readable text for token types

inline constexpr auto STRING_EOF = "end of file";
inline constexpr auto STRING_ERROR = "error";
inline constexpr auto STRING_EQ = "=";
inline constexpr auto STRING_INTEGER = "integer";
inline constexpr auto STRING_HEXINTEGER = "hexadecimal integer";

// local and a magic marker for local definitions

inline constexpr auto STRING_LOCAL = "Local";
inline constexpr auto STRING_MAGIC_START = "[[";
inline constexpr auto STRING_MAGIC_END = "]]";

}  // namespace egel
