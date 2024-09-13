%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;	/* integer value */
  double                d;	/* double value */
  std::string          *s;	/* symbol name or string literal */
  std::vector<std::shared_ptr<cdk::basic_type>> *v;
  std::shared_ptr<cdk::basic_type> variable_type;
  cdk::basic_node      *node;	/* node pointer */
  cdk::typed_node      *typednode;	
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  mml::block_node      *block;
  mml::function_definition_node *function;
  mml::variable_declaration_node *vd;
  mml::while_node *loop;
};

%token <i> tINTEGER
%token <s> tIDENTIFIER tSTRING
%token <d> tDOUBLE
%token tWHILE tIF tPRINT tPRINTLN tBEGIN tEND tARROW
%token <i> tPUBLIC tFOREIGN tFORWARD  
%token <type> tAUTO
%token tINTEGER_TYPE tDOUBLE_TYPE tSTRING_TYPE tVOID_TYPE 
%token tSTOP tNEXT tRETURN
%token tINPUT tNULL tSIZEOF
%token tOR tBRACKET tRBRACKET

%nonassoc tIF
%nonassoc ')'
%nonassoc tELIF tELSE

%right '='
%right '~'
%left tOR tAND
%left tGE tLE tEQ tNE '>' '<'  
%left '+' '-'
%left '*' '/' '%' 

%left '?'

%nonassoc '(' '[' ']' 

%right tUNARY

%type <node> stmt cond_stmt elif 
%type <expression> expr other_expr expr_assign 
%type <lvalue> lval
%type <sequence> file fdeclarations declarations stmts exprs variables other_args
%type <v> variable_types 
%type <function> program function_definition
%type <variable_type> variable_type function_type 
%type <i> qualifier
%type <vd> declaration fdeclaration variable 
%type <block> block
%type <loop> iter_expr
%type <s> string

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file :                                        { compiler->ast($$ = new cdk::sequence_node(LINE)); }
     | fdeclarations                          { compiler->ast($$ = $1); }
     |               program                  { compiler->ast($$ = new cdk::sequence_node(LINE, $1)); }
     | fdeclarations program                  { compiler->ast($$ = new cdk::sequence_node(LINE, $2, $1)); }
     ;

fdeclarations :               fdeclaration      { $$ = new cdk::sequence_node(LINE, $1); }
              | fdeclarations fdeclaration      { $$ = new cdk::sequence_node(LINE, $2, $1); }
              ;

fdeclaration: tPUBLIC  variable_type tIDENTIFIER other_expr            { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, $2, *$3, $4); delete $3; }      
            | tFORWARD  variable_type tIDENTIFIER ';'                  { $$ = new mml::variable_declaration_node(LINE, tFORWARD, $2, *$3, nullptr); delete $3; }
            | tFOREIGN  variable_type tIDENTIFIER ';'                  { $$ = new mml::variable_declaration_node(LINE, tFOREIGN, $2, *$3, nullptr); delete $3; }
            | tPUBLIC   tIDENTIFIER other_expr                         { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, nullptr, *$2, $3); delete $2; } 
            | tPUBLIC   tAUTO tIDENTIFIER expr_assign                  { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, cdk::primitive_type::create(4, cdk::typename_type::TYPE_UNSPEC), *$3, $4); delete $3; } 
            | tAUTO tIDENTIFIER expr_assign                            { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, cdk::primitive_type::create(4, cdk::typename_type::TYPE_UNSPEC), *$2, $3); delete $2; } 
            | variable_type tIDENTIFIER other_expr                     { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, $1, *$2, $3); delete $2; } 
            ; 

program   : tBEGIN declarations stmts tEND            { $$ = new mml::function_definition_node(LINE, nullptr, cdk::primitive_type::create(4, cdk::typename_type::TYPE_INT), new mml::block_node(LINE, $2, $3), true);  }
          | tBEGIN declarations  tEND                 { $$ = new mml::function_definition_node(LINE, nullptr, cdk::primitive_type::create(4, cdk::typename_type::TYPE_INT), new mml::block_node(LINE, $2, nullptr), true);  }
          | tBEGIN  stmts  tEND                       { $$ = new mml::function_definition_node(LINE, nullptr, cdk::primitive_type::create(4, cdk::typename_type::TYPE_INT), new mml::block_node(LINE, nullptr, $2), true);  }
          | tBEGIN  tEND                              { $$ = new mml::function_definition_node(LINE, nullptr, cdk::primitive_type::create(4, cdk::typename_type::TYPE_INT), nullptr, true);  }
          ;

declarations :              declaration      { $$ = new cdk::sequence_node(LINE, $1); }
             | declarations declaration      { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;

block : '{' declarations stmts '}'                {$$ = new mml::block_node(LINE, $2, $3);}
      | '{' declarations       '}'                {$$ = new mml::block_node(LINE, $2, nullptr);}
      | '{'              stmts '}'                {$$ = new mml::block_node(LINE, nullptr, $2);}
      | '{'                    '}'                {$$ = new mml::block_node(LINE, nullptr, nullptr);}
      ;

qualifier : tPUBLIC           { $$ = $1; }
          | tFOREIGN          { $$ = $1; }
          | tFORWARD          { $$ = $1; }
          ;

declaration: qualifier variable_type  tIDENTIFIER other_expr        { $$ = new mml::variable_declaration_node(LINE, $1, $2, *$3, $4); delete $3; }
           |           variable_type  tIDENTIFIER other_expr        { $$ = new mml::variable_declaration_node(LINE, '\0', $1, *$2, $3); delete $2; }
           | tPUBLIC   tAUTO          tIDENTIFIER expr_assign       { $$ = new mml::variable_declaration_node(LINE, $1, cdk::primitive_type::create(4, cdk::typename_type::TYPE_UNSPEC), *$3, $4); delete $3; }
           | tPUBLIC                  tIDENTIFIER expr_assign       { $$ = new mml::variable_declaration_node(LINE, $1, nullptr, *$2, $3); delete $2; }
           | tAUTO                    tIDENTIFIER expr_assign       { $$ = new mml::variable_declaration_node(LINE, tPUBLIC, cdk::primitive_type::create(4, cdk::typename_type::TYPE_UNSPEC), *$2, $3); delete $2; }
           ;

other_expr    : ';'         { $$ = nullptr; }
              | expr_assign       { $$ = $1; }
              ;

expr_assign   : '=' expr ';'      { $$ = $2; }
              ;

stmts :       stmt     { $$ = new cdk::sequence_node(LINE, $1); }
      | stmts stmt     { $$ = new cdk::sequence_node(LINE, $2, $1); }
      ;

stmt : expr ';'                                   { $$ = new mml::evaluation_node(LINE, $1); }
     | exprs tPRINT                               { $$ = new mml::print_node(LINE,$1, false); }
     | exprs tPRINTLN                             { $$ = new mml::print_node(LINE, $1, true); }
     | tSTOP ';'                                  { $$ = new mml::stop_node(LINE, 1); }
     | tSTOP tINTEGER ';'                         { $$ = new mml::stop_node(LINE, $2); }
     | tNEXT ';'                                  { $$ = new mml::next_node(LINE, 1); }
     | tNEXT tINTEGER  ';'                        { $$ = new mml::next_node(LINE, $2); }
     | tRETURN ';'                                { $$ = new mml::return_node(LINE,nullptr); }
     | tRETURN expr ';'                           { $$ = new mml::return_node(LINE, $2); }
     | block                                      { $$ = $1;}
     | cond_stmt                                  { $$ = $1;}
     | iter_expr                                  { $$ = $1;}
     ;

cond_stmt : tIF '(' expr ')' stmt          { $$ = new mml::if_node(LINE, $3, $5); }
          | tIF '(' expr ')' stmt elif     { $$ = new mml::if_else_node(LINE, $3, $5, $6); }
          ;

elif : tELSE  stmt                                 { $$ = $2; }
     | tELIF '(' expr ')' stmt  %prec tIF          { $$ = new mml::if_else_node(LINE, $3, $5,nullptr); }
     | tELIF '(' expr ')' stmt  elif               { $$ = new mml::if_else_node(LINE, $3, $5, $6); }
     ;

iter_expr : tWHILE '(' expr ')' stmt     { $$ = new mml::while_node(LINE, $3, $5); }  
          ;  

variable_type : tINTEGER_TYPE                           { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT); }
              | tDOUBLE_TYPE                            { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
              | tSTRING_TYPE                            { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
              | tVOID_TYPE                              { $$ = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_VOID)); }
              | tBRACKET variable_type tRBRACKET        { if ($2->name() == cdk::TYPE_POINTER && cdk::reference_type::cast($2)->referenced()->name() == cdk::TYPE_VOID) {$$ = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_VOID)); } else {$$ = cdk::reference_type::create(4, $2);     }   }
              | function_type                           { $$ = $1; }
              ;

variable_types : variable_types variable_type          { $$ = $1; $$->push_back($2); }
               | variable_type                         { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); $$->push_back($1); }
               ;

function_type : variable_type '<' variable_types '>'   {$$ = cdk::functional_type::create(*$3, cdk::primitive_type::create(4, cdk::TYPE_VOID));delete $3;}
              | variable_type '<' '>'                  {$$ = cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_VOID));}
              ;

exprs : expr                      { $$ = new cdk::sequence_node(LINE, $1); } 
      | exprs ',' expr            { $$ = new cdk::sequence_node(LINE, $3, $1); }
      ;

variables :  variables ',' variable          { $$ = new cdk::sequence_node(LINE, $3, $1); }
          |  variable                        { $$ = new cdk::sequence_node(LINE, $1); }

variable : variable_type tIDENTIFIER                   { $$ = new mml::variable_declaration_node(LINE, '\0', $1, *$2,nullptr); delete $2; }
         ;


function_definition : '(' ')' tARROW variable_type block                  { $$ = new mml::function_definition_node(LINE, nullptr, $4, $5); }
                    | '(' variables ')' tARROW variable_type block        { $$ = new mml::function_definition_node(LINE, $2, $5, $6); }
                    ;

other_args  : exprs   {$$ =$1;}
            |         {$$= new cdk::sequence_node(LINE);}
            ;

string : tSTRING                  { $$ = $1; }
       | string tSTRING           { $$ = $1; $$->append(*$2); delete $2; }
       ;

expr : tINTEGER                                   { $$ = new cdk::integer_node(LINE, $1); }
     | '~' expr	                              { $$ = new cdk::not_node(LINE, $2); }
     | tDOUBLE                                    { $$ = new cdk::double_node(LINE, $1); }
	| string                                     { $$ = new cdk::string_node(LINE, $1); }
     | tNULL                                      { $$ = new mml::null_node(LINE); }
     | tSIZEOF '(' expr ')'                       { $$ = new mml::sizeof_node(LINE, $3); }
     | '+' expr %prec tUNARY                      { $$ = new mml::identity_node(LINE, $2); }
     | '-' expr %prec tUNARY                      { $$ = new cdk::neg_node(LINE, $2); }
     | expr '+' expr	                         { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr '-' expr	                         { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr	                         { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr	                         { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr	                         { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr	                         { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr	                         { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr	                         { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr                              { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr	                         { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr	                         { $$ = new cdk::eq_node(LINE, $1, $3); }
     | expr tAND expr	                         { $$ = new cdk::and_node(LINE, $1, $3); }
     | expr tOR expr	                         { $$ = new cdk::or_node(LINE, $1, $3); }
     | '(' expr ')'                               { $$ = $2; }
     | lval '?'                                   { $$ = new mml::address_node(LINE, $1); }
     | lval                                       { $$ = new cdk::rvalue_node(LINE, $1); }  //FIXME
     | lval '=' expr                              { $$ = new cdk::assignment_node(LINE, $1, $3); }
     | tBRACKET expr tRBRACKET                    { $$ = new mml::alloc_node(LINE, $2); }
     | function_definition                        { $$ = $1; }
     | expr  '(' other_args ')'                   { $$ = new mml::function_call_node(LINE, $1, $3); }
     | '@' '(' other_args ')'                     { $$ = new mml::function_call_node(LINE, nullptr, $3); }    
     ;

lval : tIDENTIFIER                                { $$ = new cdk::variable_node(LINE, $1); }
     ;

%%
