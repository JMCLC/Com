#ifndef __OG_AST_INPUT_NODE_H__
#define __OG_AST_INPUT_NODE_H__

#include <cdk/ast/lvalue_node.h>

namespace og {

  /**
   * Class for describing read nodes.
   */

  class input_node: public cdk::expression_node {
  
  public:
    input_node(int lineno): cdk::expression_node(lineno) {
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_input_node(this, level);
    }
  };

} // og

#endif
