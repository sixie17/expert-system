#include "expert_system/ast/NotNode.hpp"
#include "expert_system/ast/ASTVisitor.hpp"
#include "expert_system/ast/IASTNode.hpp"
#include "expert_system/exceptions/Exceptions.hpp"
#include <string>

NotNode::NotNode() : operation(OpType::NOT) {}

OpType NotNode::getType() const { return this->operation; }

IASTNode *NotNode::getRightNode() const { return this->rightNode.get(); }

void NotNode::setRightNode(std::unique_ptr<IASTNode> node) {
  if (!node)
    throw NullException("NotNode::setRightNode");
  this->rightNode = std::move(node);
}

void NotNode::unsetRightNode() { this->rightNode.reset(); }

void NotNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

std::string NotNode::getSymbol() const { return "!"; }

std::unique_ptr<IASTNode> NotNode::releaseRightNode() {
  return std::move(this->rightNode);
}

