# Kobalt
[![pipeline status](https://gitlab.com/abeliam/kobalt/badges/master/pipeline.svg)](https://gitlab.com/abeliam/kobalt/commits/master) [![coverage report](https://gitlab.com/abeliam/kobalt/badges/master/coverage.svg)](https://gitlab.com/abeliam/kobalt/commits/master)
programming language project

## Grammar
```
Source      = (Assignment | Expression | Declaration | "\n") Comment? ("\n" Source)?
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
