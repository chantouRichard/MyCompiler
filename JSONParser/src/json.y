%code requires {
  #include <iostream>
  #include <memory>
  #include <map>
  #include <string>
  #include <vector>
  #include "json_ast.h"
}

%{

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include "json_ast.h"

int yylex();
void yyerror(std::unique_ptr<JsonNode>& ast, const char* s);

%}

%parse-param { std::unique_ptr<JsonNode>& ast }

%union {
  int int_val;
  std::string* str_val;
  JsonNode* node;
  std::map<std::string, JsonNode>* obj;
  std::vector<JsonNode>* arr;
}

%token <str_val>ID
%token <int_val> NUM
%token '{' '}' ':' ',' '[' ']' TRUE FALSE TOKEN_NULL

%type <node> Value Object Array
%type <obj> Members
%type <arr> Elements

%%

Json : Object {
    ast = std::unique_ptr<JsonNode>($1);
  }

Object : '{' '}' {
    $$ = new JsonNode(std::map<std::string, JsonNode>());
  }
      | '{' Members '}' {
          $$ = new JsonNode(*$2);
          delete $2;
      }
      ;

Members : ID ':' Value {
           $$ = new std::map<std::string, JsonNode>();
           (*$$)[*$1] = *$3;
           delete $1;
           delete $3;
         }
        | Members ',' ID ':' Value {
           (*($1))[*$3] = *$5;
           delete $3;
           delete $5;
           $$ = $1;
         }
        ;

Array : '[' ']' {
    $$ = new JsonNode(std::vector<JsonNode>());
  }
      | '[' Elements ']' {
      $$ = new JsonNode(*$2);
      delete $2;
  }
  ;

Elements : Value {
    $$ = new std::vector<JsonNode>();
    $$->push_back(*$1);
    delete $1;
  }
  | Elements ',' Value {
    $1->push_back(*$3);
    delete $3;
    $$ = $1;
  }
  ;

Value : Object { $$ = $1; }
      | NUM { $$ = new JsonNode($1); }
      | ID { $$ = new JsonNode(*$1); }
      | Array { $$ = $1; }
      | TRUE { $$ = new JsonNode(true); }
      | FALSE { $$ = new JsonNode(false); }
      | TOKEN_NULL { $$ = new JsonNode(); }
      ;

%%

void yyerror(std::unique_ptr<JsonNode>& ast, const char* s) {
  std::cerr << "error: " << s << std::endl;
}