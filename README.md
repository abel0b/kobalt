# Kobalt
programming language project

## Grammar
```
Source      = (Expression | Declaration | "\n") Comment? ("\n" Source)?
Comment     = "--" :text
Declaration = TYPE :space :identifier (:equal Expression)?
Type        = ("int" | "float")
Expression  = (:identifier "(" Parameters ")" | PExpression)
Assignment  = :identifier "=" Expression
PExpression = (Expression) | :float | :integer
Parameters  = (:identifier :equal:)? PExpression (" " Parameters)?

text       = [a-zA-Z0-9_\-]
identifier = [a-zA-Z_]+[a-zA-Z0-9_\+\-\*=]*
integer    = [0-9]+
float      = :integer\.:integer
```
