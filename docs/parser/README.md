# Parser

The parser turns one line of a rule file into an Abstract Syntax Tree (AST), then
rewrites that tree into a simpler form that the rest of the expert system can
work with.

```
"A + !(B | C) => D"
        │
        ▼
  ┌───────────┐   std::vector<Token>   ┌────────────┐   AST    ┌─────────────┐   reduced AST
  │   Lexer   │ ─────────────────────▶ │   Parser   │ ───────▶ │   Reducer   │ ───────────────▶
  └───────────┘                        └────────────┘          └─────────────┘
```

| Stage   | Input                | Output                         | Files |
|---------|----------------------|--------------------------------|-------|
| Lexer   | a raw line (string)  | a list of `Token`s             | `include/expert_system/parser/Lexer.hpp`, `Token.hpp`, `src/parser/Lexer.cpp` |
| Parser  | a list of `Token`s   | the root of an AST (or `nullptr`) | `include/expert_system/parser/Parser.hpp`, `src/parser/Parser.cpp` |
| Reducer | an AST               | an equivalent AST where negations only apply to facts | `include/expert_system/parser/Reducer.hpp`, `src/parser/Reducer.cpp` |

The AST node classes (`IASTNode`, `AtomicNode`, `NotNode`, `OpNode`, `ASTVisitor`)
are described in [the AST documentation](../ast/README.md). This document only
covers how they are built and rewritten.

Errors are reported with the exception classes in
`include/expert_system/exceptions/Exceptions.hpp`, see [Errors](#errors).

---

## Table of contents

- [Usage](#usage)
- [Syntax](#syntax)
- [Token](#token)
- [Lexer](#lexer)
- [Parser](#parser-1)
  - [Public interface](#public-interface)
  - [How a line is parsed](#how-a-line-is-parsed)
  - [Operands](#operands-parseoperand)
  - [Logic operations (=> and <=>)](#logic-operations--and-)
  - [Private helpers](#private-helpers)
- [Reducer](#reducer)
  - [Public interface](#public-interface-1)
  - [The polarity flag](#the-polarity-flag)
  - [Rewrite rules](#rewrite-rules)
  - [Ownership](#ownership)
- [Errors](#errors)
- [Known limitations](#known-limitations)
- [Tests](#tests)

---

## Usage

```cpp
#include "expert_system/parser/Parser.hpp"
#include "expert_system/parser/Reducer.hpp"

auto ast = Parser::parseLine("A + !(B | C) => D", lineNumber);
if (!ast)
    return;                              // empty line or comment only
auto reduced = Reducer::reduce(std::move(ast));
// reduced: ((A + (!B + !C)) => D)
```

`Parser` and `Reducer` are stateless: every method is `static` and their
constructors are deleted, so they are always used as `Parser::...` / `Reducer::...`.
`Lexer` holds the line it is reading, so it is instantiated once per line.

---

## Syntax

| Symbol | Meaning                    | Token          | AST operation     |
|--------|----------------------------|----------------|-------------------|
| `A`-`Z`| a fact                     | `VARIABLE`     | `OpType::ATOMIC`  |
| `!`    | not                        | `NOT`          | `OpType::NOT`     |
| `+`    | and                        | `AND`          | `OpType::AND`     |
| `\|`   | or                         | `OR`           | `OpType::OR`      |
| `^`    | xor                        | `XOR`          | `OpType::XOR`     |
| `=>`   | implies                    | `IMPLIES`      | `OpType::IMPLIES` |
| `<=>`  | if and only if             | `IFAOF`        | `OpType::IFaoF`   |
| `(` `)`| grouping                   | `LPAREN`, `RPAREN` | none (becomes the tree's shape) |
| `=`    | initial facts line (`=ABG`)| `INITIAL_FACTS`| none, not parsed yet |
| `?`    | queries line (`?GVX`)      | `QUERIES`      | none, not parsed yet |
| `#`    | comment until end of line  | none           | none              |

Whitespace is ignored anywhere on the line.

---

## Token

`include/expert_system/parser/Token.hpp`

```cpp
enum class TokenType {
    VARIABLE, AND, OR, XOR, NOT, IMPLIES, IFAOF,
    LPAREN, RPAREN, INITIAL_FACTS, QUERIES, END_OF_LINE
};

struct Token {
    TokenType type;
    std::string value;          // the source text, e.g "A", "=>", "+"
    std::string toString() const;
};
```

A token is the smallest meaningful piece of a line. `value` keeps the original
text so the parser can reuse it as the symbol of an `OpNode` or `AtomicNode`.
`END_OF_LINE` has an empty value and is always the last token of a line.

---

## Lexer

`include/expert_system/parser/Lexer.hpp`, `src/parser/Lexer.cpp`

```cpp
class Lexer {
public:
    Lexer(std::string input_text);
    std::vector<Token> tokenize();
};
```

The lexer walks the line one character at a time, keeping its position in `pos`.

| Method                 | Role |
|------------------------|------|
| `current()`            | character at `pos`, or `'\0'` past the end |
| `peek(offset)`         | character at `pos + offset` without moving, or `'\0'` past the end |
| `advance(count = 1)`   | moves `pos` forward |
| `skipWhitespace()`     | advances over any `std::isspace` characters |
| `tokenize()`           | produces the full token list |

### `tokenize()`

For every position, after skipping whitespace, the checks run in this order:

1. `#`: the rest of the line is a comment, stop.
2. Multi-character operators, longest first: `<=>`, then `=>`. This order matters,
   otherwise `<=>` would never be recognized and `=>` would be read as `=` + `>`.
3. `=` alone: `INITIAL_FACTS`.
4. Single-character symbols: `?`, `+`, `|`, `^`, `!`, `(`, `)`.
5. An uppercase letter: a `VARIABLE` whose value is that letter. Facts are
   always a single letter, so `AB` gives two variables.
6. Anything else throws `std::runtime_error`, e.g
   `Lexer Error: Unrecognized character '&' at index 2`. Lowercase letters and
   digits are rejected here.

Finally an `END_OF_LINE` token is appended, so the token list is never empty.

```
"A + B => C # comment"
→ VARIABLE(A) AND(+) VARIABLE(B) IMPLIES(=>) VARIABLE(C) END_OF_LINE
```

The lexer only checks that each character is valid. Whether the tokens form a
valid rule (e.g `A + + B`) is checked by the parser.

---

## Parser

`include/expert_system/parser/Parser.hpp`, `src/parser/Parser.cpp`

### Public interface

```cpp
class Parser {
public:
    [[nodiscard]] static std::unique_ptr<IASTNode> parseLine(const std::string &line, size_t lineNumber);
    [[nodiscard]] static std::unique_ptr<IASTNode> parseTokens(std::span<const Token> tokens);
};
```

- **`parseLine(line, lineNumber)`**: lexes and parses one line. Any exception
  thrown while lexing or parsing is caught and rethrown as a `ParseError` that
  carries `lineNumber` (1-based), and whose message starts with `line N: `.
  This is the entry point used by `main`.
- **`parseTokens(tokens)`**: parses an already tokenized line. It does not wrap
  errors, which lets tests check the original exception type. It simply calls
  the private `parseTokens(tokens, true)`.

Both return `nullptr` for an empty line or a line that only holds a comment.

### How a line is parsed

The private `parseTokens(tokens, allowLogicOp)` reads the tokens left to right
and builds the tree with a stack of nodes (`NodeStack`, a
`std::stack<std::unique_ptr<IASTNode>>`). At any time the top of the stack is
the tree built so far:

| Token                      | Action |
|----------------------------|--------|
| `VARIABLE`, `NOT`, `(`     | an operand starts: `parseOperand` builds it, `attachOperand` places it |
| `+`, `\|`, `^`             | pop the top as the left node of a new `OpNode`, push the `OpNode` (its right node is still missing) |
| `=>`, `<=>`                | split the line, see [Logic operations](#logic-operations--and-) |
| `)`                        | a `)` without a `(`: throws `UnclosedParenthese` |
| `END_OF_LINE`              | stop |

`attachOperand` either pushes the operand onto an empty stack, or sets it as
the right node of the pending `OpNode` on top of it.

Example, `A + B | C`:

```
token   stack (top)              action
A       A                        stack empty, pushed
+       (A + ?)                  A popped as left node, OpNode pushed
B       (A + B)                  set as the right node of the pending +
|       ((A + B) | ?)            (A + B) popped as left node, OpNode pushed
C       ((A + B) | C)            set as the right node of the pending |
```

When the tokens run out, the top of the stack is the result. If it is an
`OpNode` still missing its right node (e.g `A +`), `MissingOperationValue` is
thrown.

Because each operator takes the whole tree built so far as its left node,
operators are **left-associative with no precedence**: `A + B + C` is
`((A + B) + C)` and `A | B + C` is `((A | B) + C)`. Use parentheses to group
differently. See [Known limitations](#known-limitations).

### Operands (`parseOperand`)

```cpp
static std::unique_ptr<IASTNode> parseOperand(TokenIt &it, TokenIt end);
```

Builds one operand starting at `it` and leaves `it` on the operand's last token,
so the loop in `parseTokens` continues right after it. An operand is:

- **`VARIABLE`**: an `AtomicNode` holding the letter.
- **`NOT`**: a `NotNode` whose right node is the next operand, parsed by calling
  `parseOperand` again. This recursion is what makes `!!!A` and `!(A + B)` work.
  `!` only applies to the operand directly after it: `!A + B` is `(!A + B)`.
- **`(`**: finds the matching `)` by counting depth (`(` +1, `)` -1, stop at 0),
  then parses the tokens in between with `parseTokens(inner, true)`. The group
  comes back as a single subtree, so parentheses leave no node in the AST, only
  a shape: `A + (B + C)` is `(A + (B + C))`. `()` throws `MissingOperationValue`,
  a missing `)` throws `UnclosedParenthese`.

Anything else where an operand is expected (an operator, `)`, end of line)
throws `MissingOperationValue`, e.g `A !=> B`, `!`, `+ A`.

### Logic operations (`=>` and `<=>`)

`=>` and `<=>` are handled differently from `+ | ^`: they **split the current
level in two**. Everything already on the stack becomes the left node, and all
the remaining tokens are parsed as the right node:

```
A + B => C + D
└─ left ─┘  └─ right ─┘      →   ((A + B) => (C + D))
```

So a logic operation always ends up as the root of its level, regardless of
what comes before or after it. A level is the whole line, or the inside of a
pair of parentheses.

The right side is parsed with `allowLogicOp = false`: a second `=>`/`<=>` on
the same level would be ambiguous (`A => B => C`), so it throws
`InvalidLogicOperation`. Parentheses start a new level where it is allowed
again, which is how nested logic operations are written:

| Line                   | Result |
|------------------------|--------|
| `A => B => C`          | `InvalidLogicOperation` |
| `A => (B => C)`        | `(A => (B => C))` |
| `(A => B) \| (A => C)` | `((A => B) \| (A => C))` |
| `!(A => B) => C`       | `(!(A => B) => C)` |

Before building the node, the left side is checked: it must exist (`=> B`
throws) and must not be an `OpNode` missing its right node (`A + => B` throws).
The right side must not be empty (`A =>` throws). All three throw
`MissingOperationValue`.

### Private helpers

| Function | Role |
|----------|------|
| `isOpToken(token)` | true for tokens that are not an operand, a parenthesis or `END_OF_LINE` |
| `isOpNode(node)` | true if the node is an `OpNode` (not `ATOMIC` nor `NOT`), meaning it has a left and a right node |
| `isLogicOperation(token)` | true for `IMPLIES` and `IFAOF` |
| `tokenToOp(type)` | `constexpr` map from `TokenType` to `OpType`, `std::nullopt` for tokens with no operation (`(`, `)`, `=`, `?`, end of line) |
| `attachOperand(node, stack)` | pushes `node` onto an empty stack, otherwise sets it as the right node of the pending `OpNode` on top. Throws `MissingOperation` when there is nothing to attach to (`A B`, `A (B)`) |

---

## Reducer

`include/expert_system/parser/Reducer.hpp`, `src/parser/Reducer.cpp`

The parser keeps the tree exactly as written. The reducer rewrites it into an
equivalent tree in **negation normal form**: every `!` applies directly to a
fact, never to a group. Double negations disappear and negated groups are
distributed.

```
!!A => B              →   (A => B)
!(A + B) => C         →   ((!A | !B) => C)
A + !(B | !C) => D    →   ((A + (!B + C)) => D)
```

This way the inference engine only has to handle `!A`, never `!(...)`.

### Public interface

```cpp
class Reducer {
public:
    [[nodiscard]] static std::unique_ptr<IASTNode> reduce(std::unique_ptr<IASTNode> root);
};
```

`reduce` takes ownership of the tree and returns the reduced tree. The input
pointer must not be used afterwards: some of its nodes are reused, others are
destroyed. It throws `NullException` if `root` is null.

### The polarity flag

All the work is done by the private overload:

```cpp
static std::unique_ptr<IASTNode> reduce(std::unique_ptr<IASTNode> node, bool negated);
```

`negated` is true when the node sits under an odd number of `!`. The public
`reduce(root)` starts with `negated = false`. The tree is walked once, top to
bottom:

| Node          | `negated == false`                     | `negated == true` |
|---------------|----------------------------------------|-------------------|
| `ATOMIC`      | returned as is                         | wrapped in a new `NotNode` |
| `NOT`         | dropped, child reduced with `true`     | dropped, child reduced with `false` |
| binary op     | same node, children reduced with `false` | rewritten, see [Rewrite rules](#rewrite-rules) |

The `NotNode` itself never survives the walk: it only flips the flag for its
child. This is what makes double negations cancel with no special case:

```
!!!A   reduce(NOT, false) → reduce(NOT, true) → reduce(NOT, false) → reduce(A, true) → !A
```

A negated fact always gets a new `NotNode` because, after distribution, there
is usually no `NotNode` above it to reuse: `!(A + B)` has one `NotNode` but its
result `(!A | !B)` needs two.

Any `OpType` that is not `ATOMIC`, `NOT` or a binary operation (checked by the
file-local `isBinaryOp`) throws `InvalidOperation`.

### Rewrite rules

When a binary operation is reached with `negated == true`, the negation is
pushed into its children using these equivalences:

| Input         | Output       | Left child | Right child |
|---------------|--------------|------------|-------------|
| `!(A + B)`    | `!A \| !B`   | negated    | negated     |
| `!(A \| B)`   | `!A + !B`    | negated    | negated     |
| `!(A ^ B)`    | `!A ^ B`     | negated    | not negated |
| `!(A => B)`   | `A + !B`     | not negated| negated     |
| `!(A <=> B)`  | `A ^ B`      | not negated| not negated |

The first two are De Morgan's laws. The last three keep the tree the same size:
XOR and `<=>` are kept rather than expanded into `+`/`|`, which would require
copying `A` and `B`.

Each child is then reduced with its own flag, so negations inside the group
combine with the one being distributed:

```
!(A + !B)       →   (!A | B)          (!!B cancels)
!(A + (B | C))  →   (!A | (!B + !C))  (distributed twice)
```

A logic operation that is not negated is left as is: the rule's own `=>`/`<=>`
stays the root, and only its two sides are reduced. A negation in a conclusion
is still distributed: `A => !(B + C)` becomes `(A => (!B | !C))`.

Reducing an already reduced tree changes nothing.

### Ownership

Nodes own their children through `std::unique_ptr`, and the reducer moves
subtrees around instead of copying them:

- Children are taken out with `releaseLeftNode()` / `releaseRightNode()`,
  reduced, and moved back or into a new node.
- When the operation does not change, the same `OpNode` is reused.
- When it does (`+` ↔ `|`, `=>` → `+`, `<=>` → `^`), a new `OpNode` is built by
  `makeOp(type, left, right)`, since an `OpNode`'s type is `const`. `makeOp`
  also sets the symbol (`+`, `|` or `^`) and throws `InvalidOperation` for any
  other type.
- Dropped `NotNode`s and replaced `OpNode`s are freed automatically when their
  `unique_ptr` goes out of scope.

No subtree is ever duplicated, so the reduced tree is never larger than the
input plus one `NotNode` per negated fact.

---

## Errors

All exceptions are defined in `include/expert_system/exceptions/Exceptions.hpp`
and derive from `std::exception`.

| Exception               | Thrown by | When | Example |
|-------------------------|-----------|------|---------|
| `std::runtime_error`    | `Lexer::tokenize` | unknown character | `A & B`, `a + B` |
| `MissingOperationValue` | `parseOperand`, `parseTokens` | an operand is missing | `A +`, `=> B`, `()`, `!` |
| `MissingOperation`      | `attachOperand` | two operands with no operator between them | `A B`, `A (B)` |
| `UnclosedParenthese`    | `parseOperand`, `parseTokens` | unbalanced parentheses | `(A + B`, `A)` |
| `InvalidLogicOperation` | `parseTokens` | a second `=>`/`<=>` on the same level | `A => B => C` |
| `ParseError`            | `parseLine` | wraps any of the above with the line number, `getLineNumber()` returns it | `line 7: ...` |
| `NullException`         | `Reducer::reduce` | null tree | `Reducer::reduce(nullptr)` |
| `InvalidOperation`      | `Reducer::reduce`, `Reducer::makeOp` | unexpected `OpType` | |

---

## Known limitations

- **No operator precedence.** Operators are applied left to right, so
  `A | B + C` gives `((A | B) + C)` rather than `(A | (B + C))`. Parentheses
  must be used to force a grouping.
- **Initial facts and queries are not parsed.** The lexer produces
  `INITIAL_FACTS` and `QUERIES` tokens, but `parseLine("=ABG")` and
  `parseLine("?GVX")` currently throw `MissingOperationValue`. Those lines need
  their own handling before the rule parser.
- **Exception messages** are built by concatenating the method name and the
  error name without a separator, e.g `parseTokensMissingOperationValue`.

---

## Tests

| File                     | Covers |
|--------------------------|--------|
| `tests/testParsing.cpp`  | valid lines (tree shape), empty lines, every error type, `parseLine` wrapping |
| `tests/testReducer.cpp`  | unchanged trees, cancellation, distribution of every operation, negations inside distributed groups, rules, idempotence, null input |

Both compare trees through a `toString` helper that prints them fully
parenthesized, e.g `((A + !B) => C)`. The tests use the small `TestSuite` in
`tests/TestSuite.hpp`, which prints every passing and failing case.

```sh
make test                                          # every test executable
make build/tests/testReducer && ./build/tests/testReducer
```
