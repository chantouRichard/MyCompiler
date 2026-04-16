%{
#include <stdio.h>
#include "y.tab.h"
void yyerror(char *);
%}

NUM [1-9]+[0-9]*|0
DNUM [1-9]+[0-9]*\.[0-9]+|0\.[0-9]+

%%

{NUM} sscanf(yytext, "%d", &yylval.inum); return NUM;
{DNUM} sscanf(yytext, "%lf", &yylval.dnum); return DNUM;

"+" return ADD;
"-" return SUB;
"*" return MUL;
"/" return DIV;

"(" return LP;
")" return RP;

\n return CR;
[ \t]+ /* ignore whitespace */;
. /* ignore other characters */

%%