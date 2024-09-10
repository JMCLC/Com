#include <string>
#include "targets/type_checker.h"
#include "ast/all.h"
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

void og::type_checker::do_power_node(og::power_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  else
    throw std::string("Wrong types in binary expression.");

}

void og::type_checker::do_while_node(og::while_node * const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->block()->accept(this, lvl + 2);
}

void og::type_checker::do_do_while_node(og::do_while_node * const node, int lvl) {
  node->condition()->accept(this, lvl + 2);
  node->block()->accept(this, lvl + 2);
}

std::shared_ptr<cdk::basic_type> og::type_checker::typeOfPointer(std::shared_ptr<cdk::reference_type> leftPtr, std::shared_ptr<cdk::reference_type> rightPtr){
    std::shared_ptr<cdk::basic_type> left, right;
    left = leftPtr;
    right = rightPtr;
    while (left->name() == cdk::TYPE_POINTER && right->name() == cdk::TYPE_POINTER) {
      left = cdk::reference_type_cast(left)->referenced();
      right = cdk::reference_type_cast(right)->referenced();
    }
    if (left->name() == cdk::TYPE_POINTER || right->name() == cdk::TYPE_POINTER)
      throw std::string("Wrong pointer type.");
    if (left->name() == cdk::TYPE_INT && right->name() == cdk::TYPE_INT)
      return cdk::make_primitive_type(4, cdk::TYPE_INT);
    else if (left->name() == cdk::TYPE_DOUBLE && right->name() == cdk::TYPE_DOUBLE)
      return cdk::make_primitive_type(8, cdk::TYPE_DOUBLE);
    else if (left->name() == cdk::TYPE_STRING && right->name() == cdk::TYPE_STRING)
      return cdk::make_primitive_type(4, cdk::TYPE_STRING);
    else // FIXME AUTO
      throw std::string("Wrong pointer type.");
}

void og::type_checker::verifyArguments(std::shared_ptr<og::symbol> function, cdk::sequence_node *arguments) {
  std::vector<std::shared_ptr<cdk::basic_type>> *types = new std::vector<std::shared_ptr<cdk::basic_type>>();

  for(size_t i = 0; i < arguments->size(); i++) {
    cdk::expression_node *expr = dynamic_cast<cdk::expression_node *>(arguments->node(i));
    if(expr != nullptr)
      types->push_back(expr->type());
  }

  if(function->arguments().size() == types->size()) {
    for(size_t i = 0; i < types->size(); i++) {
      if(function->arguments().at(i)->name() != types->at(i)->name() && 
        !(function->arguments().at(i)->name() == cdk::TYPE_DOUBLE && 
        types->at(i)->name() == cdk::TYPE_INT))
        throw std::string("'" + function->name() + "' call has no matching function types.");
      else if(function->arguments().at(i)->name() == cdk::TYPE_POINTER && types->at(i)->name() == cdk::TYPE_POINTER)
        typeOfPointer(cdk::reference_type_cast(function->arguments().at(i)), cdk::reference_type_cast(types->at(i)));
    }
  }
  else
    throw std::string("'" + function->name() + "' call with the wrong number of arguments.");
}

std::string og::type_checker::main_function(const std::string &id) {
  if (id == "og")
    return "_main";
  else if (id == "_main")
    return "._main";
  else
    return id;
}

//---------------------------------------------------------------------------

void og::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for(size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl + 2);
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->node(i));
    if(expression != nullptr && expression->is_typed(cdk::TYPE_UNSPEC)) {
      og::input_node *input = dynamic_cast<og::input_node*>(expression);

      if(input != nullptr)
        input->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
      else
        throw std::string("Unknown node with unspecified type.");
    }
  }
}

//---------------------------------------------------------------------------

void og::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------
// TYPES

void og::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
}

void og::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------
// UNARY OPERATIONS

void og::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  else if (node->argument()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *input = dynamic_cast<og::input_node*>(node->argument());

    if(input != nullptr) {
      node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
      node->argument()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong type in argument of unary expression (Integer or double expected).");
}

void og::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void og::type_checker::do_identity_node(og::identity_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void og::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *input = dynamic_cast<og::input_node*>(node->argument());

    if(input != nullptr) {
      node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
      node->argument()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong type in argument of unary expression (Integer expected).");

}

//---------------------------------------------------------------------------
// BINARY OPERATIONS

void og::type_checker::do_GeneralLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);

  if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node *>(node->left());

    if(inputl != nullptr)
      node->left()->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unknown node with unspecified type.");
  }

  node->right()->accept(this, lvl + 2);
  if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputr = dynamic_cast<og::input_node *>(node->right());

    if(inputr != nullptr)
      node->left()->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unknown node with unspecified type.");
  }

  if(node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER))
    typeOfPointer(cdk::reference_type_cast(node->left()->type()),cdk::reference_type_cast(node->right()->type()));
  else if(node->left()->type()->name() != node->right()->type()->name()) {
    if(!((node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))||
    (node->left()->is_typed(cdk::TYPE_INT)  &&  node->right()->is_typed(cdk::TYPE_INT))))
        throw std::string("Operator has incompatible types.");
  }
 node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_BooleanLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  
  if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node *>(node->left());

    if(inputl != nullptr)
      node->left()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(!node->left()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in (left and right) binary operators.");

  node->right()->accept(this, lvl + 2);
  if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputr = dynamic_cast<og::input_node *>(node->right());

    if(inputr != nullptr)
       node->right()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(!node->right()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in (left and right) binary operators.");

   node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_IntOnlyExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());

    if(inputl != nullptr)
      node->left()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");

  }
  node->right()->accept(this, lvl + 2);
  if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());

    if(inputl != nullptr)
      node->left()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");

  }
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  else
    throw std::string("Integer expression expected in (left and right) binary operators.");

}

void og::type_checker::do_ScalarLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  
  if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());

    if(inputl != nullptr)
      node->left()->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Wrong binary logical expression (expected integer or double).");
  
  node->right()->accept(this, lvl + 2);
  if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputr = dynamic_cast<og::input_node*>(node->right());

    if(inputr != nullptr)
      node->right()->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Wrong binary logical expression (expected integer or double).");

  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_IDExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());
    og::input_node *inputr = dynamic_cast<og::input_node*>(node->right());

    if(inputl != nullptr && inputr != nullptr) {
      node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
      node->left()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
      node->right()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());

    if(inputl != nullptr) {
      node->left()->type(node->right()->type());
      node->type(node->right()->type());
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputr = dynamic_cast<og::input_node*>(node->right());

    if(inputr != nullptr) {
      node->right()->type(node->left()->type());
      node->type(node->left()->type());
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong types in binary expression.");
}

void og::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(node->left()->type());
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER))
    node->type(node->right()->type());
  else if(node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());
    og::input_node *inputr = dynamic_cast<og::input_node*>(node->right());

    if(inputl != nullptr)
      node->left()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");

    if(inputr != nullptr)
      node->right()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  }
  else if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());

    if(inputl != nullptr) {
      if(node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT))
        node->left()->type(node->right()->type());
      else
        throw std::string("Invalid expression in right argument of binary expression.");
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputr = dynamic_cast<og::input_node*>(node->right());

    if(inputr != nullptr) {
      if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->left()->is_typed(cdk::TYPE_INT))
        node->right()->type(node->left()->type());
      else
        throw std::string("Invalid expression in left argument of binary expression.");
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong types in binary expression.");
  
}

void og::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
    node->type(node->left()->type());
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER))
    node->type(node->right()->type());
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    typeOfPointer(cdk::reference_type_cast(node->left()->type()),cdk::reference_type_cast(node->right()->type()));
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  }
  else if(node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());
    og::input_node *inputr = dynamic_cast<og::input_node*>(node->right());

    if(inputl != nullptr)
      node->left()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");

    if(inputr != nullptr)
      node->right()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  }
  else if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputl = dynamic_cast<og::input_node*>(node->left());

    if(inputl != nullptr) {
      if(node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT))
        node->left()->type(node->right()->type());
      else
        throw std::string("Invalid expression in right argument of binary expression.");
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if(node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputr = dynamic_cast<og::input_node*>(node->right());

    if(inputr != nullptr) {
      if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->left()->is_typed(cdk::TYPE_INT))
        node->right()->type(node->left()->type());
      else
        throw std::string("Invalid expression in left argument of binary expression.");
    }
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else
    throw std::string("Wrong types in binary expression.");
}

void og::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  do_IDExpression(node, lvl);
}
void og::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  do_IDExpression(node, lvl);
}
void og::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  do_IntOnlyExpression(node, lvl);
}
void og::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void og::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void og::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void og::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}
void og::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  do_GeneralLogicalExpression(node, lvl);
}
void og::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  do_GeneralLogicalExpression(node, lvl);
}
void og::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  do_BooleanLogicalExpression(node, lvl);
}
void og::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  do_BooleanLogicalExpression(node, lvl);
} 

//---------------------------------------------------------------------------
// VARIABLES

void og::type_checker::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  if (node->initializer() != nullptr) {
    node->initializer()->accept(this, lvl + 2);
    std::vector<std::string> *identifiers = new std::vector<std::string>();
    if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)){
       og::input_node *input = dynamic_cast<og::input_node*>(node->initializer());
       og::stack_alloc_node *stack = dynamic_cast<og::stack_alloc_node*>(node->initializer());
      if(input != nullptr) {
        if(node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE))
          node->initializer()->type(node->type());
        else
          throw std::string("Unable to read input.");
      }
      else if (stack != nullptr) {
        if (node->is_typed(cdk::TYPE_POINTER))
          node->initializer()->type(node->type());
      }
      else
        throw std::string("Unknown node with unspecified type.");
    }
    else if (node->is_typed(cdk::TYPE_INT)) {
      if (!node->initializer()->is_typed(cdk::TYPE_INT))
        throw std::string("Wrong type for initializer (integer expected).");
    }
    else if (node->is_typed(cdk::TYPE_DOUBLE)) {
      if (!node->initializer()->is_typed(cdk::TYPE_DOUBLE) && !node->initializer()->is_typed(cdk::TYPE_INT))
        throw std::string("Wrong type for initializer (integer or double expected).");
    }
    else if (node->is_typed(cdk::TYPE_STRING)) {
      if (!node->initializer()->is_typed(cdk::TYPE_STRING))
        throw std::string("Wrong type for initializer (string expected).");
    }
    else if (node->is_typed(cdk::TYPE_POINTER)) {
      if (!node->initializer()->is_typed(cdk::TYPE_POINTER))
        throw std::string("Wrong type for initializer (pointer expected).");
    }
    else if (node->is_typed(cdk::TYPE_STRUCT)) {
      og::tuple_node *tuple = dynamic_cast<og::tuple_node*>(node->initializer());
      std::string delimiter = ",";
      std::string id = node->identifier();
      size_t pos = 0;
      std::string token;
      while ((pos =  id.find(delimiter)) != std::string::npos) {
          token =  id.substr(0, pos);
          identifiers->push_back(token);
          id.erase(0, pos + delimiter.length());
      }
      identifiers->push_back(id);
      size_t size = identifiers->size();
      if (size != 1 && tuple->sequence()->size() != size)
        throw std::string("The number of variables is different from the number of initializers. This cannot happen.");
      else
      {
        node->type(tuple->type());
        if (size != 1)
        {
          for (size_t i = 0; i < size; i++)
          {
            const std::string &id = identifiers->at(i);
            std::shared_ptr<og::symbol> symbol = std::make_shared<og::symbol>(node->qualifier(),
             cdk::structured_type_cast(tuple->type())->component(i),
              id,
              false,
              0,
              false);

            if (_symtab.insert(id, symbol))
              _parent->set_new_symbol(symbol);
            else
              throw std::string("Variable '" + id + "' has been redeclared.");
          }
        }
        else if (tuple->is_typed(cdk::TYPE_STRUCT))
        {
          const std::string &id = identifiers->at(0);
            std::shared_ptr<og::symbol> symbol = std::make_shared<og::symbol>(node->qualifier(),
              tuple->type(),
              id,
              false,
              0,
              false);

            if (_symtab.insert(id, symbol))
              _parent->set_new_symbol(symbol);
            else
              throw std::string("Variable '" + id + "' has been redeclared.");
        }
      }


    }
    else if ((node->initializer()->is_typed(cdk::TYPE_INT) || 
    node->initializer()->is_typed(cdk::TYPE_DOUBLE) || 
    node->is_typed(cdk::TYPE_STRING)) && node->is_typed(cdk::TYPE_STRUCT)) {
      og::tuple_node *tuple = dynamic_cast<og::tuple_node*>(node->initializer());
      if (tuple->sequence()->size() == 1)
        node->type(node->initializer()->type());
    }
    else if (node->initializer()->is_typed(cdk::TYPE_STRUCT))
      node->type(node->initializer()->type());
    else
      throw std::string("Unknown type for variable initializer.");
  }
  if (!node->is_typed(cdk::TYPE_STRUCT)) {
    const std::string &id = node->identifier();
    std::shared_ptr<og::symbol> symbol = std::make_shared<og::symbol>(node->qualifier(),
      node->type(),
      id,
      false,
      0,
      false);

      if (_symtab.insert(id, symbol))
        _parent->set_new_symbol(symbol);
      else
        throw std::string("Variable '" + id + "' has been redeclared.");
  }
}

void og::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr)
    node->type(symbol->type());
  else
     throw "Undeclared variable '" + id + "'.";
}

void og::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());

}

void og::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->rvalue()->accept(this, lvl + 2);
  if(node->lvalue()->is_typed(cdk::TYPE_UNSPEC))
    throw std::string("Left value must have a type.");

  if(node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *inputr = dynamic_cast<og::input_node *>(node->rvalue());
    og::stack_alloc_node *stackr = dynamic_cast<og::stack_alloc_node *>(node->rvalue());

    if(inputr != nullptr) {
      if(node->lvalue()->is_typed(cdk::TYPE_INT) || node->lvalue()->is_typed(cdk::TYPE_DOUBLE))
        node->rvalue()->type(node->lvalue()->type());
      else
        throw std::string("Invalid expression for lvalue node.");
    }
    else if(stackr != nullptr) {
      if(node->lvalue()->is_typed(cdk::TYPE_POINTER))
        node->rvalue()->type(node->lvalue()->type());
      else 
        throw std::string("A pointer is required to allocate.");
    }
    else
      throw std::string("Unknown node with unspecified type");
  }

  if(node->lvalue()->is_typed(cdk::TYPE_INT) && node->rvalue()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
  }
  else if(node->lvalue()->is_typed(cdk::TYPE_DOUBLE) && (node->rvalue()->is_typed(cdk::TYPE_DOUBLE) || node->rvalue()->is_typed(cdk::TYPE_INT))) {
    node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
  }
  else if(node->lvalue()->is_typed(cdk::TYPE_STRING) && node->rvalue()->is_typed(cdk::TYPE_STRING)) {
    node->type(cdk::make_primitive_type(4, cdk::TYPE_STRING));
  }
  else if(node->lvalue()->is_typed(cdk::TYPE_POINTER) && node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
    og::nullptr_node *n = dynamic_cast<og::nullptr_node *>(node->rvalue());

    if(n == nullptr)
      typeOfPointer(cdk::reference_type_cast(node->lvalue()->type()), cdk::reference_type_cast(node->rvalue()->type()));
    node->type(node->lvalue()->type());
  }
  else if(node->lvalue()->is_typed(cdk::TYPE_STRUCT) && node->rvalue()->is_typed(cdk::TYPE_STRUCT))
    throw std::string("Cheguei");
  else {
    throw std::string("wrong types in assignment");
  }
  
}

//---------------------------------------------------------------------------

void og::type_checker::do_evaluation_node(og::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);

  if(node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *input = dynamic_cast<og::input_node *>(node->argument());

    if(input != nullptr) {
      node->argument()->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    }
    else {
      throw std::string("Unknown node with unspecified type.");
    }
  }
}

void og::type_checker::do_write_node(og::write_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);

  for(size_t i = 0; i < node->argument()->size(); i++) {
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->argument()->node(i));
     if (expression != nullptr && expression->is_typed(cdk::TYPE_VOID)){
        throw std::string("Wrong type in write argument.");
     }

  }
}

//---------------------------------------------------------------------------
// INPUT NODE

void og::type_checker::do_input_node(og::input_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------
// LOOPS

void og::type_checker::do_for_node(og::for_node *const node, int lvl) {
  if (node->inits())
    node->inits()->accept(this, lvl + 4);
  if (node->condition())
    node->condition()->accept(this, lvl + 4);
  if (node->incrs())
    node->incrs()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------
// CONDITIONAL OPERATIONS

void og::type_checker::do_if_node(og::if_node * const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  if (node->condition()->is_typed(cdk::TYPE_UNSPEC))
  {
    og::input_node *input = dynamic_cast<og::input_node *>(node->condition());

    if(input != nullptr) {
      node->condition()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    else {
      throw std::string("Unknown node with unspecified type.");
    }

  }
  else if (!node->condition()->is_typed(cdk::TYPE_INT)) 
    throw std::string("Expected integer condition.");
}


void og::type_checker::do_if_else_node(og::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  if (node->condition()->is_typed(cdk::TYPE_UNSPEC))
  {
    og::input_node *input = dynamic_cast<og::input_node *>(node->condition());

    if(input != nullptr) {
      node->condition()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    else {
      throw std::string("Unknown node with unspecified type.");
    }

  }
  else if (!node->condition()->is_typed(cdk::TYPE_INT)) 
    throw std::string("Expected integer condition.");
}

//---------------------------------------------------------------------------
// FUNCTIONS

void og::type_checker::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = main_function(node->identifier());
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);

  if (symbol == nullptr) throw std::string("Symbol '" + id + "' is undeclared.");

  if (!symbol->isFunction()) throw std::string("Symbol '" + id + "' is not a function.");

  node->type(symbol->type());

  if (node->arguments() && symbol->arguments().size() != 0) {
      node->arguments()->accept(this, lvl + 4);
      verifyArguments(symbol,node->arguments());
  }
  else if ((!node->arguments() && symbol->arguments().size() != 0) || (node->arguments() && symbol->arguments().size() == 0)){
    throw std::string("'" + symbol->name() + "' called with wrong number of arguments.");
  }
}

void og::type_checker::do_function_definition_node(og::function_definition_node *const node, int lvl) {
  const std::string &id = main_function(node->identifier());
  std::shared_ptr<og::symbol> function;
  std::shared_ptr<og::symbol> previous;

  if (node->block())
  {
    function = std::make_shared<og::symbol>(node->qualifier(), node->type(), id, true, 0, false);
    function->set_offset(-node->type()->size());
    std::vector<std::shared_ptr<cdk::basic_type>> *argsTypes = new std::vector<std::shared_ptr<cdk::basic_type>>();
    if (node->arguments()) {
      for(size_t i = 0; i < node->arguments()->size(); i++) {
        og::variable_declaration_node *arg = dynamic_cast<og::variable_declaration_node *>(node->arguments()->node(i));
        if(arg != nullptr)
          argsTypes->push_back(arg->type());
      }
    }
    function->setArguments(*argsTypes);
    previous = _symtab.find(function->name());
    if(previous) {
      if(previous->isFunction()) {
        if(!previous->forward())
           throw std::string("'" + function->name() + "' has already been defined.");

        if(function->qualifier() != previous->qualifier())
          throw std::string("'" + function->name() + "' declaration has a different qualifier.");
  
        if(function->type()->name() != previous->type()->name()) 
          throw std::string("'" + function->name() + "' redeclaration has a different return type.");
        
        if(function->type()->name() == cdk::TYPE_POINTER && 
        previous->type()->name() == cdk::TYPE_POINTER)
          typeOfPointer(cdk::reference_type_cast(previous->type()), cdk::reference_type_cast(function->type()));
        
        if(previous->arguments().size() == function->arguments().size()) {
          for(size_t i = 0; i < function->arguments().size(); i++) {
            if(previous->arguments().at(i)->name() != function->arguments().at(i)->name()) 
              throw std::string("'" + function->name() + "' has no matching arguments types.");
            else if(previous->arguments().at(i)->name() == cdk::TYPE_POINTER && function->arguments().at(i)->name() == cdk::TYPE_POINTER)
              typeOfPointer(cdk::reference_type_cast(previous->arguments().at(i)), cdk::reference_type_cast(function->arguments().at(i)));
          }
        }
        else
          throw std::string("'" + function->name() + "' has a different number of arguments.");
        _symtab.replace(function->name(), function);
      }
      else
        throw std::string("'" + function->name() + "' has already been declared as a different type of symbol.");

    }
    else
      _symtab.insert(function->name(), function);

    _parent->set_new_symbol(function);
  }
  else
  {
    function = std::make_shared<og::symbol>(node->qualifier(), node->type(), id, true, 0 , true);
    std::vector<std::shared_ptr<cdk::basic_type>> *argsTypes = new std::vector<std::shared_ptr<cdk::basic_type>>();
    if (node->arguments()) {
      for(size_t i = 0; i < node->arguments()->size(); i++) {
        auto m = dynamic_cast<og::variable_declaration_node *>(node->arguments()->node(i));
        if(m != nullptr) {
          argsTypes->push_back(m->type());
        }
      }
    }
    function->setArguments(*argsTypes);
    previous = _symtab.find(function->name());

    if(previous){
      if (previous->isFunction()) {
        if(function->qualifier() != previous->qualifier()) {
        throw std::string("'" + function->name() + "' declaration has a different qualifier.");
      }

      if(function->type()->name() != previous->type()->name())
        throw std::string("'" + function->name() + "' redeclaration has a different return type.");
      if(function->type()->name() == cdk::TYPE_POINTER 
      && previous->type()->name() == cdk::TYPE_POINTER)
        typeOfPointer(cdk::reference_type_cast(previous->type()), cdk::reference_type_cast(function->type()));

      if(previous->arguments().size() == function->arguments().size()) {
        for(size_t i = 0; i < function->arguments().size(); i++) {
          if(previous->arguments().at(i)->name() != function->arguments().at(i)->name())
            throw std::string("'" + function->name() + "' has no matching arguments types.");
          else if(previous->arguments().at(i)->name() == cdk::TYPE_POINTER && function->arguments().at(i)->name() == cdk::TYPE_POINTER)
            typeOfPointer(cdk::reference_type_cast(previous->arguments().at(i)), cdk::reference_type_cast(function->arguments().at(i)));
        
        }
      }
      else
        throw std::string("'" + function->name() + "' has a different number of arguments.");

      _parent->set_new_symbol(previous);
      }
    }
    else {
      _symtab.insert(function->name(),function);
      _parent->set_new_symbol(function);
    }
  }
}

//---------------------------------------------------------------------------
// OTHER OPERATIONS

void og::type_checker::do_nullptr_node(og::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_reference_type(4, nullptr));
}

void og::type_checker::do_return_node(og::return_node *const node, int lvl) {
    if (node->retval()) {
      if (_function->type()->name() == cdk::TYPE_VOID)
        throw std::string("Void function cannot return values.");
      
      node->retval()->accept(this, lvl + 2);

      if (_function->type()->name() == cdk::TYPE_INT) {
        if (!node->retval()->is_typed(cdk::TYPE_INT))
          throw std::string("Wrong type for initializer (integer expected).");
      } else if (_function->type()->name() == cdk::TYPE_DOUBLE) {
        if (!node->retval()->is_typed(cdk::TYPE_INT) &&
        !node->retval()->is_typed(cdk::TYPE_DOUBLE))
          throw std::string("Wrong type for initializer (integer or double expected).");
      } else if (_function->type()->name() == cdk::TYPE_STRING) {
        if (!node->retval()->is_typed(cdk::TYPE_STRING))
          throw std::string("Wrong type for initializer (string expected).");
      } else if (_function->type()->name() == cdk::TYPE_POINTER) {
        typeOfPointer(cdk::reference_type_cast(node->retval()->type()), 
        cdk::reference_type_cast(_function->type()));
      } else {
        throw std::string("Unknown type for return expression.");
      }
    }
}

void og::type_checker::do_stack_alloc_node(og::stack_alloc_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if(node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *input = dynamic_cast<og::input_node *>(node->argument());

    if(input != nullptr)
      node->argument()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in allocation expression.");

  node->type(cdk::make_primitive_type(0, cdk::TYPE_UNSPEC));
}

void og::type_checker::do_address_of_node(og::address_of_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  if (!node->lvalue()->is_typed(cdk::TYPE_UNSPEC) &&
  !node->lvalue()->is_typed(cdk::TYPE_VOID))
    node->type(cdk::make_reference_type(4, node->lvalue()->type()));
  else
    throw std::string("Wrong type in unary logical expression.");
}

void og::type_checker::do_left_index_node(og::left_index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  if (!node->base()->is_typed(cdk::TYPE_POINTER))
    throw std::string("Index left-value must be a pointer.");

  node->index()->accept(this, lvl + 2);
  if(node->index()->is_typed(cdk::TYPE_UNSPEC)) {
    og::input_node *input = dynamic_cast<og::input_node *>(node->index());

    if(input != nullptr)
      node->index()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if (!node->index()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in left-value index.");

  node->type(cdk::reference_type_cast(node->base()->type())->referenced());
}

void og::type_checker::do_block_node(og::block_node *const node, int lvl) {
  if (node->declarations())
    node->declarations()->accept(this, lvl + 2);
  
  if (node->instructions())
    node->instructions()->accept(this, lvl + 2);
}

void og::type_checker::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_tuple_node(og::tuple_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->sequence()->accept(this, lvl + 2);
  std::vector<std::shared_ptr<cdk::basic_type>> *types = new std::vector<std::shared_ptr<cdk::basic_type>>();
  size_t tupleSize = node->sequence()->size();

  if (tupleSize > 1) {
    for (size_t i = 0; i < tupleSize; i++) {
      cdk::expression_node *expression = dynamic_cast<cdk::expression_node*>(node->sequence()->node(i));
      types->push_back(expression->type());
    }
    node->type(cdk::make_structured_type(*types));
  }
  else {
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node*>(node->sequence()->node(0));
    node->type(expression->type());
  }
}

void og::type_checker::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  og::tuple_node *tuple = dynamic_cast<og::tuple_node*>(node->base());
  if ((size_t) node->index() > tuple->sequence()->size() - 1)
    throw std::string("Right value index out of range.");
}

void og::type_checker::do_break_node(og::break_node *const node, int lvl) {
  // EMPTY
}

void og::type_checker::do_continue_node(og::continue_node *const node, int lvl) {
  // EMPTY
}