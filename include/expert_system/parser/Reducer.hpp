#pragma once
#include <memory>
#include "expert_system/ast/IASTNode.hpp"

class Reducer {
  private:
    //avoid mistakenly creating a Reducer instance
    Reducer() = delete;  
    /*
     * @brief builds a binary OpNode of the given type from two operands
     * @throws InvalidOperation if type is not AND, OR or XOR
     * */
    static std::unique_ptr<IASTNode> makeOp(OpType type, std::unique_ptr<IASTNode> left,
                                            std::unique_ptr<IASTNode> right);

    /*
     * @brief reduces node, negated tells if it sits under an odd number of negations.
     * NotNodes are dropped and flip the flag, so !!A cancels out, and a negation is
     * pushed down until it only applies to atomic nodes:
     *   !(A + B)   -> !A | !B
     *   !(A | B)   -> !A + !B
     *   !(A ^ B)   -> !A ^ B
     *   !(A => B)  -> A + !B
     *   !(A <=> B) -> A ^ B
     * @throws NullException if node is null
     * @throws InvalidOperation on an unknown OpType
     * */
    static std::unique_ptr<IASTNode> reduce(std::unique_ptr<IASTNode> node, bool negated);

  public:
    /*
     * @breif takes an AST as input and reduces it's logical expression into something we can easily handle
     * @param root the root of the AST to be reduced
     * @return IASTNode in it's reduced form 
     *
     * */
    [[nodiscard]] static std::unique_ptr<IASTNode> reduce(std::unique_ptr<IASTNode> root);
};

