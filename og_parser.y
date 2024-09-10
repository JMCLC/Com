%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <cdk/compiler.h>
#include "ast/all.h"
#define LINE               compiler->scanner()->lineno()
#define yylex()            compiler->scanner()->scan()
#define yyerror(s)         compiler->scanner()->error(s)
#define YYPARSE_PARAM_TYPE std::shared_ptr<cdk::compiler>
#define YYPARSE_PARAM      compiler
//-- don't change *any* of these --- END!
%}

%union {
  int                   i;  /* integer value */
  double                d;  /* double value */
  std::string          *s;  /* symbol name or string literal */
  cdk::basic_node      *node; /* node pointer */
  cdk::typed_node      *typenode;
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  cdk::basic_type      *types;
  og::block_node       *bloco;
};

%token <i> tINTEGER
%token <s> tIDENTIFIER tSTRING
%token <d> tREAL
%token tFOR tIF tDO tTHEN tELIF tWHILE
%token tINT_TYPE tREAL_TYPE tSTRING_TYPE tPTR_TYPE tAUTO
%token tWRITELN tWRITE tINPUT tOR  tAND tPOWER
%token tRETURN tCONTINUE tBREAK tSIZEOF
%token tPUBLIC tREQUIRE tNULLPTR tPROCEDURE 

%nonassoc tIF
%nonassoc tTHEN
%nonassoc tELSE 
%nonassoc tELIF

%right '='
%left tOR
%left tAND
%nonassoc '~'
%left  tEQ tNE 
%left tGE tLE '>' '<'
%left '+' '-'
%left '*' '/' '%'
%nonassoc '?' tUNARY
%nonassoc '[' ']'
%nonassoc ','
%nonassoc ';'


%type <node> instruction declaration elif 
%type <sequence> file exprs declarations instructions blockInstructions 
%type <sequence> blockDecs functionArgs variables exprsFor
%type <typenode> variable functionDec functionDef functionArg localVariable
%type <expression> expr
%type <types> type auto voidType
%type <lvalue> lval
%type <bloco> block
%type <s> string identifiers identifier
%type <i> tREQUIRE accessType

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file  : declarations { compiler->ast($$ = $1); }

declarations : declarations declaration { $$ = new cdk::sequence_node(LINE, $2, $1); }
             | declaration              { $$ = new cdk::sequence_node(LINE, $1); }
             ;


declaration : variable ';'    {$$ = $1; }
            | functionDec     {$$ = $1; }
            | functionDef     {$$ = $1; }
            ;

variables : variables ',' localVariable {$$ = new cdk::sequence_node(LINE, $3, $1); }
          | localVariable               {$$ = new cdk::sequence_node(LINE, $1); }
          ;

variable : accessType type identifier  {$$ = new og::variable_declaration_node(LINE,$1, $2, *$3, nullptr); delete $3; }  
         | accessType type identifier  '=' expr { $$ = new og::variable_declaration_node(LINE,$1, $2, *$3, $5); delete $3;}
         | accessType auto identifiers '=' exprs { $$ = new og::variable_declaration_node(LINE,$1, $2, *$3, new og::tuple_node(LINE,$5)); delete $3; }
         | tREQUIRE type identifier  {$$ = new og::variable_declaration_node(LINE,$1, $2, *$3, nullptr); delete $3; }  
         | tREQUIRE type identifier  '=' expr { $$ = new og::variable_declaration_node(LINE,$1, $2, *$3, $5); delete $3;}
         ;

type : tINT_TYPE               {$$ = new cdk::primitive_type(4,cdk::typename_type::TYPE_INT);}
     | tREAL_TYPE              {$$ = new cdk::primitive_type(8,cdk::typename_type::TYPE_DOUBLE);}
     | tSTRING_TYPE            {$$ = new cdk::primitive_type(4,cdk::typename_type::TYPE_STRING);}
     | tPTR_TYPE '<' type '>'  {$$ = new cdk::reference_type(4,std::shared_ptr<cdk::basic_type>($3));}
     | tPTR_TYPE '<' auto '>'  {$$ = new cdk::reference_type(4,std::shared_ptr<cdk::basic_type>($3));}
     ;

auto : tAUTO  { $$ = new cdk::primitive_type(4, cdk::typename_type::TYPE_STRUCT); }
     ;

voidType : tPROCEDURE { $$ = new cdk::primitive_type(0, cdk::typename_type::TYPE_VOID); }
         ;
         
accessType : tPUBLIC {$$ = tPUBLIC;}
           |         {$$ = '\0';}
           ;

identifier : tIDENTIFIER  {$$ = $1; }
           ;

identifiers : identifiers ',' identifier { $$ = new std::string(*$1 + "," + *$3); delete $1; delete $3;}
            | identifier                 { $$ = $1; }
            ;


functionArgs: functionArgs ',' functionArg   { $$ = new cdk::sequence_node(LINE, $3, $1); }
            | functionArg                    { $$ = new cdk::sequence_node(LINE,$1);}
            |                                { $$ = nullptr;}
            ;

functionArg  : type identifier { $$ = new og::variable_declaration_node(LINE, '\0', $1, *$2, nullptr); delete $2;}


functionDec : accessType type identifier '(' functionArgs ')'        {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, nullptr); delete $3;}
            | accessType auto identifier '(' functionArgs ')'        {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, nullptr); delete $3; }
            | accessType voidType identifier '(' functionArgs ')'   {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, nullptr); delete $3; }
            | tREQUIRE type identifier '(' functionArgs ')'        {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, nullptr); delete $3; }
            | tREQUIRE auto identifier '(' functionArgs ')'        {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, nullptr); delete $3; }
            | tREQUIRE voidType identifier '(' functionArgs ')'   {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, nullptr); delete $3; }
            ;

functionDef : accessType type identifier '(' functionArgs')' block       {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, $7); delete $3; }
            | accessType auto identifier '(' functionArgs')' block       {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, $7); delete $3; }
            | accessType voidType identifier '(' functionArgs ')' block   {$$ = new og::function_definition_node(LINE,$1, $2, *$3, $5, $7); delete $3; }
            ;

localVariable  : type identifier  { $$ = new og::variable_declaration_node(LINE,'\0', $1, *$2, nullptr); delete $2; }  
               | type identifier  '=' expr { $$ = new og::variable_declaration_node(LINE,'\0', $1, *$2, $4); delete $2;}
               | auto identifiers '=' exprs { $$ = new og::variable_declaration_node(LINE,'\0', $1, *$2, new og::tuple_node(LINE,$4)); delete $2; }
               ;


blockDecs : blockDecs localVariable ';'      { $$ = new cdk::sequence_node(LINE, $2, $1); }
          | localVariable  ';'               { $$ = new cdk::sequence_node(LINE, $1); }
          ;

block : '{' blockDecs blockInstructions '}'  {$$ = new og::block_node(LINE, $2, $3);}
      | '{' blockInstructions '}'            {$$ = new og::block_node(LINE, nullptr, $2);}
      ;

blockInstructions : instructions       {$$ = $1; }
                  |                    {$$ = nullptr; }
                  ;

instructions : instructions instruction   { $$ = new cdk::sequence_node(LINE, $2, $1); }
             | instruction                { $$ = new cdk::sequence_node(LINE, $1); }
             ;


instruction : expr ';'                                                 { $$ = new og::evaluation_node(LINE, $1); }
            | tWRITE exprs ';'                                         { $$ = new og::write_node(LINE, $2, false); }
            | tWRITELN exprs ';'                                       { $$ = new og::write_node(LINE, $2, true); }
            | tBREAK                                                   { $$ = new og::break_node(LINE); }
            | tCONTINUE                                                { $$ = new og::continue_node(LINE); }    
            | tRETURN exprsFor ';'                                     { $$ = new og::return_node(LINE, new og::tuple_node(LINE, $2)); }    
            | tFOR exprsFor ';' exprsFor ';' exprsFor tDO instruction  { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
            | tFOR variables ';' exprsFor ';' exprsFor tDO instruction { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
            | tDO block tWHILE expr ';'                                { $$ = new og::do_while_node(LINE, $4, $2); } 
            | tWHILE '(' expr ')' block                                { $$ = new og::while_node(LINE, $3, $5); }
            | tIF expr tTHEN instruction                               { $$ = new og::if_node(LINE, $2, $4); }
            | tIF expr tTHEN instruction elif                          { $$ = new og::if_else_node(LINE, $2, $4, $5); }
            | block                                                    { $$ = $1; }
            ;  

elif        : tELSE instruction                   { $$ = $2; }
            | tELIF expr tTHEN instruction        { $$ = new og::if_node(LINE, $2, $4); }
            | tELIF expr tTHEN instruction elif   { $$ = new og::if_else_node(LINE, $2, $4, $5); }
            ;

exprsFor : exprs  {$$ = $1; }
         |        {$$ = nullptr; }
         ;

exprs : exprs ',' expr { $$ = new cdk::sequence_node(LINE, $3, $1); }
      | expr           { $$ = new cdk::sequence_node(LINE, $1); } 
      ;

string : tSTRING          {$$ = $1;}
       | string tSTRING   {$$ = new std::string(*$1 + *$2); delete $1; delete $2;}
       ;



expr : tINTEGER                         { $$ = new cdk::integer_node(LINE, $1); }
     | tREAL                            { $$ = new cdk::double_node(LINE, $1); }
     | string                           { $$ = new cdk::string_node(LINE, $1); }
     | tNULLPTR                         { $$ = new og::nullptr_node(LINE); }
     | '+' expr %prec tUNARY            { $$ = new og::identity_node(LINE, $2); }
     | '-' expr %prec tUNARY            { $$ = new cdk::neg_node(LINE, $2); }
     | expr '+' expr                    { $$ = new cdk::add_node(LINE, $1, $3); }
     | expr tPOWER expr                 { $$ = new og::power_node(LINE, $1, $3); }
     | expr '-' expr                    { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr                    { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr                    { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr                    { $$ = new cdk::mod_node(LINE, $1, $3); }
     | expr '<' expr                    { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr '>' expr                    { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr                    { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tLE expr                    { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tNE expr                    { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tEQ expr                    { $$ = new cdk::eq_node(LINE, $1, $3); }
     | expr tOR expr                    { $$ = new cdk::or_node(LINE, $1, $3); }
     | expr tAND expr                   { $$ = new cdk::and_node(LINE, $1, $3); }
     | '(' expr ')'                     { $$ = $2; }
     | tINPUT                           { $$ = new og::input_node(LINE); }
     | lval '?'                         { $$ = new og::address_of_node(LINE, $1);}
     | lval                             { $$ = new cdk::rvalue_node(LINE, $1); }  
     | lval '=' expr                    { $$ = new cdk::assignment_node(LINE, $1, $3); }
     | '~' expr                         { $$ = new cdk::not_node(LINE, $2); }
     | tSIZEOF '(' exprs ')'            { $$ = new og::sizeof_node(LINE, new og::tuple_node(LINE, $3)); }
     | identifier '(' exprsFor ')'      { $$ = new og::function_call_node(LINE, *$1 , $3); delete $1; }
     | '[' expr ']'                     { $$ = new og::stack_alloc_node(LINE, $2); }
     ;

lval : identifier             { $$ = new cdk::variable_node(LINE, $1); }
     | lval '@' tINTEGER      { $$ = new og::tuple_index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
     | lval '[' expr ']'      { $$ = new og::left_index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }  
     | '(' expr ')' '[' expr  ']' { $$ = new og::left_index_node(LINE, $2, $5); }  
     | identifier '(' exprsFor ')' '@' tINTEGER   { $$ = new og::tuple_index_node(LINE, new og::function_call_node(LINE, *$1, $3), $6); delete $1; } 
     | identifier '(' exprsFor ')' '[' expr ']'  { $$ = new og::left_index_node(LINE, new og::function_call_node(LINE, *$1, $3), $6); delete $1; }  
     ;

%%
