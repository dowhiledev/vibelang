%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/ast.h"     // Fix: use correct path to ast.h
#include "parser.tab.h"       // This will be generated by Bison

// Define YYLTYPE structure if not defined in parser.tab.h
#ifndef YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
#define YYLTYPE_IS_DECLARED 1
#endif

// Track line numbers for error reporting
int line_num = 1;
int col_num = 1;

// Define YYLTYPE and YYLVAL to be compatible with Bison
extern YYLTYPE yylloc;
extern YYSTYPE yylval;

// Helper function to update location
void update_loc() {
    yylloc.first_line = yylloc.last_line = line_num;
    yylloc.first_column = col_num;
    col_num += yyleng;
    yylloc.last_column = col_num - 1;
}

// Helper function for string literals
char* process_string_literal(char* text) {
    // Remove quotes and handle escapes
    int len = strlen(text) - 2; // exclude quotes
    char* str = malloc(len + 1);
    strncpy(str, text + 1, len);
    str[len] = '\0';
    return str;
}
%}

%option noyywrap
%option yylineno

%%
[ \t]+          { col_num += yyleng; } /* Ignore whitespace */
\n              { line_num++; col_num = 1; } /* Track newlines */

"//"[^\n]*      { /* Ignore comments */ }

"fn"            { update_loc(); return FN; }
"type"          { update_loc(); return TYPE; }
"class"         { update_loc(); return CLASS; }
"import"        { update_loc(); return IMPORT; }
"let"           { update_loc(); return LET; }
"return"        { update_loc(); return RETURN; }
"prompt"        { update_loc(); return PROMPT; }
"Meaning"       { update_loc(); return MEANING; }

"true"          { update_loc(); yylval.bool_val = 1; return BOOL_LIT; }
"false"         { update_loc(); yylval.bool_val = 0; return BOOL_LIT; }

"("             { update_loc(); return '('; }
")"             { update_loc(); return ')'; }
"{"             { update_loc(); return '{'; }
"}"             { update_loc(); return '}'; }
"<"             { update_loc(); return '<'; }
">"             { update_loc(); return '>'; }
";"             { update_loc(); return ';'; }
":"             { update_loc(); return ':'; }
"="             { update_loc(); return '='; }
","             { update_loc(); return ','; }
"->"            { update_loc(); return ARROW; }

[0-9]+\.[0-9]+  { 
    update_loc(); 
    yylval.float_val = atof(yytext); 
    return FLOAT_LIT; 
}

[0-9]+          { 
    update_loc(); 
    yylval.int_val = strtoll(yytext, NULL, 10); 
    return INT_LIT; 
}

\"[^\"]*\"      { 
    update_loc(); 
    yylval.str_val = process_string_literal(yytext); 
    return STRING_LIT; 
}

[a-zA-Z_][a-zA-Z0-9_]*  { 
    update_loc(); 
    yylval.str_val = strdup(yytext); 
    return IDENTIFIER; 
}

.               { update_loc(); return yytext[0]; } /* Catch any other character */

%%

// Reset lexer state between parses
void yylex_reset() {
    line_num = 1;
    col_num = 1;
    // No need to call yylex_destroy in non-reentrant mode
}
