%{
#include "json.tab.hpp"
%}

%option noyywrap

%%

"{"             { return '{'; }
"}"             { return '}'; }
":"             { return ':'; }
","             { return ','; }
"["             { return '['; }
"]"             { return ']'; }

"true"          { 
    return TRUE; 
}
"false"         { 
    return FALSE; 
}
"null"          {
    return TOKEN_NULL;
}
\"(\\.|[^"\\])*\"  {
    std::string* s = new std::string();
    // 去掉引号，处理转义
    for (int i = 1; i < yyleng - 1; i++) {
        if (yytext[i] == '\\' && i + 1 < yyleng - 1) {
            i++;
            switch (yytext[i]) {
                case '"': s->push_back('"'); break;
                case '\\': s->push_back('\\'); break;
                case '/': s->push_back('/'); break;
                case 'n': s->push_back('\n'); break;
                case 'r': s->push_back('\r'); break;
                case 't': s->push_back('\t'); break;
                case 'b': s->push_back('\b'); break;
                default: s->push_back(yytext[i]); break;
            }
        } else {
            s->push_back(yytext[i]);
        }
    }
    yylval.str_val = s;
    return ID;
}
\"[^\"]*\"      {  // 匹配双引号内的任意非引号字符
    std::string* s = new std::string(yytext + 1, yyleng - 2);
    yylval.str_val = s;
    return ID;  // 注意：返回 STRING，不是 ID
}

[-]?[0-9]+([.][0-9]+)?([eE][+-]?[0-9]+)?  {
    yylval.int_val = atof(yytext);
    return NUM;
}
[0-9]|[1-9][0-9]+       { yylval.int_val = atoi(yytext); return NUM; }

[ \t\n]+        { /* 忽略空白 */ }

%%