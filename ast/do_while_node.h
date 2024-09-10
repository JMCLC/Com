#ifndef __OG_AST_DO_WHILE_H__
#define __OG_AST_DO_WHILE_H__

#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/typed_node.h>



namespace og {

  /**
   * Class for describing do while cycles.
   */

  class do_while_node: public cdk::basic_node {
    cdk::expression_node *_condition;
    block_node *_block;

  public:
    do_while_node(int lineno, cdk::expression_node *condition,  block_node *block) :
        cdk::basic_node(lineno), _condition(condition), _block(block) {}


  public:
   
    cdk::expression_node *condition() {
      return _condition;
    }
    block_node *block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_do_while_node(this, level);
    }

  };

}

#endif
