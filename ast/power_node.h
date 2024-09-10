#ifndef __OG_AST_POWER_NODE_H__
#define __OG_AST_POWER_NODE_H__

#include <cdk/ast/binary_operation_node.h>

namespace og {

  /**
   * Class for describing the addition ('+') operator
   */
  class power_node: public cdk::binary_operation_node {
  public:
    /**
     * @param lineno source code line number for this node
     * @param left first operand
     * @param right second operand
     */
    power_node(int lineno, expression_node *left, expression_node *right) :
        binary_operation_node(lineno, left, right) {
    }

    /**
     * @param av basic AST visitor
     * @param level syntactic tree level
     */
    void accept(basic_ast_visitor *av, int level) {
      av->do_power_node(this, level);
    }

  };

} // og

#endif
