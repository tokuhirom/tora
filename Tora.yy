%{
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <cassert>
#include <sstream>
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
    OP_PUSH_INT,
    OP_PUSH_TRUE,
    OP_PUSH_FALSE,
    OP_PUSH_IDENTIFIER,
    OP_FUNCALL,
} OP_TYPE;

typedef struct {
    int op_type;
    union {
        int  int_value;
        bool bool_value;
        char *str_value;
    } operand;
} OP;

typedef enum {
    VALUE_TYPE_INT = 1,
    VALUE_TYPE_BOOL,
    VALUE_TYPE_STR,
} value_type_t;

/**
 * The value class
 */
class Value {
public:
    value_type_t value_type;
    union {
        int  int_value;
        bool bool_value;
        const char *str_value;
    } value;
    int refcnt; // reference count

    Value() {
        refcnt = 1;
    }
    ~Value() {
        if (value_type == VALUE_TYPE_STR) {
            delete [] value.str_value;
        }
    }
    void release() {
        --refcnt;
        if (refcnt == 0) {
            delete this;
        }
    }
    void retain() {
        ++refcnt;
    }
    void set_int(int i) {
        value.int_value = i;
        value_type = VALUE_TYPE_INT;
    }
    void set_bool(bool b) {
        value.bool_value = b;
        value_type = VALUE_TYPE_BOOL;
    }
    void set_str(const char *s) {
        // memory leak
        value.str_value = s;
        value_type = VALUE_TYPE_STR;
    }
    void dump() {
        switch (value_type) {
        case VALUE_TYPE_INT:
            printf("[dump] IV: %d\n", value.int_value);
            break;
        case VALUE_TYPE_BOOL:
            printf("[dump] bool: %s\n", value.bool_value ? "true" : "false");
            break;
        case VALUE_TYPE_STR:
            printf("[dump] str: %s\n", value.str_value);
            break;
        default:
            printf("[dump] unknown: %d\n", value_type);
            break;
        }
    }
    Value *to_s() {
        switch (value_type) {
        case VALUE_TYPE_INT: {
            Value *v = new Value();
            std::ostringstream os;
            os << this->value.int_value;
            v->set_str(strdup(os.str().c_str()));
            return v;
        }
        case VALUE_TYPE_STR: {
            Value *v;
            v->set_str(strdup(this->value.str_value));
            return v;
        }
        default:
            printf("[BUG] unknown in to_s: %d\n", value_type);
            abort();
            break;
        }
    }
};

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
            case OP_PUSH_TRUE: {
                Value *v = new Value;
                v->set_bool(false);
                stack.push_back(v);
                break;
            }
            case OP_PUSH_FALSE: {
                Value *v = new Value;
                v->set_bool(false);
                stack.push_back(v);
                break;
            }
            case OP_PUSH_INT: {
                Value *v = new Value;
                v->set_int(op->operand.int_value);
                stack.push_back(v);
                break;
            }
            case OP_ADD: {
                Value *v1 = stack.back();
                Value *v2 = stack.back();
                stack.pop_back();
                stack.pop_back();
                Value *v = new Value;
                v->set_int(v1->value.int_value + v2->value.int_value);
                stack.push_back(v);
                break;
            }
            case OP_SUB: {
                Value *v1 = stack.back();
                Value *v2 = stack.back();
                stack.pop_back();
                stack.pop_back();
                Value *v = new Value;
                v->set_int(v1->value.int_value - v2->value.int_value);
                stack.push_back(v);
                break;
            }
            case OP_FUNCALL: {
                printf("OP_FUNCALL with %d\n", stack.size());
                assert(stack.size() >= op->operand.int_value + 1);
                Value *funname = stack.at(stack.size()-op->operand.int_value-1);
                assert(funname->value_type == VALUE_TYPE_STR);
                funname->dump();
                if (strcmp(funname->value.str_value, "print") == 0) {
                    Value *v = stack.back();
                    stack.pop_back();
                    Value *s = v->to_s();
                    printf("%s", s->value.str_value);
                    s->release();
                } else if (strcmp(funname->value.str_value, "say") == 0) {
                    Value *v = stack.back();
                    stack.pop_back();
                    Value *s = v->to_s();
                    printf("%s\n", s->value.str_value);
                    s->release();
                } else {
                    fprintf(stderr, "Unknown function: %s\n");
                }
                dump_stack();
                break;
            }
            case OP_PUSH_IDENTIFIER: {
                // operand = the number of args
                printf("OP_PUSH_IDENTIFIER\n");
                Value *v = new Value;
                v->value.str_value = op->operand.str_value;
                v->value_type = VALUE_TYPE_STR;
                v->dump();
                stack.push_back(v);
                break;
            }
            case OP_PRINT: {
                Value *v1 = stack.back();
                printf(">> %d\n", v1->value.int_value);
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
    void dump_stack() {
        printf("-- STACK DUMP --\nSP: %d\n", sp);
        for (int i=0; i<stack.size(); i++) {
            printf("[%d] ", i);
            stack.at(i)->dump();
        }
        printf("--           --\n");
    }
};

VM vm;

%}
%token IF
%token L_PAREN R_PAREN L_BRACE R_BRACE
%union {
    int int_value;
    char *str_value;
}

%token <int_value> INT_LITERAL;
%token <str_value> IDENTIFIER;
%token ADD SUB MUL DIV CR
%token TRUE FALSE
%type <int_value> expression term primary_expression
%type <str_value> identifier

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
    | CR
    | IF "(" expression ")" "{" line_list "}"
    {
        printf("if stmt\n");
    }
    | identifier L_PAREN expression R_PAREN
    {
        // funciton call
        // TODO: support multiple args
        // TODO: support vargs
        // call function
        printf("FUNCALL stmt in parser!\n");
        OP * tmp = new OP;
        tmp->op_type = OP_FUNCALL;
        tmp->operand.int_value = 1; // the number of args
        vm.ops.push_back(tmp);
    }

expression
    : term
    | expression ADD term
    {
        OP * tmp = new OP;
        tmp->op_type = OP_ADD;
        vm.ops.push_back(tmp);
    }
    | expression SUB term
    {
        OP * tmp = new OP;
        tmp->op_type = OP_SUB;
        vm.ops.push_back(tmp);
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
        tmp->operand.int_value = $1;
        vm.ops.push_back(tmp);
        $$ = $1;
    }
    | FALSE
    {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        vm.ops.push_back(tmp);
    }
    | TRUE
    {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        vm.ops.push_back(tmp);
    }

identifier
    : IDENTIFIER
    {
        printf("IDENTIFIER in parser\n");
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_IDENTIFIER;
        tmp->operand.str_value = $1;
        vm.ops.push_back(tmp);
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

