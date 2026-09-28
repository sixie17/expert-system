# AST

The Abstract Syntax Tree (AST) is the in-memory form of one rule. The
[parser](../parser/README.md) builds it from a line of text, the
[reducer](../parser/README.md#reducer) rewrites it, and later stages (knowledge
base, inference) read it.

```
A + !B => C

        =>              OpNode(IMPLIES)
       /  \
      +    C            OpNode(AND)      AtomicNode("C")
     / \
    A   !               AtomicNode("A")  NotNode
        |
        B                                AtomicNode("B")
```

All the files live in `include/expert_system/ast/` and `src/ast/`.

| Class        | Role | Children |
|--------------|------|----------|
| `IASTNode`   | interface shared by every node | none |
| `AtomicNode` | a fact, e.g `A` | none (leaf) |
| `NotNode`    | a negation, `!` | right only |
| `OpNode`     | a binary operation: `+ \| ^ => <=>` | left and right |
| `ASTVisitor` | walks a tree and prints it | |

---

## Table of contents

- [OpType](#optype)
- [IASTNode](#iastnode)
- [AtomicNode](#atomicnode)
- [NotNode](#notnode)
- [OpNode](#opnode)
- [Ownership](#ownership)
- [ASTVisitor](#astvisitor)
- [Working with nodes](#working-with-nodes)
- [Notes](#notes)

---

## OpType

`include/expert_system/ast/IASTNode.hpp`

```cpp
enum class OpType {ATOMIC, AND, OR, XOR, IMPLIES, IFaoF, NOT};
```

Every node reports one of these through `getType()`. It tells which concrete
class the node is, and for an `OpNode`, which operation it holds:

| `OpType`  | Symbol | Node class   |
|-----------|--------|--------------|
| `ATOMIC`  | `A`-`Z`| `AtomicNode` |
| `NOT`     | `!`    | `NotNode`    |
| `AND`     | `+`    | `OpNode`     |
| `OR`      | `\|`   | `OpNode`     |
| `XOR`     | `^`    | `OpNode`     |
| `IMPLIES` | `=>`   | `OpNode`     |
| `IFaoF`   | `<=>`  | `OpNode`     |

Parentheses have no `OpType`: they only decide the shape of the tree. For
example `A + (B + C)` is an `AND` whose right node is another `AND`.

---

## IASTNode

`include/expert_system/ast/IASTNode.hpp`

```cpp
class IASTNode {
public:
    virtual ~IASTNode() = default;
    virtual OpType getType() const = 0;
    virtual void accept(ASTVisitor &visitor) = 0;
};
```

The common base of every node. Trees are always handled through
`IASTNode` pointers (`std::unique_ptr<IASTNode>` to own, `IASTNode *` to look),
so a parent does not need to know what kind of child it holds.

- **`getType()`**: the node's `OpType`. This is how code decides what a node is
  before casting it, see [Working with nodes](#working-with-nodes).
- **`accept(visitor)`**: the Visitor pattern's entry point. Each node
  implements it as `visitor.visit(*this)`, so the right `visit` overload is
  called for the node's real type (double dispatch).
- **Virtual destructor**: deleting a node through an `IASTNode` pointer runs
  the derived destructor, which frees its children. This is what lets
  `std::unique_ptr<IASTNode>` own any kind of node.

---

## AtomicNode

`include/expert_system/ast/AtomicNode.hpp`, `src/ast/AtomicNode.cpp`

```cpp
class AtomicNode : public IASTNode {
public:
    AtomicNode(const std::string &sym);
    OpType getType() const override;       // always ATOMIC
    std::string getSymbol() const;         // the fact's letter, e.g "A"
    void accept(ASTVisitor &visitor) override;
};
```

A fact, and the only kind of leaf. It holds the letter it was parsed from, and
two `AtomicNode`s with the same symbol refer to the same fact. The symbol is
`const` and there is no default constructor, so an `AtomicNode` always has a
symbol that never changes.

---

## NotNode

`include/expert_system/ast/NotNode.hpp`, `src/ast/NotNode.cpp`

```cpp
class NotNode : public IASTNode {
public:
    NotNode();
    OpType getType() const override;                  // always NOT
    std::string getSymbol() const;                    // always "!"
    IASTNode *getRightNode() const;
    void setRightNode(std::unique_ptr<IASTNode> node);
    std::unique_ptr<IASTNode> releaseRightNode();
    void unsetRightNode();
    void accept(ASTVisitor &visitor) override;
};
```

A negation of whatever is under it. `!` is a prefix operator (`!A` is valid,
`A!` is not), so a `NotNode` only has a **right** node. The child can be any
node:

```
!A          NotNode → AtomicNode
!(A + B)    NotNode → OpNode(AND)
!!A         NotNode → NotNode → AtomicNode
```

A `NotNode` is created empty and its child is set right after, as the parser
does:

```cpp
auto notNode = std::make_unique<NotNode>();
notNode->setRightNode(std::make_unique<AtomicNode>("A"));   // !A
```

`getSymbol()` always returns `"!"`. It exists so that every node that is
printed has the same `getSymbol()` interface.

The child methods are described in [Ownership](#ownership).

---

## OpNode

`include/expert_system/ast/OpNode.hpp`, `src/ast/OpNode.cpp`

```cpp
class OpNode : public IASTNode {
public:
    OpNode(const std::string &sym, OpType opType);
    OpType getType() const override;
    std::string getSymbol() const;
    IASTNode *getLeftNode() const;
    IASTNode *getRightNode() const;
    void setLeftNode(std::unique_ptr<IASTNode> node);
    void setRightNode(std::unique_ptr<IASTNode> node);
    std::unique_ptr<IASTNode> releaseLeftNode();
    std::unique_ptr<IASTNode> releaseRightNode();
    void unsetLeftNode();
    void unsetRightNode();
    void accept(ASTVisitor &visitor) override;
};
```

A binary operation: `AND`, `OR`, `XOR`, `IMPLIES` or `IFaoF`. It holds:

- **`symbol`**: the text of the operator, e.g `"+"`, `"=>"`. The parser passes
  the token's text, the reducer passes the symbol matching the new operation.
- **`operation`**: the `OpType`.
- **`leftNode` / `rightNode`**: the two operands, `A` and `B` in `A + B`.

Symbol and operation are both `const`: an `OpNode` cannot change operation
after it is built. To turn `A + B` into `A | B`, a new `OpNode` is created and
the children are moved into it (this is what `Reducer::makeOp` does).

The default constructor is private, so an `OpNode` always has an operation.
Its children may be missing for a while though: the parser builds `A + B` by
first creating the `+` with only `A` as its left node, then setting `B` when it
reaches it. A finished tree always has both children set.

---

## Ownership

Each node **owns** its children through `std::unique_ptr<IASTNode>`: a tree
has exactly one owner, the pointer to its root, and destroying the root
destroys the whole tree. No node is ever shared between two parents.

`NotNode` (right only) and `OpNode` (left and right) offer the same four
operations on a child:

| Method | Ownership | Effect |
|--------|-----------|--------|
| `getXNode()` | borrows | returns a raw `IASTNode *` to look at the child. The parent still owns it: do not delete it or keep it after the parent is gone. `nullptr` if there is no child |
| `setXNode(node)` | gives | takes ownership of `node`, which must be passed with `std::move`. Any previous child is destroyed. Throws `NullException` if `node` is null |
| `releaseXNode()` | takes | returns the child as a `std::unique_ptr`, the parent is left without it. Used to move a subtree elsewhere without copying it |
| `unsetXNode()` | destroys | destroys the child, the parent is left without it |

Moving a subtree from one parent to another:

```cpp
auto child = oldParent->releaseRightNode();   // oldParent no longer has it
newParent->setLeftNode(std::move(child));     // newParent owns it now
```

The ownership rules are enforced by the compiler: a `std::unique_ptr` cannot be
copied, so passing a node to `setXNode` without `std::move`, or giving the same
node to two parents, does not compile.

---

## ASTVisitor

`include/expert_system/ast/ASTVisitor.hpp`, `src/ast/ASTVisitor.cpp`

```cpp
class ASTVisitor {
public:
    void visit(AtomicNode &node);
    virtual void visit(OpNode &node);
    virtual void visit(NotNode &node);
};
```

Walks a tree and prints it, one node per line:

```cpp
ASTVisitor visitor;
root->accept(visitor);
```

```
=>
├── +
│   ├── A
│   └── !
│       └── B
└── C
```

### How the Visitor pattern works here

`root->accept(visitor)` calls the node's own `accept`, which calls
`visitor.visit(*this)`. Since `*this` has the node's real type inside
`accept`, C++ picks the matching `visit` overload. The visitor never has to
check `getType()` or cast. Each `visit` then calls `accept` on the node's
children, which walks the whole tree.

### How the tree is drawn

The visitor keeps some state while it walks:

- **`prefix`**: what is printed before a node's branch. It grows as the walk
  goes deeper: `"│   "` below a node that has siblings after it, `"    "`
  below the last child.
- **`isLast`**: whether the node is the last child of its parent. The last
  child uses `└── `, the others `├── `.
- **`isRoot`**: the root is printed with no branch and no prefix.

Each `visit` prints the node, extends `prefix` for its children, visits them
(left then right), then restores `prefix` and `isLast` before returning.

Because `isRoot` becomes `false` after the first node, a visitor can only print
one tree. Create a new `ASTVisitor` for each tree, as `main` does.

---

## Working with nodes

Code that needs to handle each kind of node differently checks `getType()`,
then casts the `IASTNode *` to the concrete class:

```cpp
void print(const IASTNode *node) {
    switch (node->getType()) {
    case OpType::ATOMIC:
        std::cout << static_cast<const AtomicNode *>(node)->getSymbol();
        break;
    case OpType::NOT:
        std::cout << "!";
        print(static_cast<const NotNode *>(node)->getRightNode());
        break;
    default: {   // AND, OR, XOR, IMPLIES, IFaoF
        auto *op = static_cast<const OpNode *>(node);
        std::cout << "(";
        print(op->getLeftNode());
        std::cout << " " << op->getSymbol() << " ";
        print(op->getRightNode());
        std::cout << ")";
    }
    }
}
```

`static_cast` is safe here because `getType()` already guarantees which class
the node is. `dynamic_cast` also works and returns `nullptr` if the type is
wrong, which is what the tests' `toString` helper uses.

The alternative is to write an `ASTVisitor` with one `visit` per node type,
which avoids casting entirely.

---

## Notes

- **`ASTVisitor` is a printer, not a generic visitor.** Its `visit` methods
  print the tree. To add another operation over the tree (e.g evaluating it),
  either derive from it and override the `virtual` methods, or turn
  `ASTVisitor` into an abstract interface with a separate printing visitor.
- **`visit(AtomicNode &)` is not `virtual`**, unlike the two others, so a
  derived visitor cannot override how facts are handled.
- **Circular include**: `ASTVisitor.hpp` includes `NotNode.hpp`, which includes
  `ASTVisitor.hpp`. It works thanks to `#pragma once` and the forward
  declarations in `ASTVisitor.hpp`, but the include of `NotNode.hpp` there is
  not needed and could be removed.
- **`operation` member in `AtomicNode` and `NotNode`**: they store an `OpType`
  that is always the same (`ATOMIC`, `NOT`). `getType()` could return the
  constant directly.
