#pragma once

class ASTVisitor;

/**
 * @brief Represents the types of operations and symbols in the AST.
 */
enum class OpType {ATOMIC, AND, OR, XOR, IMPLIES, IFaoF, NOT};

/**
 * @brief Base interface for all Abstract Syntax Tree (AST) nodes.
 * 
 * Defines the common behavior for AST entities in the expert system,
 * notably enabling the Visitor pattern for operations like inference 
 * or structure generation.
 */
class IASTNode {
    public:
        /**
         * @brief Virtual destructor to ensure proper cleanup of derived types.
         */
        virtual ~IASTNode() = default;
    
        /**
         * @brief Gets the operation type of the node.
         * @return The OpType representing this node.
         */
        virtual OpType getType() const = 0;

        /**
         * @brief Accepts a visitor to perform an operation on this node (Double Dispatch).
         * @param visitor The visitor executing logic on the AST.
         */
        virtual void accept(ASTVisitor &visitor) = 0;

};
