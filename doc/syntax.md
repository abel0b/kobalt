# Syntax

## Lexic

## Grammar

```
file       -> toplevel ENDF
toplevel   -> (fun|todo) [toplevel]
fun        -> ID"fun" ID LPAR funargs RPAR EQ expr
funargs    -> [var|val] ID COLON type [funargs]
expr       -> (decl|assign|seq|block)
seq        -> expr SEMI expr
block      -> INDENT expr DEDENT
decl       -> ID=(var|val) ID [COLON type] [EQUAL expr]
type       -> ID
literal    -> (FLOAT|INT|STR)
expr       -> (binexp|term)
term       -> (LPAR expr RPAR|literal)
funcall    -> ID funparams
funparams  -> [ID EQUAL] expr [params]
assign     -> ID EQUAL expr

binexp     -> term (PLUS|MINUS|STAR|SLASH) term
tuple      -> LPAR typelst RPAR
typelst    -> [type] [typelst]
struct     -> LBRACK structbody RBRACK
structbody -> (INDENT structits DEDENT|structits)
structits  -> [SEMI] [ID COLON type] [structits]
if
elif
else
case
let
for
while
gen
macro
```
## Keywrds

### Hard keywords
These are reserved words, they can't be used as identifier.
- if
- elif
- else
- for
- while
- true
- false

### Soft Keywords
These are considered keywords only in specific contexts, otherwise they can be use as identifier.

- type
- template
- struct
- union
- macro
- define
- tpl
- def
- obj
