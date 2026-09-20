#include "expert_system/ast/OpNode.hpp"
#include "expert_system/exceptions/Exceptions.hpp"

OpNode::OpNode(const std::string &sym, OpType opType): symbol(sym), operation(opType){}

std::string OpNode::getSymbol() const {
    return this->symbol;
}


OpType OpNode::getType() const {
    return this->operation;
}

IASTNode *OpNode::getLeftNode() const {
    return this->leftNode.get();
}

IASTNode *OpNode::getRightNode() const {
    return this->rightNode.get();
}

void OpNode::setLeftNode(std::unique_ptr<IASTNode> node) {
    if (!node)
        throw NullException("OpNode::setLeftNode");
    this->leftNode = std::move(node);
}

void OpNode::setRightNode(std::unique_ptr<IASTNode> node) {
    if (!node)
        throw NullException("OpNode::setRightNode");
    this->rightNode = std::move(node);
}

void OpNode::unsetLeftNode() {
    this->leftNode.reset();
}

void OpNode::unsetRightNode() {
    this->rightNode.reset();
}

void OpNode::accept(ASTVisitor &visitor) {
    visitor.visit(*this);
}
