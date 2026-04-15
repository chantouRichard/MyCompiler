/*practice1.l*/
%{
#include <stdio.h>
#include "y.tab.h"
void yyerror(char *);
%}
NUM [1-9]+[0-9]*|0
TYPE "int"|"float"|"char"|"string"|"double"
FLOAT_LIT   [0-9]+\.[0-9]+
CHAR_LIT    \'[^\']\'
STRING_LIT  \"[^\"]*\"
END ";"
%%

{TYPE}                      return TYPE;
{NUM}		                return NUM;
{FLOAT_LIT}                 return FLOAT_LIT;
{CHAR_LIT}                  return CHAR_LIT;
{STRING_LIT}                return STRING_LIT;
{END}                       return END;
[a-zA-Z_$]+[a-zA-Z_$0-9]*	return VAR;

"="                         return '=';   /* 新增：处理等号 */
\n                          return CR;
[ \t]+                     /* ignore whitespace */;
.
%%