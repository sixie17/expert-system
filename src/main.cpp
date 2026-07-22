#include <iostream>
#include <memory>
#include <fstream>
#include <vector>
#include "expert_system/ast/AtomicNode.hpp"
#include "expert_system/ast/OpNode.hpp"
#include "expert_system/ast/ASTVisitor.hpp"
#include "expert_system/parser/Lexer.hpp"

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
        auto lexer = std::make_unique<Lexer>(line);
        std::vector<Token> tokens = lexer->tokenize();
        for (const auto& token : tokens) {
            std::cout << token.value << " ";
        }
        std::cout << std::endl;
    }

    // 4. Close the file
    file.close();
    return 0;
}
