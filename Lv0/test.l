%{
#include <stdio.h>
%}

%%

([1-9]+[0-9]*)|[0]	{printf("NUM ");};
[0-9]+\.[0-9]+  {printf("FLOAT ");};
[+\-*/]	{printf("SIGN ");};
\{	{printf("LBRACE ");};
\}	{printf("RBRACE ");};
. {}

%%

int main(int argc, char **argv)
{
  yylex();
  yywrap();
}
int yywrap()
{
	return 1;
}