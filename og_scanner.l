%option c++ prefix="og_scanner_" outfile="og_scanner.cpp"
%option stack noyywrap yylineno 8bit
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "og_parser.tab.h"

static std::ostringstream strlit;
// don't change this
#define yyerror LexerError
%}
%x X_STRING X_COMMENT X_SPECIALCHAR
%%

 /* ====================================================================== */
 /* ====[                        OPERADORES                          ]==== */
 /* ====================================================================== */

"sizeof"			                return tSIZEOF;

">="                          return tGE; 
"<="                          return tLE;

"=="                          return tEQ; 
"!="                          return tNE;


"&&"                          return tAND;
"||"                          return tOR;
"**"                          return tPOWER;


  /* ====================================================================== */
  /* ====[                       READ & WRITE	                      ]==== */
  /* ====================================================================== */

"input"                       return tINPUT; 
"write"				                return tWRITE;
"writeln"			                return tWRITELN;


  /* ====================================================================== */
  /* ====[                        KEYWORDS	                          ]==== */
  /* ====================================================================== */

"auto" 				                return tAUTO;
"int"                         return tINT_TYPE;
"real"				                return tREAL_TYPE;
"string"			                return tSTRING_TYPE;
"ptr"				                  return tPTR_TYPE;
"nullptr"			                return tNULLPTR;
"procedure"			              return tPROCEDURE;


  /* ====================================================================== */
  /* ====[                        CICLOS	                            ]==== */
  /* ====================================================================== */

"for"                         return tFOR; 
"do"                          return tDO;
"while"                       return tWHILE;


  /* ====================================================================== */
  /* ====[                        CONDIÇÃO	                          ]==== */
  /* ====================================================================== */

"if"                          return tIF; 
"else"                        return tELSE; 
"elif"                        return tELIF;
"then"				                return tTHEN;


  /* ====================================================================== */
  /* ====[                      SKIP OPERATORS	                      ]==== */
  /* ====================================================================== */

"break"			   	          return tBREAK;
"continue"			          return tCONTINUE;
"return"			            return tRETURN;


 /* ====================================================================== */
 /* ====[                         SIMBOLOS                           ]==== */
 /* ====================================================================== */

"public"                      return tPUBLIC;
"require"                     return tREQUIRE;

 /* ====================================================================== */
 /* ====[                       IDENTIFICADORES                      ]==== */
 /* ====================================================================== */

[A-Za-z][A-Za-z0-9_]*         yylval.s = new std::string(yytext); return tIDENTIFIER;

  /* ====================================================================== */
  /* ====[                        COMENTARIOS                         ]==== */
  /* ====================================================================== */

"//".*\n                      ; /* ignore comments */

"/*"                          yy_push_state(X_COMMENT);
<X_COMMENT>"/*"               yy_push_state(X_COMMENT);
<X_COMMENT>"*/"               yy_pop_state();
<X_COMMENT>.|\n               ; /* ignore content */

  /* ====================================================================== */
  /* ====[                      	STRINGS                           ]==== */
  /* ====================================================================== */

"\""                          yy_push_state(X_STRING);
<X_STRING>"\\"                yy_push_state(X_SPECIALCHAR);
<X_STRING>"\""                {
                                yylval.s = new std::string(strlit.str());
                                strlit.str("");
                                yy_pop_state();
                                return tSTRING;
                              }
<X_STRING>\n                  yyerror("Error in string");
<X_STRING>.                   strlit << *yytext;

<X_SPECIALCHAR>n              { strlit << '\n'; yy_pop_state(); }
<X_SPECIALCHAR>r              { strlit << '\r'; yy_pop_state(); }
<X_SPECIALCHAR>t              { strlit << '\t'; yy_pop_state(); }
<X_SPECIALCHAR>"\""           { strlit << '"'; yy_pop_state(); }
<X_SPECIALCHAR>"\\"           { strlit << "\\"; yy_pop_state(); }
<X_SPECIALCHAR>[0-9A-Fa-f]{1,2} {
                                 strlit << (char)(unsigned char)std::stoul(yytext, NULL, 16);
                                 yy_pop_state();
                               }
<X_SPECIALCHAR>0[g-zG-Z0][A-Za-z0-9]* {
                                 strlit << '\0';
                                 yy_pop_state();
                               }
<X_SPECIALCHAR>.			         { strlit << *yytext; yy_pop_state(); }
  
  /* ====================================================================== */
  /* ====[                         INTEIROS                           ]==== */
  /* ====================================================================== */

[0-9]+                        {  try { 
                                  yylval.i = std::stoi(yytext, nullptr, 10);
                                  return tINTEGER;
                                } catch (std::out_of_range& e) { yyerror("Integer out of range.");}
                              }
0[xX][A-Fa-f0-9]+	          { try { 
                                  yylval.i = std::stoi(yytext, nullptr, 16);
                                  return tINTEGER;
                                } catch (std::out_of_range& e) { yyerror("Integer out of range."); }
                              }

0[xX][0-9a-zA-Z]+           yyerror("Invalid base 16 number.");

 /* ====================================================================== */
 /* ====[                           REAIS                            ]==== */
 /* ====================================================================== */

[0-9]+"."[0-9]+  { try { yylval.d = std::stod(yytext, NULL); 
                         return tREAL; 
                       } catch (std::out_of_range& e) { yyerror("Real out of range."); }
                 }

[0-9]+"."        { try { yylval.d = std::stod(yytext, NULL); 
                         return tREAL; 
                       } catch (std::out_of_range& e) { yyerror("Real out of range."); }
                 }

"."[0-9]+        { try { yylval.d = std::stod(yytext, NULL); 
                         return tREAL; 
                       } catch (std::out_of_range& e) { yyerror("Real out of range."); }
                 }

[0-9]+("."[0-9]+)?[Ee][-+]?[0-9]+  { try { yylval.d = std::stod(yytext, NULL); 
                                           return tREAL; 
                                         } catch (std::out_of_range& e) { yyerror("Real out of range."); }
                                   }

 /* ====================================================================== */
 /* ====[                        OUTROS CHARS                        ]==== */
 /* ====================================================================== */

[-(){}\[\]<>=+*/%;,@~?\^]              return *yytext;

[ \t\n\r]+                     ; /* ignore whitespace */
.                            yyerror("Unknown character");

%%

