#ifndef __OG_AST_TUPLE_H__
#define __OG_AST_TUPLE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace og {

    class tuple_node: public cdk::expression_node {
        cdk::sequence_node *_sequence;

    public:
        tuple_node(int lineno, cdk::sequence_node *sequence) : 
                cdk::expression_node(lineno), _sequence(sequence) {
        }

    public:

        cdk::sequence_node *sequence() {
            return _sequence;
        }

        void accept(basic_ast_visitor *sp, int level) {
            sp->do_tuple_node(this, level);
        }
    };
}

#endif