#pragma once
#include "expert_system/ast/NotNode.hpp"
#include <string>
class AtomicNode;
class NotNode; // only has right node
class OpNode; // all other nodes will have left and right

/**
 * @brief class for the standard Visitor pattern on the AST.
 * 
 * Implement this class to define operations (like evaluation or printing)
 * that traverse through the Abstract Syntax Tree.
 */
class ASTVisitor {
    private:
        std::string prefix = "";
        bool isLast = true;
        bool isRoot = true;
    public:

        
        /**
         * @brief Virtual destructor.
         */

        virtual ~ASTVisitor() = default;

        /**
         * @brief Visits an AtomicNode.
         * @param node The atomic node being visited.
         */
        void visit(AtomicNode &node);

        /**
         * @brief Visits an Operation node (e.g., AND, OR).
         * @param node The operation node being visited.
         */
        virtual void visit(OpNode &node);


        /**
         *  @brief Visits a Not node 
         *  @param node the Not node being visited.
         * */

        virtual void visit(NotNode &node);
};
