%{
#include <stdio.h>
#include <string.h>
int yylex(void);
void yyerror(char *);

%}

%token NUM VAR CR TYPE END
%token FLOAT_LIT CHAR_LIT STRING_LIT DOUBLE_LIT

%%
line_list : line
          | line_list line
          ;
line : definition END CR {printf("YES\n");}
     ;

definition : TYPE VAR
           | TYPE VAR '=' NUM
           | TYPE VAR '=' FLOAT_LIT
           | TYPE VAR '=' CHAR_LIT
           | TYPE VAR '=' STRING_LIT
           | TYPE VAR '=' DOUBLE_LIT
           ;


%%

void yyerror(char *str){
    fprintf(stderr, "error: %s\n", str);
}

int yywrap() {
    return 1;
}

int main() {
    yyparse();
}