#include "expert_system/parser/Parser.hpp"
#include "expert_system/parser/Lexer.hpp"
#include "expert_system/ast/AtomicNode.hpp"
#include "expert_system/ast/NotNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/exceptions/Exceptions.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>


bool Parser::isOpToken(const Token &token) {
  auto tokenType = token.type;
  return tokenType != TokenType::VARIABLE &&
          tokenType != TokenType::NOT &&
          tokenType != TokenType::END_OF_LINE &&
          tokenType != TokenType::LPAREN &&
          tokenType != TokenType::RPAREN;
}


bool Parser::isOpNode(const std::unique_ptr<IASTNode> &node) {
  auto opType = node->getType();
  return opType != OpType::ATOMIC && opType != OpType::NOT;
}


bool Parser::isLogicOperation(const Token &token) {
  auto tokenType = token.type;
  return tokenType == TokenType::IFAOF || tokenType == TokenType::IMPLIES;
}


// Hardcoded compile-time mapping function
constexpr std::optional<OpType> Parser::tokenToOp(TokenType token) {
    switch (token) {
        case TokenType::VARIABLE: return OpType::ATOMIC;
        case TokenType::AND:      return OpType::AND;
        case TokenType::OR:       return OpType::OR;
        case TokenType::XOR:      return OpType::XOR;
        case TokenType::NOT:      return OpType::NOT;
        case TokenType::IMPLIES:  return OpType::IMPLIES;
        case TokenType::IFAOF:    return OpType::IFaoF;

        // These tokens do not map to an operator
        default:                  return std::nullopt;
    }
}


std::unique_ptr<IASTNode> Parser::parseOperand(TokenIt &it, TokenIt end) {
  if (it == end)
    throw MissingOperationValue("parseOperand");

  if (it->type == TokenType::VARIABLE)
    return std::make_unique<AtomicNode>(it->value);

  // e.g A !=> B
  if (isLogicOperation(*it))
    throw InvalidLogicOperation("parseOperand");

  if (it->type == TokenType::NOT) {
    auto notNode = std::make_unique<NotNode>();
    ++it;
    notNode->setRightNode(parseOperand(it, end));
    return notNode;
  }

  //what is inside parantheses is processed recursively
  if (it->type == TokenType::LPAREN) {
    //find the matching closing parenthesis, taking nesting into account
    int depth = 0;
    auto rParen = std::ranges::find_if(it, end, [&depth](const Token &token) {
        if (token.type == TokenType::LPAREN)
          depth++;
        else if (token.type == TokenType::RPAREN)
          depth--;
        return depth == 0;
    });
    if (rParen == end)
        throw  UnclosedParenthese("parseOperand");
    auto node = parseTokens(std::span<const Token>(it + 1, rParen), true);
    if (!node)
      throw MissingOperationValue("parseOperand");
    it = rParen;
    return node;
  }

  // operator, ')' or end of line where an operand was expected
  throw MissingOperationValue("parseOperand");
}


void Parser::attachOperand(std::unique_ptr<IASTNode> node, NodeStack &nodeStack) {
  if (nodeStack.empty()) {
    nodeStack.push(std::move(node));
    return;
  }
  if (!isOpNode(nodeStack.top()))
    throw  MissingOperation("attachOperand", "Left");
  auto opNode = dynamic_cast<OpNode*>(nodeStack.top().get());
  if (opNode->getRightNode())
    throw  MissingOperation("attachOperand", "Right");
  opNode->setRightNode(std::move(node));
}


std::unique_ptr<IASTNode> Parser::parseTokens(std::span<const Token> tokens) {
  return parseTokens(tokens, false);
}


std::unique_ptr<IASTNode> Parser::parseTokens(std::span<const Token> tokens, bool nested) {
  NodeStack nodeStack;

  for (auto it = tokens.begin(); it != tokens.end(); it++) {
    const auto &token = *it;
    if (token.type == TokenType::END_OF_LINE)
      break;
    // Build AST
    if (token.type == TokenType::VARIABLE ||
        token.type == TokenType::NOT ||
        token.type == TokenType::LPAREN)
    {
      attachOperand(parseOperand(it, tokens.end()), nodeStack);
      continue;
    }
    else if (token.type == TokenType::RPAREN)
      throw UnclosedParenthese("parseTokens");
    // => and <=> split the line: left side is what is on the stack,
    // right side is the rest of the tokens, so the logic operation is the root
    else if (isLogicOperation(token))
    {
      if (nested)
        throw InvalidLogicOperation("parseTokens");
      if (nodeStack.empty())
        throw  MissingOperationValue("parseTokens");
      auto leftNode = std::move(nodeStack.top());
      if (isOpNode(leftNode) && !dynamic_cast<OpNode*>(leftNode.get())->getRightNode())
        throw  MissingOperationValue("parseTokens");
      auto rightNode = parseTokens(std::span<const Token>(it + 1, tokens.end()), true);
      if (!rightNode)
        throw  MissingOperationValue("parseTokens");

      auto logicNode = std::make_unique<OpNode>(token.value, tokenToOp(token.type).value());
      logicNode->setLeftNode(std::move(leftNode));
      logicNode->setRightNode(std::move(rightNode));
      return logicNode;
    }
    else if (isOpToken(token))
    {
      if (nodeStack.empty())
        throw  MissingOperationValue("parseTokens");
      auto leftNode = std::move(nodeStack.top());
      nodeStack.pop();
      auto opTypeOpt = tokenToOp(token.type);
      if (!opTypeOpt.has_value()) {
        throw std::runtime_error("Unexpected token type: expected an operator.");
      }

      // Extract the raw value safely using value() or *
      auto currentNode = std::make_unique<OpNode>(token.value, opTypeOpt.value());
      currentNode->setLeftNode(std::move(leftNode));
      nodeStack.push(std::move(currentNode));
      continue;
    }
  }

  if (nodeStack.empty())
    return nullptr;
  // e.g A + (operation without a right node)
  if (isOpNode(nodeStack.top()) &&
      !dynamic_cast<OpNode*>(nodeStack.top().get())->getRightNode())
    throw  MissingOperationValue("parseTokens");
  return std::move(nodeStack.top());
}


std::unique_ptr<IASTNode> Parser::parseLine(const std::string &line, size_t lineNumber) {
  try {
    Lexer lexer(line);
    std::vector<Token> tokens = lexer.tokenize();
    return parseTokens(tokens);
  } catch (const std::exception &error) {
    throw ParseError(lineNumber, error.what());
  }
}
