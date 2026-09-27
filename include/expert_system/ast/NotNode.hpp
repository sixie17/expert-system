#pragma once
#include <memory>
#include <string>
#include "IASTNode.hpp"
#include "ASTVisitor.hpp"



/**
 * @breif NotNode is in between AtomicNode and OpNode in concept the diference is it only has a right node and does not have a symbol
 * reason A! is invalid but !A is valid, the right node can be atomic or OpNode 
 * or another NotNode e.g:
 * @example !A valid
 * @example !(A + B) valid
 * @example !!A valid
 * @example !!(A OR B) valid
 */


class NotNode : public IASTNode {
  private:
    std::unique_ptr<IASTNode> rightNode;
    const OpType operation;

  public:
    /**
     * @brief Virtual destructor
     */
     ~NotNode() = default;
     

     /**
      *  
      * @brief Construction Not node
      *  
      * */
     NotNode(); 

     /**
      * @brief 
      */
    OpType getType() const override;



    /**
     *  @brief returns the right node
     */
    IASTNode *getRightNode() const;


    /**
     *  @brief for compatibility although it will always return the same symbol
     */
    std::string getSymbol() const;

    /**
     *  @brief sets the right node 
     */
    void setRightNode(std::unique_ptr<IASTNode> node);

    /**
     *  @brief
     */
    void unsetRightNode();


  
    void accept(ASTVisitor &visitor) override;
};

