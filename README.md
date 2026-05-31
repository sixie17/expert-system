# Expert System

An expert system for propositional logic written in C++.

## Why This Is Challenging

Propositional logic inference can quickly become complex when mapping out a dense knowledge base. Rules are not isolated pathways; they interact. A rule often shares sub-expressions with other rules or combines existing conditions. 

For example, when multiple rules share common antecedents or consequences, evaluating them independently leads to redundant work. Evaluating whether `A` and `B` are true might be needed for many separate propositions. Furthermore, handling equivalences like `(A & B) <=> C` means that any information deduced about `A & B` must instantly reflect onto `C` and vice versa. As the number of facts and complex rules grows, a traditional linear rule engine or simple tree-based evaluator struggles to keep equivalent statements aligned and avoid repetitive deduction loops.

## Approach

This project abandons the pure linear or recursive tree evaluation in favor of an **e-graph / hypergraph** representation.

- **E-graph (Equivalence Graph):** Groups equivalent logical expressions together. If `(A & B) <=> C`, they become part of the same equivalence class in the graph.
- **Hypergraph:** Captures the complex logical relationships—where multiple facts combine to satisfy a single condition, or where one rule branch depends on multiple other independent rules.
- **Shared Substructure:** Common sub-expressions (like `A & B`) are stored as single nodes. The system reuses the evaluation of this node across all rules that depend on it.

This model perfectly aligns with propositional logic, where exactly the same logical meaning can span multiple rules. By normalizing facts into shared nodes and tying them with equivalence classes, the system reasons over a minimal, unified graph instead of resolving the same derivations repeatedly.

## What This System Is Meant To Do

- Store propositions, facts, and rules in a compact graph-based form.
- Detect and reuse equivalent logical expressions.
- Support sub-rule and sub-case reasoning.
- Propagate knowledge across equivalent forms of the same statement.
- Provide a foundation for scalable inference in C++.

## Design Goal

The goal is not just to answer whether a statement is true or false. The goal is to build a reasoning core that can explain how one proposition relates to another, how equivalence changes what can be inferred, and how shared structure can reduce redundant work.

## Planned Direction

- Parse propositional expressions into an internal graph representation.
- Canonicalize equivalent forms where possible.
- Track implication and equivalence relationships across the graph.
- Use the shared structure to drive inference, simplification, and query answering.

## Summary

An e-graph and hypergraph approach addresses the core difficulties of propositional logic inference: redundant evaluation of shared conditions and managing cyclic or equivalent logical rules. Modeling the knowledge base as a unified graph provides a natural, scalable way to propagate truth values and answer queries without repeating deductions.

