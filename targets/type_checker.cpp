#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void mml::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl + 2);
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node*>(node->node(i));
    if (expression != nullptr && expression->is_typed(cdk::TYPE_UNSPEC)) {
      mml::read_node *read = dynamic_cast<mml::read_node*>(expression);
      if (read != nullptr)
        read->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      else
        throw std::string("Unknown node");
    }
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void mml::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

void mml::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

//---------------------------------------------------------------------------

void mml::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in argument of unary expression");

  // in MML, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void mml::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void mml::type_checker::do_identity_node(mml::identity_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void mml::type_checker::do_alloc_node(mml::alloc_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void mml::type_checker::do_sizeof_node(mml::sizeof_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::processBinaryExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of binary expression");

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of binary expression");

  // in MML, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryExpression(node, lvl);
}
void mml::type_checker::do_or_node(cdk::or_node *const node, int lvl) {  
  processBinaryExpression(node, lvl);
}
//---------------------------------------------------------------------------

void mml::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<mml::symbol> symbol = _symtab.find(id);
  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw "Undeclared variable '" + id + "'.";
  }
}

void mml::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

void mml::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
  } catch (const std::string &id) {
    auto symbol = std::make_shared<mml::symbol>(cdk::primitive_type::create(4, cdk::TYPE_INT), id, 0);
    _symtab.insert(id, symbol);
    _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
    node->lvalue()->accept(this, lvl);  //DAVID: bah!
  }

  if (!node->lvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in left argument of assignment expression");

  node->rvalue()->accept(this, lvl + 2);
  if (!node->rvalue()->is_typed(cdk::TYPE_INT)) throw std::string("wrong type in right argument of assignment expression");

  // in MML, expressions are always int
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_evaluation_node(mml::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if(node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    mml::read_node *read = dynamic_cast<mml::read_node *>(node->argument());
    if(read != nullptr) {
      node->argument()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    }
    else {
      throw std::string("Unknown node with unspecified type.");
    }
  }
}

void mml::type_checker::do_print_node(mml::print_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  for(size_t i = 0; i < node->argument()->size(); i++) {
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->argument()->node(i));
     if (expression != nullptr && expression->is_typed(cdk::TYPE_VOID)){
        throw std::string("Wrong type in print argument.");
     }
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_read_node(mml::read_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_while_node(mml::while_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_if_node(mml::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void mml::type_checker::do_if_else_node(mml::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void mml::type_checker::do_address_node(mml::address_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } catch (const std::string &id) {
    throw std::string("Variable not declared: '" + id + "'");
  }
}

void mml::type_checker::do_stop_node(mml::stop_node *const node, int lvl) {
  // EMPTY
}

void mml::type_checker::do_next_node(mml::next_node *const node, int lvl) {
  // EMPTY
}

void mml::type_checker::do_return_node(mml::return_node *const node, int lvl) {
  if (node->rvalue()) {
    if (_function->is_typed(cdk::TYPE_VOID))
      throw std::string("A void function cannot return values");
    node->rvalue()->accept(this, lvl + 2);
    if (_function->is_typed(cdk::TYPE_INT)) {
      if (!node->rvalue()->is_typed(cdk::TYPE_INT))
        throw std::string("Wrong return type (integer expected)");
    } else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
      if (!node->rvalue()->is_typed(cdk::TYPE_INT) && !node->rvalue()->is_typed(cdk::TYPE_DOUBLE) )
        throw std::string("Wrong return type (double expected)");
    } else if (_function->is_typed(cdk::TYPE_STRING)) {
      if (!node->rvalue()->is_typed(cdk::TYPE_STRING))
        throw std::string("Wrong return type (string expected)");
    } else if (_function->is_typed(cdk::TYPE_POINTER)) {
      cdk::reference_type::cast(_function->type());
    } else {
      throw std::string("Unknown type in return expression");
    }
  }
}

void mml::type_checker::do_index_node(mml::index_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->base()->accept(this, lvl + 2);
  if (!node->base()->is_typed(cdk::TYPE_POINTER))
    throw std::string("Left-Value: Base must be a pointer.");
  node->index()->accept(this, lvl + 2);
  if(node->index()->is_typed(cdk::TYPE_UNSPEC)) {
    mml::read_node *read = dynamic_cast<mml::read_node *>(node->index());
    if (read != nullptr)
      node->index()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Left-Value: Node with an unspecified type.");
  } else if (!node->index()->is_typed(cdk::TYPE_INT))
    throw std::string("Left-Value: Index expected to be an integer.");
  node->type(cdk::reference_type::cast(node->base()->type())->referenced());
}

void mml::type_checker::do_block_node(mml::block_node *const node, int lvl) {
  if (node->declarations())
    node->declarations()->accept(this, lvl + 2);
  
  if (node->instructions())
    node->instructions()->accept(this, lvl + 2);
}

void mml::type_checker::do_function_call_node(mml::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;
  cdk::rvalue_node *rvalue = dynamic_cast<cdk::rvalue_node*>(node->function());
  cdk::variable_node *var = dynamic_cast<cdk::variable_node*>(rvalue->lvalue());
  std::shared_ptr<mml::symbol> symbol = _symtab.find(var->name());
  node->type(symbol->type());
  if (node->arguments() && symbol->arguments().size() != 0) {
    node->arguments()->accept(this, lvl + 4);
  } else if ((node->arguments()->size() == 0 && symbol->arguments().size() != 0) || (node->arguments()->size() != 0 && symbol->arguments().size() == 0)){
    throw std::string("'" + symbol->name() + "' called with wrong number of arguments.");
  }
}

void mml::type_checker::do_function_definition_node(mml::function_definition_node *const node, int lvl) {
  std::string id = "_undefined";
  if (node->main())
    id = "_main";
  std::shared_ptr<mml::symbol> function = mml::symbol::createSymbol(node->type(), id);
  _symtab.insert(function->name(), function);
  _parent->set_new_symbol(function);
}

void mml::type_checker::do_null_node(mml::null_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, nullptr));
}

void mml::type_checker::do_variable_declaration_node(mml::variable_declaration_node *const node, int lvl) {
  std::shared_ptr<cdk::basic_type> type = node->type();
  if (node->initializer() != nullptr) {
    node->initializer()->accept(this, lvl + 2);
    if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)) {
      mml::read_node *input = dynamic_cast<mml::read_node*>(node->initializer());
      mml::alloc_node *stack = dynamic_cast<mml::alloc_node*>(node->initializer());
      if(input != nullptr) {
        if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE))
          node->initializer()->type(type);
        else
          throw std::string("Unable to read input.");
      }
      else if (stack != nullptr) {
        if (node->is_typed(cdk::TYPE_POINTER))
          node->initializer()->type(type);
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
    if (node->is_typed(cdk::TYPE_UNSPEC)) {
      node->type(node->initializer()->type());
      type = node->initializer()->type();
    }
  }
  std::shared_ptr<mml::symbol> symbol = _symtab.find(node->identifier());
  if (symbol == nullptr) {
    symbol = mml::symbol::createSymbol(node->qualifier(), type, node->identifier(), 0);
    _symtab.insert(node->identifier(), symbol);
    _parent->set_new_symbol(symbol);
  }
}