export module constants;

#define CHAR_BEL '\a'
#define CHAR_BS '\b'
#define CHAR_HT '\t'
#define CHAR_LF '\n'
#define CHAR_VT '\v'
#define CHAR_FF '\f'
#define CHAR_CR '\n'
#define CHAR_BB '\\'
#define CHAR_SQ '\''
#define CHAR_DQ '"'
#define CHAR_BQ '`'

#define CHAR_SPACE ' '
#define CHAR_BREAK '\n'

#define CHAR_SQUOTE '\''
#define CHAR_DQUOTE '"'

// quotes

#define SQUOTE "'"
#define DQUOTE "\""
#define BQUOTE "`"
#define BACKSLASH "\\"

// commments

#define COMMENT_LINE "//"
#define COMMENT_START "/*"
#define COMMENT_END "*/"

// seperators

#define STRING_COLON ":"
#define STRING_DCOLON "::"
#define STRING_SEMICOLON ";"
#define STRING_DSEMICOLON ";;"
#define STRING_DOT "."
#define STRING_COMMA ","
#define STRING_BAR "|"
#define STRING_HASH "#"
#define STRING_UNDER "_"

#define STRING_LPAREN "("
#define STRING_RPAREN ")"
#define STRING_LSQUARE "["
#define STRING_RSQUARE "]"
#define STRING_LCURLY "{"
#define STRING_RCURLY "}"

// expressions

#define STRING_EQUAL "="
#define STRING_ASSIGN "<-"
#define STRING_LAMBDA "\\"
#define STRING_ARROW "->"
#define STRING_DARROW "=>"
#define STRING_STAR "*"
#define STRING_PLUS "+"
#define STRING_QUESTION "?"
#define STRING_BANG "!"
#define STRING_SLASH "/"

// namespaces

#define STRING_IMPORT "import"
#define STRING_NAMESPACE "namespace"
#define STRING_USING "using"

// declarations

#define STRING_DEF "def"
#define STRING_VAL "val"
#define STRING_DATA "data"

// expressions
#define STRING_IF "if"
#define STRING_THEN "then"
#define STRING_ELSE "else"
#define STRING_LET "let"
#define STRING_IN "in"
#define STRING_THROW "throw"
#define STRING_TRY "try"
#define STRING_CATCH "catch"
#define STRING_LET "let"
#define STRING_IN "in"
#define STRING_VAL "val"

// names, combinators, ..

#define STRING_PART "part"
#define STRING_LOWERCASE "lowercase"
#define STRING_UPPERCASE "uppercase"
#define STRING_NAME "name"
#define STRING_VARIABLE "variable"
#define STRING_IDENTIFIER "identifier"
#define STRING_COMBINATOR "combinator"
#define STRING_OPERATOR "operator"

// basic types

#define STRING_UNIT "unit"
#define STRING_NONE "none"

#define STRING_BOOL "bool"
#define STRING_TRUE "true"
#define STRING_FALSE "false"

#define STRING_INT "int"
#define STRING_FLOAT "float"
#define STRING_LONG "long"
#define STRING_CHAR "char"
#define STRING_STRING "string"
#define STRING_DOUBLE "double"
#define STRING_PTR "ptr"

#define STRING_TEXT "text"

#define STRING_LIST "list"
#define STRING_NIL "nil"
#define STRING_CONS "cons"

#define STRING_TUPLE "tuple"
#define STRING_CLASS "class"
#define STRING_OBJECT "object"
#define STRING_EXTEND "extend"    // this is the combinator
#define STRING_EXTENDS "extends"  // this is the keyword
#define STRING_WITH "with"

#define STRING_FAIL "fail"

// System

#define STRING_SYSTEM "System"
#define STRING_K "k"
#define STRING_ID "id"

// main section

#define STRING_MAIN "main"

// compiler phases

#define STRING_INTERNAL "internal"
#define STRING_IO "input/output"
#define STRING_LEXICAL "lexical"
#define STRING_IDENTIFICATION "identification"
#define STRING_SYNTACTICAL "syntactical"
#define STRING_SEMANTICAL "semantical"
#define STRING_LINKER "linker"
#define STRING_CODEGEN "code"

// compiler errors

#define STRING_ERROR_REDECLARE "redeclaration"
#define STRING_ERROR_UNDECLARED "undeclared"
#define STRING_ERROR_DECLARED "declared"
#define STRING_ERROR_NOPATTERN "pattern expected"
#define STRING_ERROR_NODEF "definition expected"
#define STRING_ERROR_NOEXPR "expression expected"
#define STRING_ERROR_NODECL "declaration expected"

// lambda
#define STRING_CONST "const"
#define STRING_EMBED "embed"
#define STRING_CALL "call"
#define STRING_SELECT "select"

// readable text for token types
#define STRING_EOF "end of file"
#define STRING_ERROR "error"
#define STRING_NAME "name"
#define STRING_EQ "="
#define STRING_INTEGER "integer"
#define STRING_HEXINTEGER "hexadecimal integer"
#define STRING_DARROW "=>"

// human readable byte opcodes
#define STRING_OP_ASSIGN "assign"
#define STRING_OP_INC "inc"
#define STRING_OP_ADD "add"
#define STRING_OP_SUB "sub"
#define STRING_OP_MUL "mul"
#define STRING_OP_DIV "div"
#define STRING_OP_MOD "mod"
#define STRING_OP_LEN "len"
#define STRING_OP_EQ "eq"
#define STRING_OP_LT "lt"
#define STRING_OP_LE "le"
#define STRING_OP_NIL "nil"
#define STRING_OP_MOV "mov"
#define STRING_OP_LOAD "load"
#define STRING_OP_DATA "data"
#define STRING_OP_TAKE "take"
#define STRING_OP_SPLIT "split"
#define STRING_OP_TAKEX "takex"
#define STRING_OP_SPLITX "splitx"
#define STRING_OP_COMBINE "combine"
#define STRING_OP_CONCAT "concat"
#define STRING_OP_CONCATX "concatx"
#define STRING_OP_TEST "test"
#define STRING_OP_TAG "tag"
#define STRING_OP_JMP "jmp"
#define STRING_OP_BRANCH "branch"

// local and a magic marker for local definitions
#define STRING_LOCAL "Local"
#define STRING_MAGIC_START "[["
#define STRING_MAGIC_END "]]"
