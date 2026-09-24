#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>
#include <stack>
#include <optional>
#include <fstream>

#include "expert_system/exceptions/Exceptions.hpp"
#include "expert_system/ast/AtomicNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/ast/ASTVisitor.hpp"
#include "expert_system/parser/Lexer.hpp"
#include "expert_system/ast/IASTNode.hpp"
#include "expert_system/ast/OpNode.hpp"



/**
 * @brief needed during parsing after tokenization 
 * */


bool isOpToken(const Token &token) {
  auto tokenType = token.type;
  return tokenType != TokenType::VARIABLE &&
          tokenType != TokenType::NOT &&
          tokenType != TokenType::END_OF_LINE &&
          tokenType != TokenType::LPAREN &&
          tokenType != TokenType::RPAREN; 
}

/**
 * @breif: needed during parsing to check if an AST node is an op node
 *
 * */

bool isOpNode(const std::unique_ptr<IASTNode>& node) {
  auto opType = node->getType();
  return opType != OpType::ATOMIC && opType != OpType::NOT;
}


// Hardcoded compile-time mapping function
constexpr std::optional<OpType> tokenToOp(TokenType token) {
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


std::unique_ptr<IASTNode> parseTokens(std::vector<Token> tokens) {
  std::stack<std::unique_ptr<IASTNode>> nodeStack;

  for (auto it = tokens.begin(); it != tokens.end(); it++) {
    auto token = *it;
    // Build AST
    if (token.type == TokenType::VARIABLE)
    {
      if (!nodeStack.empty()) {
        if (isOpNode(nodeStack.top())) {
          auto opNode = dynamic_cast<OpNode*>(nodeStack.top().get());
          if (opNode->getRightNode()) {
            throw  MissingOperation("parseLine", "Right"); 
          }
          opNode->setRightNode(
              std::make_unique<AtomicNode>(token.value)
          );
        }
          
        else
            throw  MissingOperation("parseLine", "Left");
      }
      else {
        nodeStack.push(std::make_unique<AtomicNode>(token.value));
      }
      continue;
    }
    else if (isOpToken(token))
    {
      if (nodeStack.empty())
        throw  MissingOperationValue("parseLine");
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

    //what is inside parantheses is processed recursively
    else if (token.type == TokenType::LPAREN) {
      //find closed parenthesis
      auto rParen = std::find_if(it + 1, tokens.end(), [](Token token) {
          return token.type == TokenType::RPAREN;
      });
      if (it == tokens.end())
          throw  UnclosedParenthese("parseLine");
      auto subTokens = std::ranges::subrange(it + 1, rParen);
      std::vector<Token> subVector(subTokens.begin(), subTokens.end());
      auto node = parseTokens(subVector);
      if (!nodeStack.empty()) {
        if (isOpNode(nodeStack.top())) {
          auto opNode = dynamic_cast<OpNode*>(nodeStack.top().get());
          if (opNode->getRightNode())
            throw  MissingOperation("ParseLine", "Right");
          opNode->setRightNode(std::move(node));
        }
        else
            throw  MissingOperation("parseLine", "Left");         
      }
      else {
        nodeStack.push(std::move(node));
      }

      it = rParen;

    }


  }

  return nodeStack.empty() ? nullptr : std::move(nodeStack.top());
}


// tokenize each line individually
// each line = 1 AST
// each ATOMIC stack is pushed directly
// kind of like RPN
// e.g A => B
// A finds stack empty so it is pushed in the stack directly
// => pops from stack, setLeftNode(A) and pushed back to
// B finds OpNode inside stack, calls setLeftRightNode(B) and pushed Back to th stack, 
// ANOTHER CASE
// A AND (B OR C)
std::unique_ptr<IASTNode> parseLine(std::string line) {
  auto lexer = std::make_unique<Lexer>(line);
  std::vector<Token> tokens = lexer->tokenize();
  return parseTokens(tokens);
}




int main() {
    // std::cout << "--- Building the AST ---" << std::endl;

    // //  Create the atomic facts (Leaves)
    // auto atomA = std::make_unique<AtomicNode>("A");
    // auto atomB = std::make_unique<AtomicNode>("B");
    // auto atomC = std::make_unique<AtomicNode>("C");
    // auto atomD = std::make_unique<AtomicNode>("D");

    // // create ops nodes
    // auto opAnd = std::make_unique<OpNode>("+", OpType::AND);
    // auto opXor = std::make_unique<OpNode>("^", OpType::XOR);
    // auto opImplies = std::make_unique<OpNode>("=>", OpType::IMPLIES);

    // // assembling tree
    // // simulating (A + B) ^ C => D
    // opAnd->setLeftNode(std::move(atomA));
    // opAnd->setRightNode(std::move(atomB));
    // opXor->setLeftNode(std::move(opAnd));
    // opXor->setRightNode(std::move(atomC));
    // opImplies->setLeftNode(std::move(opXor));
    // opImplies->setRightNode(std::move(atomD));

    // std::cout << "Executing Visitor..." << std::endl;
    // ASTVisitor visitor;

    // std::cout << "Result: ";
    // opImplies->accept(visitor);
    // std::cout << std::endl;
    //
    std::ifstream file("examples/example1.txt");

    // 2. Check if the file opened successfully
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file!" << std::endl;
        return 1;
    }

    std::vector<std::string> lines;
    std::string line;
    // 3. Read the file line by line
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    // std::cout << text << std::endl;
    for (const auto& line : lines) {
        std::cout<<"parsing line ..."<<std::endl;
        std::cout<<line<<std::endl;
        try {
          auto node = parseLine(line);
          if (!node)
            continue ;
          ASTVisitor visitor;
          node->accept(visitor);
          std::cout <<std::endl;
        } catch (std::exception& error) {
          std::cerr<< error.what()<<std::endl;
        }
    }

    // 4. Close the file
    file.close();
    return 0;
}
