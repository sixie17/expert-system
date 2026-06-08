#pragma once
#include "IASTNode.hpp"
#include "ASTVisitor.hpp"

/**
 * @brief Represents an atomic fact or proposition in the knowledge base.
 *
 * Atomic nodes represent base variables (e.g., "A", "B") and are the
 * leaf nodes of the Abstract Syntax Tree.
 */
class AtomicNode : public IASTNode {
    private:
        const std::string symbol;
        const OpType operation;
        AtomicNode();
    public:
        /**
         * @brief Virtual destructor.
         */
        ~AtomicNode() = default;

        /**
         * @brief Constructs an AtomicNode with a given symbol.
         * @param sym The string representation of this atomic fact.
         */
        AtomicNode(const std::string &sym);

        /**
         * @brief Gets the node type.
         * @return Always returns OpType::ATOMIC.
         */
        OpType getType() const override;

        /**
         * @brief Gets the symbol associated with this fact.
         * @return A string representing this atomic fact.
         */
        std::string getSymbol() const;

        /**
         * @brief Accepts a visitor for processing this node's atomic logic.
         * @param visitor The visitor being accepted.
         */
        void accept(ASTVisitor &visitor) override;
};