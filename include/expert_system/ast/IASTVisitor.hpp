#pragma once
class AtomicNode;
class OpNode; // all other nodes will have left and right

/**
 * @brief Interface for the standard Visitor pattern on the AST.
 * 
 * Implement this interface to define operations (like evaluation or printing)
 * that traverse through the Abstract Syntax Tree.
 */
class IASTVisitor {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~IASTVisitor() = default;

        /**
         * @brief Visits an AtomicNode.
         * @param node The atomic node being visited.
         */
        virtual void visit(AtomicNode &node) = 0;

        /**
         * @brief Visits an Operation node (e.g., AND, OR).
         * @param node The operation node being visited.
         */
        virtual void visit(OpNode &node) = 0;
};
