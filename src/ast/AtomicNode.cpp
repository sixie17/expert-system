#include "expert_system/ast/AtomicNode.hpp"

AtomicNode::AtomicNode(const std::string &sym):symbol(sym), operation(OpType::ATOMIC) {};

std::string AtomicNode::getSymbol() const {
    return this->symbol;
}

OpType AtomicNode::getType() const {
    return this->operation;
}

void AtomicNode::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}