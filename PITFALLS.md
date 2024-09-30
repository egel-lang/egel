# Common Pifalls when Programming in Egel 

Egel is a minimalist language, reading the man page should suffice to
start programming in Egel.

However, I expect people to encounter the following common pitfalls.

## Wrong usage of patterns

The following two patterns are very different. The first defines a
function that takes two arguments, the latter defines a function that
takes one argument, that one argument being the composite of two terms.

```
    [X Y -> ... ] [(X Y) -> ... ]
```

## Forget to open namespace in interactie mode

In interactive mode the prelude is loaded and the System and List
namespaces are opened. For the other namespace use:

```
    >> using String;; using Math
```

## No short-circuited boolean operators

The `&&` and `||` combinators take functions as their second argument.
Use in the following manner:

```
    true || [_ -> false && [_ -> true]]
```

## Passing variadic functions

The following will not have the effect that one would assume it has.

```
    map print {"hello", 3.14}
```

Print is a variadic function and Egel has eager semantics.  Given that
`print` is variadic it will print nothing and reduce to `none` after
which that will be applied to the constants in the list.

The following is safe, however.

```
    map [X -> print X] {"hello", 3.14}
```

## Math operators only work on floats

Despite Egel being a dynamic language, the combinators defined in the
Math namespace exclusively work on floats. Look in the prelude or cast
with `to_float` and `to_int` combinators.

## Overlapping usings

When two namespaces `A` and `B` both define `foo` and both namespaces are
opened, `foo` will be redeclared and give a runtime error.  Handle with
care.

```
    namespace A ( data foo ) namespace B ( data foo ) using A using B
```
