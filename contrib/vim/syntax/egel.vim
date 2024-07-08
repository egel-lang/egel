" Vim syntax file
" Language:	egel
" Filenames:	*.eg
" Maintainer:	Marco Devillers - marco(.)devillers(@)gmail(.)com
" Credits:	
" Last Change:	
" Change History:

if exists("b:current_syntax")
   finish
endif

syn case match

syn keyword egelKeyword			if then else let in try catch throw do
syn keyword egelKeyword			data def val
syn keyword egelDirective		import using namespace

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

syn region egelText matchgroup=egelTripleQuotes start=+[uU]\=\z('''\|"""\)+ end="\z1" keepend

"syn match egelError	"-\=\<\d\+\.\d\+\.[^*/\\^]"
"syn match egelError	"-\=\<\d\+\.\d\+[eEdD][-+]\=\d\+\.\([^*/\\^]\)"

if !exists("did_egel_syntax_inits")
  let did_egel_syntax_inits = 1

  highlight default link egelArrow			SpecialChar
  highlight default link egelSemicolon			SpecialChar
  highlight default link egelDot			SpecialChar
  highlight default link egelComma			SpecialChar
  highlight default link egelBar			SpecialChar
  highlight default link egelQuestion			SpecialChar
  highlight default link egelSquare			SpecialChar
  highlight default link egelParen			SpecialChar

  highlight default link egelKeyword			Keyword
  highlight default link egelDirective			Include
  highlight default link egelNamespace			Type
  highlight default link egelOperator			Operator
  highlight default link egelTodo			Todo
  highlight default link egelTab			Todo
  highlight default link egelChar			String
  highlight default link egelText			String
  highlight default link egelNumber			Number
  highlight default link egelFloat			Float
  highlight default link egelError			Error
  highlight default link egelComment			Comment

  highlight default link egelIdentifier		Identifier
  highlight default link egelTab			Error

endif

let b:current_syntax = "egel"

"EOF	vim: ts=8 noet tw=100 sw=8 sts=0
