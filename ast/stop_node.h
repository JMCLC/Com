#ifndef __MML_AST_STOP_NODE_H__
#define __MML_AST_STOP_NODE_H__

#include <cdk/ast/expression_node.h>

namespace mml {

  /**
   * Class for describing stop nodes.
   */
  class stop_node : public cdk::basic_node {
    int _svalue;

  public:
    inline stop_node(int lineno, int svalue = 1) :
        cdk::basic_node(lineno), _svalue(svalue) {
    }

  public:
    inline int svalue() {
      return _svalue;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_stop_node(this, level);
    }

  };

} // mml

#endif