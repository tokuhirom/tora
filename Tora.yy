%{
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#define YYDEBUG 1

extern int yylex();
int yyerror(const char *err);

typedef enum {
    OP_NOP = 0,
    OP_ADD,
    OP_SUB,
    OP_DIV,
    OP_MUL,
    OP_PRINT,
    OP_PUSH_INT
} OP_TYPE;

typedef struct {
    int op_type;
    union {
        int operand_int;
    } operand;
} OP;

typedef union {
    int int_value;
} Value;

class VM {
public:
    std::vector<Value*> stack;
    int sp; // stack pointer
    int pc; // program counter
    std::vector<OP*> ops;

    VM() {
        sp = 0;
        pc = 0;
    }

    // run program
    void execute() {
        printf("[DEBUG] OPS: %d, %d\n", pc, ops.size());
        while (pc < ops.size()) {
            OP *op = ops[pc];
            switch (op->op_type) {
            case OP_PUSH_INT: {
                Value *v = new Value;
                v->int_value = op->operand.operand_int;
                stack.push_back(v);
                break;
            }
            case OP_ADD: {
                Value *v1 = stack.back();
                Value *v2 = stack.back();
                Value *v = new Value;
                v->int_value = v1->int_value + v2->int_value;
                stack.push_back(v);
                break;
            }
            case OP_SUB: {
                Value *v1 = stack.back();
                Value *v2 = stack.back();
                Value *v = new Value;
                v->int_value = v1->int_value - v2->int_value;
                stack.push_back(v);
                break;
            }
            case OP_PRINT: {
                Value *v1 = stack.back();
                printf(">> %d\n", v1->int_value);
                break;
            }
            default: {
                fprintf(stderr, "OOPS. unknown op code\n");
                break;
            }
            }
            pc++;
        }
    }
};

VM vm;

%}
%token IF
%union {
    int int_value;
}

%token <int_value> INT_LITERAL;
%token ADD SUB MUL DIV CR
%type <int_value> expression term primary_expression

%%

line_list
    : line
    | line_list line
    ;

line
    :expression CR
    {
        OP * tmp = new OP;
        tmp->op_type = OP_PRINT;
        vm.ops.push_back(tmp);
    }

expression
    : term
    | expression ADD term
    {
        OP * tmp = new OP;
        tmp->op_type = OP_ADD;
        vm.ops.push_back(tmp);
        $$ = $1 + $3
    }
    | expression SUB term
    {
        OP * tmp = new OP;
        tmp->op_type = OP_SUB;
        vm.ops.push_back(tmp);
        $$ = $1 - $3
    }
    ;

term
    : primary_expression
    | term MUL primary_expression
    {
        OP * tmp = new OP;
        tmp->op_type = OP_MUL;
        vm.ops.push_back(tmp);
        $$ = $1 * $3;
    }
    | term DIV primary_expression
    {
        OP * tmp = new OP;
        tmp->op_type = OP_DIV;
        vm.ops.push_back(tmp);
        $$ = $1 / $3;
    }
    ;

primary_expression
    : INT_LITERAL
    {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.operand_int = $1;
        vm.ops.push_back(tmp);
        $$ = $1;
    }
    ;

%%
int yyerror(const char *err) {
    extern char *yytext;
    fprintf(stderr, "parser error near %s\n", yytext);
    return 0;
}

int main() {
    extern int yyparse(void);
    extern FILE *yyin;
    yyin = stdin;
    if (yyparse()) {
        fprintf(stderr, "Error!\n");
        exit(1);
    } else {
        vm.execute();
    }
}

