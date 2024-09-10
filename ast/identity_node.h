#ifndef __OG_IDENTITY_NODE_H__
#define __OG_IDENTITY_NODE_H__


namespace og {

    /**
     * Class for describing the identity operator.
     */
    class identity_node: public cdk::unary_operation_node {

    public:
      identity_node(int lineno, cdk::expression_node *argument) :
        unary_operation_node(lineno, argument) {
      }

    public:
      void accept(basic_ast_visitor *sp, int level) {
        sp->do_identity_node(this, level);
      }

    };

} // og

#endif