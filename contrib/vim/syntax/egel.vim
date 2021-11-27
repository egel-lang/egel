" Vim syntax file
" Language:	egel
" Filenames:	*.eg
" Maintainer:	Marco Devillers - marco(.)devillers(@)gmail(.)com
" Credits:	
" Last Change:	
" Change History:

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn case match

syn keyword egelKeyword			if then else
syn keyword egelKeyword			try catch throw
syn keyword egelKeyword			data def val
syn keyword egelKeyword			class extends with
syn keyword egelKeyword			let in 
syn keyword egelDirective		import using
syn keyword egelNamespace		namespace

syn keyword egelTodo			contained  TODO XXX

syn match egelOperator			"[-+]"

syn match egelChar			"'.'"
syn region egelText			start=+\"+ end=+\"+	oneline skip=+\"\"+
syn match egelNumber			"\<\d\+[ij]\=\>"
syn match egelFloat			"\<\d\+\(\.\d+\)\=\([eE][-]\=\d\+\)\=[ij]\=\>"

syn match egelTab			"\t"

syn match egelArrow			"->"
syn match egelSemicolon			";"
syn match egelDot			"\."
syn match egelComma			","
syn match egelBar			"|"
syn match egelQuestion			"?"
syn match egelSquare			"[][]"
syn match egelParen			"[()]"

syn match egelComment			"#.*$"	contains=egelTodo,egelTab

syn region egelText matchgroup=egelTripleQuotes
      \ start=+[uU]\=\z('''\|"""\)+ end="\z1" keepend

"syn match egelError	"-\=\<\d\+\.\d\+\.[^*/\\^]"
"syn match egelError	"-\=\<\d\+\.\d\+[eEdD][-+]\=\d\+\.\([^*/\\^]\)"

if !exists("did_egel_syntax_inits")
  let did_egel_syntax_inits = 1
  command -nargs=+ HiLink hi link <args>

  HiLink egelArrow			SpecialChar
  HiLink egelSemicolon			SpecialChar
  HiLink egelDot			SpecialChar
  HiLink egelComma			SpecialChar
  HiLink egelBar			SpecialChar
  HiLink egelQuestion			SpecialChar
  HiLink egelSquare			SpecialChar
  HiLink egelParen			SpecialChar

  HiLink egelKeyword			Keyword
  HiLink egelDirective			Include
  HiLink egelNamespace			Type
  HiLink egelOperator			Operator
  HiLink egelTodo			Todo
  HiLink egelTab			Todo
  HiLink egelChar			String
  HiLink egelText			String
  HiLink egelNumber			Number
  HiLink egelFloat			Float
  HiLink egelError			Error
  HiLink egelComment			Comment

"optional highlighting
  HiLink egelIdentifier			Identifier
  HiLink egelTab			Error

  delcommand HiLink
endif

let b:current_syntax = "egel"

"EOF	vim: ts=8 noet tw=100 sw=8 sts=0
