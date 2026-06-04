#pragma once
#include "IASTNode.hpp"
#include "IASTVisitor.hpp"

/**
 * @brief Represents a logical operation in the Abstract Syntax Tree.
 *
 * OpNode acts as an intermediate node handling logical links like AND, OR, 
 * XOR, IMPLIES, etc. It manages ownership of its child subtrees.
 */
class OpNode : public IASTNode {
    private:
        const std::string symbol;
        const OpType operation;
        std::unique_ptr<IASTNode> leftNode;
        std::unique_ptr<IASTNode> rightNode;
        OpNode();
    public:
        /**
         * @brief Virtual destructor.
         */
        ~OpNode() = default;

        /**
         * @brief Constructs an Operation Node.
         * @param sym The string representation of the operation.
         * @param opType The specific logical operation type.
         */
        OpNode(const std::string &sym, OpType opType);

        /**
         * @brief Retrieves the operation type for this node.
         * @return The OpType representing the logical operation.
         */
        OpType getType() const override;

        /**
         * @brief Retrieves the string symbol of this operation.
         * @return A string representing the operation.
         */
        std::string getSymbol() const;

        /**
         * @brief Gets a non-owning pointer to the left child subtree.
         * @return Raw pointer to the left IASTNode.
         */
        IASTNode *getLeftNode() const;

        /**
         * @brief Gets a non-owning pointer to the right child subtree.
         * @return Raw pointer to the right IASTNode.
         */
        IASTNode *getRightNode() const;

        /**
         * @brief Sets and takes ownership of the right child subtree.
         * @param node Unique pointer to the right operand node.
         * @throws NullException if the provided node is null.
         */
        void setRightNode(std::unique_ptr<IASTNode> node);

        /**
         * @brief Sets and takes ownership of the left child subtree.
         * @param node Unique pointer to the left operand node.
         * @throws NullException if the provided node is null.
         */
        void setLeftNode(std::unique_ptr<IASTNode> node);

        /**
         * @brief Detaches/deletes the left child subtree.
         */
        void unsetLeftNode();

        /**
         * @brief Detaches/deletes the right child subtree.
         */
        void unsetRightNode();

        /**
         * @brief Accepts a visitor to evaluate or process this logical operation.
         * @param visitor The visitor being accepted.
         */
        void accept(IASTVisitor &visitor) override;

};

