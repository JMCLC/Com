#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include "ast/all.h" 
#include "og_parser.tab.h"


//---------------------------------------------------------------------------

void og::postfix_writer::do_power_node(og::power_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int i = 1;
  cdk::integer_node *intNode = dynamic_cast<cdk::integer_node*>(node->right());

  _pf.INT(1);
  while (i <= intNode->value()) {
    node->left()->accept(this, lvl);
    if(node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT))
      _pf.I2D();

    _pf.MUL();
    ++i;
  }

}

void og::postfix_writer::do_while_node(og::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  node->condition()->accept(this, lvl + 2);
  _pf.JZ(mklbl(lbl2 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2));
}


void og::postfix_writer::do_do_while_node(og::do_while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  node->condition()->accept(this, lvl + 2);
  _pf.JZ(mklbl(lbl2 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl1));
  _pf.LABEL(mklbl(lbl2));
}

void og::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void og::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}


//---------------------------------------------------------------------------

void og::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if(_inFunctionBody)
    _pf.INT(node->value());
  else
    _pf.SINT(node->value());
}

void og::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if(_inFunctionBody){
    _pf.DOUBLE(node->value());
  }
  else{
    _pf.SDOUBLE(node->value());
  }
}

void og::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  _pf.RODATA(); 
  _pf.ALIGN(); 
  _pf.LABEL(mklbl(lbl1 = ++_lbl));
  _pf.SSTRING(node->value());

  _pf.ALIGN();
  if (_inFunctionBody) {
    _pf.TEXT();
    _pf.ADDR(mklbl(lbl1));
  }
  else
  {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------
void og::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
   ASSERT_SAFE_EXPRESSIONS;
   node->argument()->accept(this, lvl);
   _pf.INT(0);
   _pf.EQ();
}

void og::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DNEG();
  }
  else if(node->is_typed(cdk::TYPE_INT)) {
    _pf.NEG();
  }
}
void og::postfix_writer::do_identity_node(og::identity_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  else if(node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {

    if(cdk::reference_type_cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE) {
      _pf.INT(3);
    }
    else {
      _pf.INT(2);
    }
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)){
   if(cdk::reference_type_cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE) {
      _pf.INT(3);
    }
    else {
      _pf.INT(2);
    }
    _pf.SHTL();
  }

  if(node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DADD();
  }
  else {
    _pf.ADD();
  }
}
void og::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
   else if(node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    if(cdk::reference_type_cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE) {
      _pf.INT(3);
    }
    else {
      _pf.INT(2);
    }
    _pf.SHTL();
  }

  node->right()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  else if(node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    if(cdk::reference_type_cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE) {
      _pf.INT(3);
    }
    else {
      _pf.INT(2);
    }
    _pf.SHTL();
  }
  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)){
    int lbl1;

    _pf.SUB();
    _pf.INT(cdk::reference_type_cast(node->left()->type())->referenced()->size());
    _pf.DIV();
    _pf.DUP32();
    _pf.INT(0);
    _pf.LT(); 
    _pf.JZ(mklbl(lbl1 = ++_lbl));
    _pf.NEG();
    _pf.ALIGN();
    _pf.LABEL(mklbl(lbl1));
  }
  else{
    if(node->is_typed(cdk::TYPE_DOUBLE)) {
        _pf.DSUB();
      }
      else {
        _pf.SUB();
      }
  }
}

void og::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if(node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DMUL();
  }
  else {
    _pf.MUL();
  }
}

void og::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if(node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DDIV();
  }
  else {
    _pf.DIV();
  }
  
}
void og::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}
void og::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.LT();
}
void og::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.LE();
}
void og::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.GE();
}
void og::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.GT();
}
void og::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.NE();
}
void og::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  if(node->right()->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }
  if(node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
  _pf.EQ();
}

void og::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
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


void og::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
 ASSERT_SAFE_EXPRESSIONS;
 int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this, lvl);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}
//---------------------------------------------------------------------------

void og::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if(_inFunctionBody) {
    std::shared_ptr<og::symbol> var = _symtab.find(node->name());
    if(var->offset() == 0)
      _pf.ADDR(var->name());
    else
      _pf.LOCAL(var->offset());
  }
  else
    _pf.ADDR(node->name()); 
}

void og::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
  if(node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER) || 
     node->is_typed(cdk::TYPE_STRING)) 
    _pf.LDINT();
  else if(node->is_typed(cdk::TYPE_DOUBLE))
    _pf.LDDOUBLE();
}

void og::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl); 
  if(node->is_typed(cdk::TYPE_DOUBLE)){
    if(node->rvalue()->is_typed(cdk::TYPE_INT))
      _pf.I2D();
    _pf.DUP64();
  }
  else
    _pf.DUP32();

  node->lvalue()->accept(this, lvl);
  if(node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  }
  else {
    _pf.STINT();
  }
}


//---------------------------------------------------------------------------

void og::postfix_writer::do_evaluation_node(og::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if (node->argument()->is_typed(cdk::TYPE_INT) || 
   node->argument()->is_typed(cdk::TYPE_STRING) ||
   node->argument()->is_typed(cdk::TYPE_POINTER) ) {
    _pf.TRASH(4);
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.TRASH(8);
  } else if (!node->argument()->is_typed(cdk::TYPE_VOID) &&
  !node->argument()->is_typed(cdk::TYPE_STRUCT)){
    std::cerr << "ERROR: Invalid type." << std::endl;
    exit(1);
  }
}

void og::postfix_writer::do_write_node(og::write_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  for(size_t i = 0; i < node->argument()->size(); i++) {
     cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(node->argument()->node(i));
     cdk::variable_node *variable = dynamic_cast<cdk::variable_node *>(node->argument()->node(i));
    if(expression != nullptr){
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
    }
    else if (variable != nullptr) {
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

void og::postfix_writer::do_input_node(og::input_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if(node->is_typed(cdk::TYPE_INT))
  {
    _pf.CALL("readi");
    _pf.LDFVAL32();
  }
  else if (node->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.CALL("readd");
    _pf.LDFVAL64();
  }
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_for_node(og::for_node * const node, int lvl) {
  
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1 = ++_lbl; _forIni.push(lbl1);
  int lbl2 = ++_lbl; _forStep.push(lbl2);
  int lbl3 = ++_lbl; _forEnd.push(lbl3);

  int i;
  int size = 0;
  cdk::expression_node *expNode, *lastCondition;

  _symtab.push();
 
  if(node->inits()) {
    node->inits()->accept(this,lvl+2);
  }
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));



  if(node->condition()) {
    int last = node->condition()->size()-1;
    lastCondition = dynamic_cast<cdk::expression_node *>(node->condition()->node(last));
  }

  if(node->condition()) {
    node->condition()->accept(this, lvl); 
    if(lastCondition->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.D2I();
    }
  }

  _pf.JZ(mklbl(lbl3));
  node->block()->accept(this, lvl + 2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl2));

  if(node->incrs()) {
    node->incrs()->accept(this, lvl);
  }
  _pf.JMP(mklbl(lbl1));

  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl3));

  if(node->inits()) {
    for(i = 0; i < (int)(node->inits()->size()); i++) {
      expNode = dynamic_cast<cdk::expression_node *>(node->inits()->node(i));
      if(expNode) {
        if(expNode->is_typed(cdk::TYPE_INT) || expNode->is_typed(cdk::TYPE_POINTER)) {
          size =+ 4;
        }
        if(expNode->is_typed(cdk::TYPE_DOUBLE)) {
          size =+ 8;
        }
      }
    }
  }

   if(node->condition()) {
    for(i = 0; i < (int)(node->condition()->size()-1); i++) {
      expNode = dynamic_cast<cdk::expression_node *>(node->condition()->node(i));
      if(expNode->is_typed(cdk::TYPE_INT) || expNode->is_typed(cdk::TYPE_POINTER)) {
          size =+ 4;
        }
      if(expNode->is_typed(cdk::TYPE_DOUBLE)) {
        size =+ 8;
      }
    }
  }

   if(node->incrs()) {
    for(i = 0; i < (int)(node->incrs()->size()); i++) {
      expNode = dynamic_cast<cdk::expression_node *>(node->incrs()->node(i));
      if(expNode->is_typed(cdk::TYPE_INT) || expNode->is_typed(cdk::TYPE_POINTER)) {
          size =+ 4;
        }
      if(expNode->is_typed(cdk::TYPE_DOUBLE)) {
        size =+ 8;
      }
    }
  }

  _symtab.pop();
  _pf.TRASH(size);
  _forIni.pop();
  _forStep.pop();
  _forEnd.pop();
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_if_node(og::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));

  node->block()->accept(this, lvl + 2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void og::postfix_writer::do_if_else_node(og::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));

  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));

  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);

  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl2));
}


void og::postfix_writer::do_function_call_node(og::function_call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  std::string id = node->identifier();
  if (id == "og")
    id = "_main";
  else if (id == "_main")
    id = "._main";
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);
  int argumentsSize = 0;
  if(node->arguments())
  {
    for (size_t i = node->arguments()->size();i > 0; i--)
    {
      std::shared_ptr<cdk::basic_type> type = symbol->arguments().at(i - 1);
      cdk::expression_node *arg = dynamic_cast<cdk::expression_node *>(node->arguments()->node(i - 1));
      arg->accept(this, lvl + 2);
      if(type->name() == cdk::TYPE_DOUBLE && arg->is_typed (cdk::TYPE_INT)) {
        _pf.I2D();
      }
      argumentsSize += type->size();
    }
  }
   _pf.CALL(id);
  if(argumentsSize != 0){
    _pf.TRASH(argumentsSize);
  }


  if(symbol->type()->name() == cdk::TYPE_INT || symbol->type()->name() == cdk::TYPE_POINTER || 
    symbol->type()->name() == cdk::TYPE_STRING) {
    _pf.LDFVAL32();
  }
  else if(symbol->type()->name() == cdk::TYPE_DOUBLE) {
    _pf.LDFVAL64();
  }
  else if(symbol->type()->name() != cdk::TYPE_VOID) {
    std::cerr << "Cannot call function '" + id + "'" << std::endl;
  }

}

void og::postfix_writer::do_function_definition_node(og::function_definition_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if(_inFunctionBody || _inFunctionArgs) {
    std::cerr << node->lineno() << ": Can only declare a function outside of another function body and arguments.";
    return;
  }
  if (node->block())
  {
    std::string id;
    if(node->identifier() == "og") {
      for (std::string s : _functions_to_declare) {
        _pf.EXTERN(s);
      } 
      id = "_main";
    }
    else if(node->identifier() == "_main") {
      id = "._main";
    }
    else {
      id = node->identifier();
    }
    _function = new_symbol();
    _functions_to_declare.erase(_function->name());
    reset_new_symbol();
    _offset = 8;
    _symtab.push();
    if(node->arguments()){
      _inFunctionArgs = true;
       node->arguments()->accept(this,lvl+2);
       _inFunctionArgs = false;
    }
    _pf.TEXT();
    _pf.ALIGN();
    if (node->qualifier() == tPUBLIC) 
      _pf.GLOBAL(_function->name(), _pf.FUNC());
    _pf.LABEL(_function->name());
    frame_size_calculator lsc(_compiler, _symtab);
    node->accept(&lsc, lvl);
    _pf.ENTER(lsc.localsize());
    _inFunctionBody = true;
    _offset = 0;
    node->block()->accept(this, lvl + 4);
    _inFunctionBody = false;
    _symtab.pop();

    _pf.LEAVE();
    _pf.RET();
  }

  else {
    if(!new_symbol()) {
      return;
    }

    std::shared_ptr<og::symbol> function = new_symbol();
    reset_new_symbol();
    _functions_to_declare.insert(function->name());
  }
}

void og::postfix_writer::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  std::vector<std::string> *identifiers = new std::vector<std::string>();
  std::string id = node->identifier();
  std::string delimiter = ",";
  size_t pos = 0;
  std::string token;
  while ((pos =  id.find(delimiter)) != std::string::npos) {
      token =  id.substr(0, pos);
      identifiers->push_back(token);
      id.erase(0, pos + delimiter.length());
  } 
  identifiers->push_back(id);
  size_t size = identifiers->size();
  if (node->is_typed(cdk::TYPE_STRUCT) && size != 1) {
    for (size_t i = 0; i < size; i++) {
      id = identifiers->at(i);
      int offset = 0;
      int size = cdk::structured_type_cast(node->type())->component(i)->size();
      std::shared_ptr<cdk::basic_type> type = cdk::structured_type_cast(node->type())->component(i);
      og::tuple_node *tuple = dynamic_cast< og::tuple_node *>(node->initializer());
      cdk::expression_node *expression = dynamic_cast<cdk::expression_node *>(tuple->sequence()->node(i));
       
      if(_inFunctionBody) {
        _offset -= size;
        offset = _offset;
      }
      else if(_inFunctionArgs) {
        offset = _offset;
        _offset += size;
      }
      else
        offset = 0;

      std::shared_ptr<og::symbol> symbol = new_symbol();

      if(symbol) {
        symbol->set_offset(offset);
        reset_new_symbol();
      }

      if(_inFunctionBody) { 
        expression->accept(this, lvl);
        if(type->name() == cdk::TYPE_DOUBLE) {
          if(expression->is_typed(cdk::TYPE_INT))
            _pf.I2D();

          _pf.LOCAL(offset);
          _pf.STDOUBLE();
        }
        else {
          _pf.LOCAL(offset);
          _pf.STINT();
        }
      }
      else if (!_inFunctionBody && !_inFunctionArgs)
      {
        if(type->name() == cdk::TYPE_INT || type->name() == cdk::TYPE_POINTER
        || type->name() == cdk::TYPE_DOUBLE) {
          _pf.DATA();
          _pf.ALIGN();
          if (node->qualifier() == tPUBLIC) {
            _pf.GLOBAL(id, _pf.OBJ());
          }
          _pf.LABEL(id);
          if(type->name() == cdk::TYPE_INT || type->name() == cdk::TYPE_POINTER) {
            expression->accept(this, lvl);
          }
          else if (type->name() == cdk::TYPE_DOUBLE){
            if(expression->is_typed(cdk::TYPE_DOUBLE))
            {
              expression->accept(this, lvl);
            }
            else if(expression->is_typed(cdk::TYPE_INT)) {
              cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(expression);
              cdk::double_node ddi(dclini->lineno(), dclini->value());
              ddi.accept(this, lvl);
            }
            else {
              std::cerr << node->lineno() << ": '" << id << "' wrong initializer for real variable\n";
              exit(2);
            }
          }
        }
        else if (type->name() == cdk::TYPE_STRING) {
          _pf.DATA(); 
          _pf.ALIGN();
          _pf.LABEL(id);
          expression->accept(this, lvl);
        }
      }
      else {
        std::cerr << node->lineno() << ": '" << id << "' has unexpected initializer\n";
        exit(2);
      }
    }
  }
   else {
     int offset = 0;
     int size = node->type()->size();
     if(_inFunctionBody) {
      _offset -= size;
      offset = _offset;
     }
     else if(_inFunctionArgs) {
      offset = _offset;
      _offset += size;
    }
    else
      offset = 0;

    std::shared_ptr<og::symbol> symbol = new_symbol();

    if(symbol) {
      symbol->set_offset(offset);
      reset_new_symbol();
    }

    if (node->initializer()) {
      if(_inFunctionBody) { 
        node->initializer()->accept(this, lvl);
        if(node->is_typed(cdk::TYPE_DOUBLE)) {
          if(node->initializer()->is_typed(cdk::TYPE_INT))
            _pf.I2D();

          _pf.LOCAL(offset);
          _pf.STDOUBLE();
        }
        else {
          _pf.LOCAL(offset);
          _pf.STINT();
        }
      }
      else if (!_inFunctionBody && !_inFunctionArgs)
      {
        if(node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER)
        || node->is_typed(cdk::TYPE_DOUBLE)) {
          _pf.DATA();
          _pf.ALIGN();
          if (node->qualifier() == tPUBLIC) {
            _pf.GLOBAL(id, _pf.OBJ());
          }
          _pf.LABEL(id);
          if(node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER)) {
            node->initializer()->accept(this, lvl);
          }
          else if (node->is_typed(cdk::TYPE_DOUBLE)){
            if(node->initializer()->is_typed(cdk::TYPE_DOUBLE))
              node->initializer()->accept(this, lvl);
            else if(node->initializer()->is_typed(cdk::TYPE_INT)) {
              cdk::integer_node *dclini = dynamic_cast<cdk::integer_node *>(node->initializer());
              cdk::double_node ddi(dclini->lineno(), dclini->value());
              ddi.accept(this, lvl);
            }
            else {
              std::cerr << node->lineno() << ": '" << id << "' wrong initializer for real variable.\n";
              exit(2);
            }
          }
        }
        else if (node->is_typed(cdk::TYPE_STRING)) {
          _pf.DATA(); 
          _pf.ALIGN();
          _pf.LABEL(node->identifier());
          node->initializer()->accept(this, lvl); 
        }
      }
      else {
        std::cerr << node->lineno() << ": '" << id << "' has an unexpected initializer.\n";
        exit(2);
      }
    }
    else 
    {
      if (!_inFunctionBody && !_inFunctionArgs && (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_POINTER)
        || node->is_typed(cdk::TYPE_DOUBLE) || node->is_typed(cdk::TYPE_STRING)) )
      {
        _pf.BSS();
        _pf.ALIGN();
        _pf.LABEL(id);
        _pf.SALLOC(size);
      }
    }
  }
}

void og::postfix_writer::do_break_node(og::break_node *const node, int lvl) {
  if(_forEnd.size() > 0)
    _pf.JMP(mklbl(_forEnd.top()));
  else
    std::cerr << "Break instruction can only be used inside a for cycle." << std::endl;
}

void og::postfix_writer::do_continue_node(og::continue_node *const node, int lvl) {
  if(_forStep.size() > 0)
  _pf.JMP(mklbl(_forStep.top()));
  else
    std::cerr << "Continue instruction can only be used inside a for cycle." << std::endl;
}

void og::postfix_writer::do_return_node(og::return_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  og::tuple_node *tuple = dynamic_cast<og::tuple_node*>(node->retval());
  for (size_t i = 0; i < tuple->sequence()->size(); i++) {
    if (!_function->is_typed(cdk::TYPE_VOID)) {
    tuple->sequence()->node(i)->accept(this, lvl + 2);

    if (_function->is_typed(cdk::TYPE_INT) || _function->is_typed(cdk::TYPE_STRING)
        || _function->is_typed(cdk::TYPE_POINTER))
      _pf.STFVAL32();
    else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
      cdk::expression_node *expression = dynamic_cast<cdk::expression_node*>(tuple->sequence()->node(i));
      if (expression->is_typed(cdk::TYPE_INT))
        _pf.I2D();
      _pf.STFVAL64();
    } else
      std::cerr << node->lineno() << ": Unknown types for function return instruction." << std::endl;
    }
  }

  _pf.LEAVE();
  _pf.RET();

}

void og::postfix_writer::do_nullptr_node(og::nullptr_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (_inFunctionBody) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}

void og::postfix_writer::do_stack_alloc_node(og::stack_alloc_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  if(cdk::reference_type_cast(node->type())->referenced()->name() == cdk::TYPE_DOUBLE)
    _pf.INT(3);
  else
    _pf.INT(2);

  _pf.SHTL();
  _pf.ALLOC();    
  _pf.SP();
}

void og::postfix_writer::do_address_of_node(og::address_of_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS
  node->lvalue()->accept(this, lvl);
}

void og::postfix_writer::do_left_index_node(og::left_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);
  _pf.INT(node->type()->size());
  _pf.MUL();
  _pf.ADD(); 
}

void og::postfix_writer::do_block_node(og::block_node *const node, int lvl) {
  if(node->declarations())
    node->declarations()->accept(this,lvl);

  if(node->instructions())
    node->instructions()->accept(this,lvl);
}

void og::postfix_writer::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  og::tuple_node *tuple = dynamic_cast<og::tuple_node*>(node->argument());

  if (tuple != nullptr) {
    for (size_t i = 0; i < tuple->sequence()->size(); i++) {
      cdk::typed_node *typed = dynamic_cast<cdk::typed_node*>(tuple->sequence()->node(i));
      _pf.INT(typed->type()->size());
      _pf.ADD();
    }
  }
}

void og::postfix_writer::do_tuple_node(og::tuple_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->sequence()->accept(this, lvl + 2);
}

void og::postfix_writer::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->base()->accept(this, lvl + 2);

  int stop = -1;
  og::tuple_node *tuple = dynamic_cast<og::tuple_node*>(node->base());
  for (size_t i = 0; i < tuple->sequence()->size(); i++) {
    if (stop == node->index())
      break;

    cdk::typed_node *typed = dynamic_cast<cdk::typed_node*>(tuple->sequence()->node(i));
    _pf.INT(typed->type()->size());
    _pf.ADD();

    stop++;
  }

  _pf.ADD();

}


