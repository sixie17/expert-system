#include "expert_system/parser/Reducer.hpp"
#include "expert_system/ast/IASTNode.hpp"
#include "expert_system/ast/NotNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/exceptions/Exceptions.hpp"
#include <memory>

/*
 * @brief true for the operations held by an OpNode (they have a left and a right node)
 * */
static inline bool isBinaryOp(OpType type) {
  return type == OpType::AND || type == OpType::OR || type == OpType::XOR ||
         type == OpType::IMPLIES || type == OpType::IFaoF;
}

std::unique_ptr<IASTNode> Reducer::makeOp(OpType type, std::unique_ptr<IASTNode> left,
                                          std::unique_ptr<IASTNode> right) {
  std::string symbol;
  switch (type) {
  case OpType::AND: symbol = "+"; break;
  case OpType::OR:  symbol = "|"; break;
  case OpType::XOR: symbol = "^"; break;
  default:
    throw InvalidOperation("Reducer::makeOp");
  }
  auto node = std::make_unique<OpNode>(symbol, type);
  node->setLeftNode(std::move(left));
  node->setRightNode(std::move(right));
  return node;
}

std::unique_ptr<IASTNode> Reducer::reduce(std::unique_ptr<IASTNode> node, bool negated) {
  if (!node)
    throw NullException("Reducer::reduce");

  if (node->getType() == OpType::ATOMIC) {
    if (!negated)
      return node;
    auto notNode = std::make_unique<NotNode>();
    notNode->setRightNode(std::move(node));
    return notNode;
  }
  // the NotNode itself is dropped, its negation is carried by the flag: !!A cancels out
  if (node->getType() == OpType::NOT)
    return reduce(static_cast<NotNode *>(node.get())->releaseRightNode(), !negated);
  if (!isBinaryOp(node->getType()))
    throw InvalidOperation("Reducer::reduce");

  auto *opNode = static_cast<OpNode *>(node.get());
  auto left = opNode->releaseLeftNode();
  auto right = opNode->releaseRightNode();

  if (!negated) {
    opNode->setLeftNode(reduce(std::move(left), false));
    opNode->setRightNode(reduce(std::move(right), false));
    return node;
  }

  switch (node->getType()) {
  // !(A + B) -> !A | !B
  case OpType::AND:
    return makeOp(OpType::OR, reduce(std::move(left), true), reduce(std::move(right), true));
  // !(A | B) -> !A + !B
  case OpType::OR:
    return makeOp(OpType::AND, reduce(std::move(left), true), reduce(std::move(right), true));
  // !(A ^ B) -> !A ^ B
  case OpType::XOR:
    return makeOp(OpType::XOR, reduce(std::move(left), true), reduce(std::move(right), false));
  // !(A => B) -> A + !B
  case OpType::IMPLIES:
    return makeOp(OpType::AND, reduce(std::move(left), false), reduce(std::move(right), true));
  // !(A <=> B) -> A ^ B
  default:
    return makeOp(OpType::XOR, reduce(std::move(left), false), reduce(std::move(right), false));
  }
}

std::unique_ptr<IASTNode> Reducer::reduce(std::unique_ptr<IASTNode> root) {
  return reduce(std::move(root), false);
}
