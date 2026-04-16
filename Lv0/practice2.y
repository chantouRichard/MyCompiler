%{

#include <stdio.h>
#include <string.h>
int yylex(void);
void yyerror(char *);
    
%}
%union {
    int inum;
    double dnum;
}
%token ADD SUB MUL DIV CR LP RP
%token <inum> NUM
%token <dnum> DNUM
%type <inum> expression term factor single
%type <dnum> d_expression d_term d_factor d_single

%%

line_list: line_list line
         | line
         | line_list dline
         | dline
         ;
line: expression CR {printf("Result: %d\n", $1);}

expression: term
          | expression ADD term {$$ = $1 + $3;}
          | expression SUB term {$$ = $1 - $3;}
          ;

term:factor
    |term MUL factor {$$ = $1 * $3;}
    |term DIV factor {$$ = $1 / $3;}
    ;

factor:single
      |LP expression RP {$$ = $2;}
      ;
single:NUM;

dline: d_expression CR {printf("Double Result: %lf\n", $1);}

d_expression:d_term 
            |d_expression ADD d_term {$$ = $1 + $3;}
            |d_expression SUB d_term {$$ = $1 - $3;}
            ;
d_term:d_factor
      |d_single MUL d_factor {$$ = $1 * $3;}
      |d_single DIV d_factor {$$ = $1 / $3;}
      ;

d_factor:d_single
        |LP d_expression RP {$$ = $2;}
d_single:DNUM;
%%

void yyerror(char *str){
    fprintf(stderr, "error:%s\n", str);
}

int yywrap(){
    return 1;
}

int main(){
    yyparse();
}