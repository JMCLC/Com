#ifndef __OG_AST_TUPLE_INDEX_H__
#define __OG_AST_TUPLE_INDEX_H__

#include <cdk/ast/lvalue_node.h>
#include "tuple_node.h"
#include "targets/basic_ast_visitor.h"

namespace og {

  class tuple_index_node: public cdk::lvalue_node {
    cdk::expression_node *_base;
    int _index;

  public:
    tuple_index_node(int lineno, cdk::expression_node *base, int index) :
        cdk::lvalue_node(lineno), _base(base), _index(index) {
    }

  public:
    cdk::expression_node *base() {
      return _base;
    }
    
    int index() {
      return _index;
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tuple_index_node(this, level);
    }

  };

}

#endif
