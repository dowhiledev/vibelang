%{
#include <stdio.h>
#include <stdlib.h>
#include "../utils/ast.h"      // Fix: use correct path to ast.h
#include "../utils/log_utils.h" // Fix: use correct path to log_utils.h

// Define YYLTYPE structure before using it
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
#define YYLTYPE_IS_DECLARED 1

// Forward declarations
extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern int yylineno;
extern void yylex_reset();

// Flex buffer state typedef - needed for yy_scan_string
typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char* str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

// Location tracking
YYLTYPE yylloc;

// Error handling
void yyerror(const char *s);

// Parser result
ast_node_t* parse_result = NULL;

// Forward declaration - actual definition in parser_utils.c
extern ast_node_t* parse_string(const char* source);
%}

// Enable detailed error messages for Bison 2.3
%error-verbose

// Track locations for better error reporting
%locations

// Declare value types
%union {
    long long int_val;
    double float_val;
    int bool_val;
    char *str_val;
    ast_node_t *ast;
}

// Define tokens
%token FN TYPE CLASS IMPORT LET RETURN PROMPT MEANING ARROW
%token <int_val> INT_LIT
%token <float_val> FLOAT_LIT
%token <bool_val> BOOL_LIT
%token <str_val> STRING_LIT IDENTIFIER

// Define types for non-terminals
%type <ast> program declarations declaration_rest declaration
%type <ast> function_declaration return_type parameter_list parameter_rest parameter
%type <ast> type_declaration type basic_type meaning_type
%type <ast> class_declaration class_members class_member member_variable import_declaration
%type <ast> block statement_list statement variable_declaration type_annotation
%type <ast> return_statement prompt_statement expression_statement
%type <ast> expression call_expression argument_list argument_rest
%type <ast> literal

// Define precedence and associativity
%left '+' '-'
%left '*' '/'

%%

program
    : declarations 
        { 
            parse_result = $1;
            $$ = $1; 
        }
    ;

declarations
    : declaration 
        {
            ast_node_t* program = create_ast_node(AST_PROGRAM);
            if ($1) ast_add_child(program, $1);
            $$ = program;
        }
    | declarations declaration_rest 
        {
            if ($2) ast_add_child($1, $2);
            $$ = $1;
        }
    ;

declaration_rest
    : declaration
        {
            $$ = $1;
        }
    ;

declaration
    : function_declaration { $$ = $1; }
    | type_declaration { $$ = $1; }
    | class_declaration { $$ = $1; }
    | import_declaration { $$ = $1; }
    ;

function_declaration
    : FN IDENTIFIER '(' parameter_list ')' return_type block 
        {
            ast_node_t* func = create_ast_node(AST_FUNCTION_DECL);
            ast_set_string(func, "name", $2);
            if ($4) ast_add_child(func, $4);
            if ($6) ast_add_child(func, $6);
            if ($7) {
                ast_node_t* func_body = create_ast_node(AST_FUNCTION_BODY);
                for (int i = 0; i < $7->child_count; i++) {
                    ast_add_child(func_body, $7->children[i]);
                }
                ast_add_child(func, func_body);
            }
            $$ = func;
            free($2);  // Free the identifier string
        }
    | FN IDENTIFIER '(' ')' return_type block 
        {
            ast_node_t* func = create_ast_node(AST_FUNCTION_DECL);
            ast_set_string(func, "name", $2);
            if ($5) ast_add_child(func, $5);
            if ($6) {
                ast_node_t* func_body = create_ast_node(AST_FUNCTION_BODY);
                for (int i = 0; i < $6->child_count; i++) {
                    ast_add_child(func_body, $6->children[i]);
                }
                ast_add_child(func, func_body);
            }
            $$ = func;
            free($2);
        }
    ;

return_type
    : ARROW type { $$ = $2; }
    | /* empty */ { $$ = NULL; }
    ;

parameter_list
    : parameter { 
        ast_node_t* params = create_ast_node(AST_PARAM_LIST);
        ast_add_child(params, $1);
        $$ = params;
    }
    | parameter_list ',' parameter_rest {
        ast_add_child($1, $3);
        $$ = $1;
    }
    ;

parameter_rest
    : parameter
        {
            $$ = $1;
        }
    ;

parameter
    : IDENTIFIER ':' type {
        ast_node_t* param = create_ast_node(AST_PARAMETER);
        ast_set_string(param, "name", $1);
        ast_add_child(param, $3);
        $$ = param;
        free($1);
    }
    ;

type_declaration
    : TYPE IDENTIFIER '=' type ';' {
        ast_node_t* type_decl = create_ast_node(AST_TYPE_DECL);
        ast_set_string(type_decl, "name", $2);
        ast_add_child(type_decl, $4);
        $$ = type_decl;
        free($2);
    }
    ;

type
    : meaning_type { $$ = $1; }
    | basic_type { $$ = $1; }
    ;

basic_type
    : IDENTIFIER {
        ast_node_t* type = create_ast_node(AST_BASIC_TYPE);
        ast_set_string(type, "type", $1);
        $$ = type;
        free($1);
    }
    ;

meaning_type
    : MEANING '<' type '>' '(' STRING_LIT ')' {
        ast_node_t* type = create_ast_node(AST_MEANING_TYPE);
        ast_set_string(type, "meaning", $6);
        ast_add_child(type, $3);
        $$ = type;
        free($6);
    }
    ;

class_declaration
    : CLASS IDENTIFIER '{' '}' {
        ast_node_t* class = create_ast_node(AST_CLASS_DECL);
        ast_set_string(class, "name", $2);
        $$ = class;
        free($2);
    }
    | CLASS IDENTIFIER '{' class_members '}' {
        ast_node_t* class = create_ast_node(AST_CLASS_DECL);
        ast_set_string(class, "name", $2);
        ast_add_child(class, $4);
        $$ = class;
        free($2);
    }
    ;

class_members
    : class_member {
        ast_node_t* members = create_ast_node(AST_CLASS_BODY);
        ast_add_child(members, $1);
        $$ = members;
    }
    | class_members class_member {
        ast_add_child($1, $2);
        $$ = $1;
    }
    ;

class_member
    : member_variable
    | function_declaration
    ;

member_variable
    : IDENTIFIER ':' type ';' {
        ast_node_t* var = create_ast_node(AST_MEMBER_VAR);
        ast_set_string(var, "name", $1);
        ast_add_child(var, $3);
        $$ = var;
        free($1);
    }
    ;

import_declaration
    : IMPORT STRING_LIT ';' {
        ast_node_t* import = create_ast_node(AST_IMPORT);
        ast_set_string(import, "path", $2);
        $$ = import;
        free($2);
    }
    ;

block
    : '{' '}' {
        ast_node_t* block = create_ast_node(AST_BLOCK);
        $$ = block;
    }
    | '{' statement_list '}' {
        ast_node_t* block = create_ast_node(AST_BLOCK);
        ast_add_child(block, $2);
        $$ = block;
    }
    ;

statement_list
    : statement {
        ast_node_t* stmts = create_ast_node(AST_BLOCK);
        if ($1) ast_add_child(stmts, $1);
        $$ = stmts;
    }
    | statement_list statement {
        if ($2) ast_add_child($1, $2);
        $$ = $1;
    }
    ;

statement
    : variable_declaration
    | return_statement
    | prompt_statement
    | expression_statement
    | block
    | error ';' { 
        WARN("Skipping statement due to syntax error"); 
        $$ = NULL; 
    }
    ;

variable_declaration
    : LET IDENTIFIER type_annotation '=' expression ';' {
        ast_node_t* var = create_ast_node(AST_VAR_DECL);
        ast_set_string(var, "name", $2);
        if ($3) ast_add_child(var, $3);
        ast_add_child(var, $5);
        $$ = var;
        free($2);
    }
    ;

type_annotation
    : ':' type { $$ = $2; }
    | /* empty */ { $$ = NULL; }
    ;

return_statement
    : RETURN expression ';' {
        ast_node_t* ret = create_ast_node(AST_RETURN_STMT);
        ast_add_child(ret, $2);
        $$ = ret;
    }
    | RETURN ';' {
        ast_node_t* ret = create_ast_node(AST_RETURN_STMT);
        $$ = ret;
    }
    ;

prompt_statement
    : PROMPT STRING_LIT ';' {
        ast_node_t* prompt = create_ast_node(AST_PROMPT_BLOCK);
        ast_set_string(prompt, "template", $2);
        $$ = prompt;
        free($2);
    }
    ;

expression_statement
    : expression ';' {
        ast_node_t* stmt = create_ast_node(AST_EXPR_STMT);
        ast_add_child(stmt, $1);
        $$ = stmt;
    }
    ;

expression
    : call_expression { $$ = $1; }
    | literal { $$ = $1; }
    | IDENTIFIER {
        ast_node_t* node = create_ast_node(AST_IDENTIFIER);
        ast_set_string(node, "name", $1);
        $$ = node;
        free($1);
    }
    ;

call_expression
    : IDENTIFIER '(' argument_list ')' {
        ast_node_t* call = create_ast_node(AST_CALL_EXPR);
        ast_set_string(call, "function", $1);
        ast_add_child(call, $3);
        $$ = call;
        free($1);
    }
    | IDENTIFIER '(' ')' {
        ast_node_t* call = create_ast_node(AST_CALL_EXPR);
        ast_set_string(call, "function", $1);
        $$ = call;
        free($1);
    }
    ;

argument_list
    : expression {
        ast_node_t* args = create_ast_node(AST_PARAM_LIST);
        ast_add_child(args, $1);
        $$ = args;
    }
    | argument_list ',' argument_rest {
        ast_add_child($1, $3);
        $$ = $1;
    }
    ;

argument_rest
    : expression
        {
            $$ = $1;
        }
    ;

literal
    : STRING_LIT {
        ast_node_t* node = create_ast_node(AST_STRING_LITERAL);
        ast_set_string(node, "value", $1);
        $$ = node;
        free($1);
    }
    | INT_LIT {
        ast_node_t* node = create_ast_node(AST_INT_LITERAL);
        ast_set_int(node, "value", $1);
        $$ = node;
    }
    | FLOAT_LIT {
        ast_node_t* node = create_ast_node(AST_FLOAT_LITERAL);
        ast_set_float(node, "value", $1);
        $$ = node;
    }
    | BOOL_LIT {
        ast_node_t* node = create_ast_node(AST_BOOL_LITERAL);
        ast_set_bool(node, "value", $1);
        $$ = node;
    }
    ;

%%

void yyerror(const char *s) {
    ERROR("Parser error at line %d: %s", yylineno, s);
}

// External interface function to parse a string
ast_node_t* parse_string(const char* source) {
    if (!source) {
        ERROR("NULL source provided to parse_string");
        return NULL;
    }
    
    // Reset parser state
    yylex_reset();
    parse_result = NULL;
    
    // Create an in-memory buffer for the lexer
    YY_BUFFER_STATE buffer = yy_scan_string(source);
    
    // Parse the input
    int result = yyparse();
    
    // Clean up
    yy_delete_buffer(buffer);
    
    if (result != 0) {
        ERROR("Parsing failed with code %d", result);
        return NULL;
    }
    
    return parse_result;
}
