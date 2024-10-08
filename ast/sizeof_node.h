#ifndef __MML_AST_SIZEOF_H__
#define __MML_AST_SIZEOF_H__

#include <cdk/ast/unary_operation_node.h>
#include <cdk/ast/expression_node.h>

namespace mml {

   /**
   * Class for describing sizeof nodes.
   */
  class sizeof_node: public cdk::unary_operation_node {
  public:
    sizeof_node(int lineno, cdk::expression_node *argument) :
        cdk::unary_operation_node(lineno, argument)
    {
    }

  public:
    
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_sizeof_node(this, level);
    }

  };

} // mml

#endif