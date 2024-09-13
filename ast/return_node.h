#ifndef __MML_AST_RETURN_NODE_H__
#define __MML_AST_RETURN_NODE_H__

#include <cdk/ast/expression_node.h>

namespace mml {

  /**
   * Class for describing return nodes.
   */
  class return_node : public cdk::basic_node {
    cdk::expression_node *_rvalue;

  public:
    inline return_node(int lineno, cdk::expression_node *rvalue) :
        cdk::basic_node(lineno), _rvalue(rvalue) {
    }

  public:
    inline cdk::expression_node *rvalue() {
      return _rvalue;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_return_node(this, level);
    }

  };

} // mml

#endif