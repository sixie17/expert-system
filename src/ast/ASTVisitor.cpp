#include "expert_system/ast/AtomicNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/ast/ASTVisitor.hpp"
#include <iostream>

void ASTVisitor::visit(AtomicNode &node) {
    if (!this->isRoot) {
        std::cout << this->prefix << (this->isLast ? "└── " : "├── ");
    }
    
    std::cout << node.getSymbol() << std::endl;
    this->isRoot = false;
}

void ASTVisitor::visit(OpNode &node) {
    if (!this->isRoot) {
        std::cout << this->prefix << (this->isLast ? "└── " : "├── ");
    }
    
    std::cout << node.getSymbol() << std::endl;

    std::string oldPrefix = prefix;
    if (!this->isRoot) {
        this->prefix += (this->isLast ? "    " : "│   ");
    }
    this->isRoot = false;

    IASTNode* left = node.getLeftNode();
    IASTNode* right = node.getRightNode();

    if (left) {
        bool oldLast = isLast;
        this->isLast = (right == nullptr); 
        left->accept(*this);
        this->isLast = oldLast;
    }

    if (right) {
        bool oldLast = isLast;
        this->isLast = true;
        right->accept(*this);
        this->isLast = oldLast;
    }

    // Restore the prefix for the parent node
    this->prefix = oldPrefix;
}



void ASTVisitor::visit(NotNode &node) {
  if (!this->isRoot) {
        std::cout << this->prefix << (this->isLast ? "└── " : "├── ");
    }
    
    std::cout << node.getSymbol() << std::endl;

    std::string oldPrefix = prefix;
    if (!this->isRoot) {
        this->prefix += (this->isLast ? "    " : "│   ");
    }
    this->isRoot = false;
    IASTNode* right = node.getRightNode();

    if (right) {
        bool oldLast = isLast;
        this->isLast = true;
        right->accept(*this);
        this->isLast = oldLast;
    }

    // Restore the prefix for the parent node
    this->prefix = oldPrefix;

}
