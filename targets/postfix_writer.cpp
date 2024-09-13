#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated
#include "mml_parser.tab.h"

//---------------------------------------------------------------------------

void mml::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void mml::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
//---------------------------------------------------------------------------

void mml::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunction)
    _pf.INT(node->value()); // push an integer
  else
    _pf.SINT(node->value());
}

void mml::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  _pf.ALIGN();
  if (_inFunction) {
    /* leave the address on the stack */
    _pf.TEXT(); // return to the TEXT segment
    _pf.ADDR(mklbl(lbl1)); // the string to be printed    
  } else {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

void mml::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunction)
    _pf.DOUBLE(node->value());
  else
    _pf.SDOUBLE(node->value());
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  _pf.NEG(); // 2-complement
}

void mml::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.ADD();
}
void mml::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.SUB();
}
void mml::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MUL();
}
void mml::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.DIV();
}
void mml::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void mml::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.LT();
}
void mml::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.LE();
}
void mml::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.GE();
}
void mml::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.GT();
}
void mml::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.NE();
}
void mml::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.EQ();
}
void mml::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}
void mml::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunction) {
    std::shared_ptr<mml::symbol> symbol = _symtab.find(node->name());
    if (symbol->offset() == 0) {
      _pf.ADDR(symbol->name());
    } else {
      _pf.LOCAL(symbol->offset());
    }
  } else {
    // simplified generation: all variables are global
    _pf.ADDR(node->name());
  }
}

void mml::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER) || 
     node->is_typed(cdk::TYPE_STRING)) 
    _pf.LDINT();
  else if(node->is_typed(cdk::TYPE_DOUBLE))
    _pf.LDDOUBLE();
}

void mml::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl); 
  if(node->is_typed(cdk::TYPE_DOUBLE)) {
    if(node->rvalue()->is_typed(cdk::TYPE_INT))
      _pf.I2D();
    _pf.DUP64();
  }
  else
    _pf.DUP32();

  node->lvalue()->accept(this, lvl);
  if(node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_evaluation_node(mml::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if (node->argument()->is_typed(cdk::TYPE_INT) || node->argument()->is_typed(cdk::TYPE_STRING) || node->argument()->is_typed(cdk::TYPE_POINTER) ) {
    _pf.TRASH(4);
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.TRASH(8);
  } else if (!node->argument()->is_typed(cdk::TYPE_VOID)){
    std::cerr << "ERROR: Invalid type." << std::endl;
    exit(1);
  }
}

void mml::postfix_writer::do_print_node(mml::print_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value to print
  for (size_t i = 0; i < node->argument()->size(); i++) {
    cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->argument()->node(i));
    cdk::variable_node *variable = dynamic_cast<cdk::variable_node *>(node->argument()->node(i));
    if (expression != nullptr) {
      if (expression->is_typed(cdk::TYPE_INT)) {
        _pf.CALL("printi");
        _pf.TRASH(4);
      } else if (expression->is_typed(cdk::TYPE_STRING)) {
        _pf.CALL("prints");
        _pf.TRASH(4);
      }
      else if (expression->is_typed(cdk::TYPE_DOUBLE))
      {
        _pf.CALL("printd");
        _pf.TRASH(8);
      } 
      else if (!expression->is_typed(cdk::TYPE_STRUCT)) {
        std::cerr << "ERROR: Invalid type." << std::endl;
        exit(1);
      }
    } else if (variable != nullptr) {
      if (variable->is_typed(cdk::TYPE_INT)) {
        _pf.CALL("printi");
        _pf.TRASH(4);
      } else if (variable->is_typed(cdk::TYPE_STRING)) {
        _pf.CALL("prints");
        _pf.TRASH(4);
      }
      else if (variable->is_typed(cdk::TYPE_DOUBLE))
      {
        _pf.CALL("printd");
        _pf.TRASH(8);
      }
      else if (!expression->is_typed(cdk::TYPE_STRUCT)) {
        std::cerr << "ERROR: Invalid type." << std::endl;
        exit(1);
      }
    }
    else {
      std::cerr << "ERROR: Invalid type." << std::endl;
        exit(1);
    }
  }
  if(node->newline())
    _pf.CALL("println");
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_read_node(mml::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.CALL("readi");
  _pf.LDFVAL32();
  _pf.STINT();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_while_node(mml::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1 = ++_lbl, lbl2 = ++_lbl;
  _whileIni.push(lbl1);
  _whileEnd.push(lbl2);
  _pf.LABEL(mklbl(lbl1));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl2));
  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2));
  _whileIni.pop();
  _whileEnd.pop();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_node(mml::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_else_node(mml::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}


void mml::postfix_writer::do_address_node(mml::address_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
}

void mml::postfix_writer::do_alloc_node(mml::alloc_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if(cdk::reference_type::cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
  else
    _pf.INT(2);
  _pf.SHTL();
  _pf.ALLOC();    
  _pf.SP();
}

void mml::postfix_writer::do_return_node(mml::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (!_function->is_typed(cdk::TYPE_VOID)) {
    node->rvalue()->accept(this, lvl + 2);
    if (_function->is_typed(cdk::TYPE_INT) || _function->is_typed(cdk::TYPE_STRING) || _function->is_typed(cdk::TYPE_POINTER)) {
      _pf.STFVAL32();
    } else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
      if (node->rvalue()->is_typed(cdk::TYPE_INT))
        _pf.I2D();
      _pf.STFVAL64();
    } else
      std::cerr << node->lineno() << " : Unknown Type in return instruction." << std::endl;
  }
  _pf.LEAVE();
  _pf.RET();
}

void mml::postfix_writer::do_sizeof_node(mml::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  cdk::typed_node *typed = dynamic_cast<cdk::typed_node*>(node->argument());
  _pf.INT(typed->type()->size());
  _pf.ADD();
}

void mml::postfix_writer::do_index_node(mml::index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);
  _pf.INT(node->type()->size());
  _pf.MUL();
  _pf.ADD(); 
}

void mml::postfix_writer::do_stop_node(mml::stop_node * const node, int lvl) {
  if (_whileEnd.size() > 0) {
    _pf.JMP(mklbl(_whileEnd.top()));
  } else
    std::cerr << "Stop can only be used in while loops" << std::endl;
}

void mml::postfix_writer::do_next_node(mml::next_node * const node, int lvl) {
  if (_whileIteration.size() > 0)
    _pf.JMP(mklbl(_whileIteration.top()));
  else
    std::cerr << "Next can only be used in while loops" << std::endl;
}

void mml::postfix_writer::do_block_node(mml::block_node * const node, int lvl) {
  if (node->declarations()) {
    node->declarations()->accept(this, lvl);
  }
  if (node->instructions()) {
    node->instructions()->accept(this, lvl);
  }
}

void mml::postfix_writer::do_function_call_node(mml::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  cdk::rvalue_node *rvalue = dynamic_cast<cdk::rvalue_node*>(node->function());
  cdk::variable_node *var = dynamic_cast<cdk::variable_node*>(rvalue->lvalue());
  std::shared_ptr<mml::symbol> symbol = _symtab.find(var->name());
  int argumentsSize = 0;
  if (node->arguments()->size() > 0) {
    for (size_t i = 0; i < node->arguments()->size(); i++) {
      std::shared_ptr<cdk::basic_type> type = symbol->arguments().at(i  + 1);
      cdk::expression_node *arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i + 1));
      arg->accept(this, lvl + 2);
      if (type->name() == cdk::TYPE_DOUBLE && arg->is_typed(cdk::TYPE_INT))
        _pf.I2D();
      argumentsSize += type->size();
    }
  }
  
  if (rvalue) {
    if (symbol->qualifier() == tFOREIGN)
      _pf.CALL(symbol->name());
    else {
      node->function()->accept(this, lvl);
      _pf.BRANCH();
    }
  }

  if (argumentsSize != 0)
    _pf.TRASH(argumentsSize);
  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.LDFVAL64();
  else
    _pf.LDFVAL32();
}

void mml::postfix_writer::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  std::string id = "_undefined";
  if (node->main()) {
    id = "_main";
  }
  _function = mml::symbol::createSymbol(node->type(), id);
  _offset = 8;
  _symtab.push();
  if (node->arguments()) {
    _inFunctionArguments = true;
    node->arguments()->accept(this, lvl + 2);
    _inFunctionArguments = false;      
  }
  _pf.TEXT();
  _pf.ALIGN();
  if (node->main()) {
    _pf.GLOBAL(id, _pf.FUNC());
    _pf.LABEL(id);
  }
  frame_size_calculator fsc(_compiler, _symtab);
  node->accept(&fsc, lvl);
  _pf.ENTER(fsc.localsize());
  _inFunction = true;
  _offset = 0;
  node->block()->accept(this, lvl + 4);
  _inFunction = false;
  _symtab.pop();
  _pf.LEAVE();
  _pf.RET();    
}

void mml::postfix_writer::do_identity_node(mml::identity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}

void mml::postfix_writer::do_null_node(mml::null_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunction)
    _pf.INT(0);
  else
    _pf.SINT(0);
}

void mml::postfix_writer::do_variable_declaration_node(mml::variable_declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int offset = 0;
  if (_inFunction) {
    _offset -= node->type()->size();
    offset = _offset;
  } else if (_inFunctionArguments) {
    offset = _offset;
    _offset += node->type()->size();
  } else
    offset = 0;
  std::shared_ptr<mml::symbol> symbol = _symtab.find(node->identifier());
  if (symbol == nullptr)
    symbol = mml::symbol::createSymbol(node->type(), node->identifier());
  if (symbol) {
    symbol->set_offset(offset);
  }
  if (node->initializer()) {
    if (_inFunction) {
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT))
          _pf.I2D();
        _pf.LOCAL(offset);
        _pf.STDOUBLE();
      } else {
        _pf.LOCAL(offset);
        _pf.STINT();
      }
    } else if (!_inFunction && !_inFunctionArguments) {
      if (node->is_typed(cdk::TYPE_STRING)) {
        _pf.DATA();
        _pf.ALIGN();
        _pf.LABEL(node->identifier());
        node->initializer()->accept(this, lvl);
      } else if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER)) {
        _pf.DATA();
        _pf.ALIGN();
        if (node->qualifier() == tPUBLIC || !_inFunction) {
          _pf.GLOBAL(node->identifier(), _pf.OBJ());
        }
        _pf.LABEL(node->identifier());
        if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER)) {
          node->initializer()->accept(this, lvl);
        } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
          if (node->initializer()->is_typed(cdk::TYPE_DOUBLE))
            node->initializer()->accept(this, lvl);
          else {
            std::cerr << node->lineno() << ": '" << node->identifier() << "' wrong initializer for double variable" << std::endl;
            exit(2);
          }
        }
      }
    } else {
      std::cerr << node->lineno() << ": '" << node->identifier() << "' has an unexpected initializer." << std::endl;
      exit(2);
    }
  } else {
    if (!_inFunction && !_inFunctionArguments && (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_POINTER))) {
      _pf.BSS();
      _pf.ALIGN();
      _pf.LABEL(node->identifier());
    }
  }
}