#ifndef __MML_AST_FUNCTION_DEFINITION_NODE_H__
#define __MML_AST_FUNCTION_DEFINITION_NODE_H__

#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include "ast/block_node.h"

namespace mml {

  /**
   * Class for describing function definition nodes.
   */
  class function_definition_node : public cdk::expression_node {
    cdk::sequence_node *_arguments;
    block_node *_block;
    bool _main;

  public: 
    inline function_definition_node(int lineno, cdk::sequence_node *arguments, std::shared_ptr<cdk::basic_type> function_type, block_node *block, bool main = false) :
        cdk::expression_node(lineno), _arguments(arguments), _block(block), _main(main) {
          type(function_type);
    }

  public:
    inline cdk::sequence_node *arguments() {
        return _arguments;
    }

    inline cdk::typed_node *argument(size_t n) {
      return dynamic_cast<cdk::typed_node*>(_arguments->node(n));
    }

    bool main() {
      return _main;
    }

    block_node *block() {
        return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // mml

#endif