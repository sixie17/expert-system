#include <iostream>
#include <memory>
#include "expert_system/ast/AtomicNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/ast/ASTVisitor.hpp"

int main() {
    std::cout << "--- Building the AST ---" << std::endl;

    //  Create the atomic facts (Leaves)
    auto atomA = std::make_unique<AtomicNode>("A");
    auto atomB = std::make_unique<AtomicNode>("B");
    auto atomC = std::make_unique<AtomicNode>("C");
    auto atomD = std::make_unique<AtomicNode>("D");

    // create ops nodes
    auto opAnd = std::make_unique<OpNode>("+", OpType::AND); 
    auto opXor = std::make_unique<OpNode>("^", OpType::XOR);
    auto opImplies = std::make_unique<OpNode>("=>", OpType::IMPLIES);

    // assembling tree
    // simulating (A + B) ^ C => D
    opAnd->setLeftNode(std::move(atomA));
    opAnd->setRightNode(std::move(atomB));
    opXor->setLeftNode(std::move(opAnd));
    opXor->setRightNode(std::move(atomC));
    opImplies->setLeftNode(std::move(opXor));
    opImplies->setRightNode(std::move(atomD));

    std::cout << "Executing Visitor..." << std::endl;
    ASTVisitor visitor;
    
    std::cout << "Result: ";
    opImplies->accept(visitor); 
    std::cout << std::endl;

    return 0;
}