%{
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <cassert>
#include <sstream>
#include <unistd.h>

#include "ops.gen.h"

#define YYDEBUG 1

extern int yylex();
int yyerror(const char *err);

typedef struct TNode {
    int type;
    union {
        struct TNode *node;
        int int_value;
        const char*str_value;
        struct {
            struct TNode *left;
            struct TNode *right;
        } binary;
    };
} NODE;

typedef enum {
    NODE_INT = 0,
    NODE_TRUE,
    NODE_FALSE,
    NODE_IDENTIFIER,
    NODE_FUNCALL,
    NODE_STRING,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_STMTS,
    NODE_NEWLINE,
} NODE_TYPE;

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
            Value *v = new Value();
            v->set_str(strdup(this->value.str_value));
            return v;
        }
        default:
            printf("[BUG] unknown in to_s: %d\n", value_type);
            abort();
            break;
        }
    }
    Value *to_i() {
        switch (value_type) {
        case VALUE_TYPE_INT: {
            Value *v = new Value();
            v->set_int(this->value.int_value);
            return v;
        }
        default:
            abort();
        }
    }
};

class VM {
public:
    std::vector<Value*> stack;
    int sp; // stack pointer
    size_t pc; // program counter
    std::vector<OP*> ops;

    VM() {
        sp = 0;
        pc = 0;
    }

    // run program
    void execute() {
        // printf("[DEBUG] OPS: %d, %d\n", pc, ops.size());
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
            case OP_PUSH_STRING: {
                Value *v = new Value;
                v->set_str(op->operand.str_value);
                stack.push_back(v);
                break;
            }
            case OP_ADD: {
                Value *v1 = stack.back();
                stack.pop_back();
                Value *v2 = stack.back();
                stack.pop_back();
                Value *v = new Value;
                v->set_int(v1->value.int_value + v2->value.int_value);
                stack.push_back(v);
                v1->release();
                v2->release();
                break;
            }
            case OP_SUB: {
                Value *v1 = stack.back();
                stack.pop_back();
                Value *v2 = stack.back();
                stack.pop_back();
                Value *v = new Value;
                v->set_int(v2->value.int_value - v1->value.int_value);
                stack.push_back(v);
                v1->release();
                v2->release();
                break;
            }
            case OP_MUL: {
                Value *v1 = stack.back();
                stack.pop_back();
                Value *v2 = stack.back();
                stack.pop_back();
                Value *v = new Value;
                v->set_int(v2->value.int_value * v1->value.int_value);
                stack.push_back(v);
                v1->release();
                v2->release();
                break;
            }
            case OP_DIV: {
                Value *v1 = stack.back();
                stack.pop_back();
                Value *v2 = stack.back();
                stack.pop_back();
                Value *v = new Value;
                v->set_int(v2->value.int_value / v1->value.int_value);
                stack.push_back(v);
                v1->release();
                v2->release();
                break;
            }
            case OP_FUNCALL: {
                assert(stack.size() >= op->operand.int_value + 1);
                Value *funname = stack.at(stack.size()-op->operand.int_value-1);
                assert(funname->value_type == VALUE_TYPE_STR);
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
                } else if (strcmp(funname->value.str_value, "exit") == 0) {
                    Value *v = stack.back();
                    stack.pop_back();
                    assert(v->value_type == VALUE_TYPE_INT);
                    Value *s = v->to_i();
                    exit(s->value.int_value);
                    s->release();
                    v->release();
                } else {
                    fprintf(stderr, "Unknown function: %s\n", funname->value.str_value);
                }
                break;
            }
            case OP_PUSH_IDENTIFIER: {
                // operand = the number of args
                Value *v = new Value;
                v->value.str_value = op->operand.str_value;
                v->value_type = VALUE_TYPE_STR;
                stack.push_back(v);
                break;
            }
            case OP_DUMP: {
                this->dump_stack();
                break;
            }
            default: {
                fprintf(stderr, "[BUG] OOPS. unknown op code: %d\n", op->op_type);
                abort();
                break;
            }
            }
            pc++;
        }
    }
    void dump_ops() {
        printf("-- OP DUMP    --\n");
        for (size_t i=0; i<ops.size(); i++) {
            printf("[%d] %s\n", i, opcode2name[ops[i]->op_type]);
        }
        printf("----------------\n");
    }
    void dump_stack() {
        printf("-- STACK DUMP --\nSP: %d\n", sp);
        for (size_t i=0; i<stack.size(); i++) {
            printf("[%d] ", i);
            stack.at(i)->dump();
        }
        printf("----------------\n");
    }
};

VM vm;

static TNode *tora_create_binary_expression(NODE_TYPE type, TNode *t1, TNode* t2) {
    TNode *node = new TNode();
    node->type = type;
    node->binary.left  = t1;
    node->binary.right = t2;
    return node;
}

void tora_dump_node(TNode *node, int indent) {
    for (int i=0; i<indent*4; i++) {
        printf(" ");
    }
    switch (node->type) {
    case NODE_STRING:
        printf("NODE_STRING('%s')\n", node->str_value);
        break;
    case NODE_INT:
        printf("NODE_INT(%d)\n", node->int_value);
        break;
    case NODE_TRUE:
        printf("NODE_TRUE\n");
        break;
    case NODE_FALSE:
        printf("NODE_FALSE\n");
        break;
    case NODE_IDENTIFIER:
        printf("NODE_IDENTIFIER(%s)\n", node->str_value);
        break;
    case NODE_NEWLINE:
        printf("NODE_NEWLINE\n");
        break;

    case NODE_FUNCALL:
        printf("NODE_FUNCALL\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_ADD:
        printf("NODE_ADD\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_SUB:
        printf("NODE_SUB\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_MUL:
        printf("NODE_MUL\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_DIV:
        printf("NODE_DIV\n");
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;
    case NODE_STMTS:
        printf("NODE_STMTS\n");
        assert(node->binary.left);
        assert(node->binary.right);
        tora_dump_node(node->binary.left, 1);
        tora_dump_node(node->binary.right, 1);
        break;

    default:
        printf("Unknown node\n");
        break;
    }
}
void tora_dump_node(TNode *node) {
    tora_dump_node(node, 0);
}

static TNode *root_node;

static void tora_compile(TNode *node) {
    switch (node->type) {
    case NODE_STRING: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_STRING;
        tmp->operand.str_value = strdup(node->str_value);
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_INT: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.int_value = node->int_value;
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_TRUE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_FALSE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        vm.ops.push_back(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_IDENTIFIER;
        tmp->operand.str_value = strdup(node->str_value);
        vm.ops.push_back(tmp);
        break;
    }

    case NODE_FUNCALL: {
        tora_compile(node->binary.left);
        tora_compile(node->binary.right);

        OP * tmp = new OP;
        tmp->op_type = OP_FUNCALL;
        tmp->operand.int_value = 1; // the number of args
        vm.ops.push_back(tmp);
        break;
    }
#define C_OP_BINARY(type) \
    { \
        tora_compile(node->binary.left); \
        tora_compile(node->binary.right); \
        OP * tmp = new OP; \
        tmp->op_type = (type); \
        vm.ops.push_back(tmp); \
        break; \
    }

    case NODE_ADD: C_OP_BINARY(OP_ADD);
    case NODE_SUB: C_OP_BINARY(OP_SUB);
    case NODE_MUL: C_OP_BINARY(OP_MUL);
    case NODE_DIV: C_OP_BINARY(OP_DIV);
#undef C_OP_BINARY

    case NODE_STMTS: {
        tora_compile(node->binary.left);
        tora_compile(node->binary.right);
        break;
    }
    case NODE_NEWLINE:
        // nop
        break;

    default:
        printf("Unknown node: %d\n", node->type);
        abort();
        break;
    }
}

%}
%token IF
%token L_PAREN R_PAREN L_BRACE R_BRACE
%union {
    int int_value;
    char *str_value;
    TNode *node;
}

%token <int_value> INT_LITERAL;
%token <str_value> IDENTIFIER;
%token ADD SUB MUL DIV CR
%token TRUE FALSE
%token <str_value>STRING_LITERAL
%type <node> expression term primary_expression line line_list root
%type <node> identifier

%%

root
    : line_list
    {
        // tora_dump_node($1);
        root_node = $1;
    }

line_list
    : line
    | line_list line
    {
        $$ = tora_create_binary_expression(NODE_STMTS, $1, $2);
    }
    ;

line
    :expression CR
    {
        $$ = $1;
    }
    | CR
    {
        TNode *node = new TNode();
        node->type = NODE_NEWLINE;
        $$ = node;
    }
    | IF L_PAREN expression R_PAREN L_BRACE line_list R_BRACE
    {
        // printf("if stmt: %d\n", $6);
        OP * tmp = new OP;
        tmp->op_type = OP_DUMP;
        vm.ops.push_back(tmp);
    }
    | identifier L_PAREN expression R_PAREN
    {
        // funciton call
        // TODO: support multiple args
        // TODO: support vargs
        // call function
        $$ = tora_create_binary_expression(NODE_FUNCALL, $1, $3);
    }

expression
    : term
    | expression ADD term
    {
        $$ = tora_create_binary_expression(NODE_ADD, $1, $3);
    }
    | expression SUB term
    {
        $$ = tora_create_binary_expression(NODE_SUB, $1, $3);
    }
    ;

term
    : primary_expression
    | term MUL primary_expression
    {
        $$ = tora_create_binary_expression(NODE_MUL, $1, $3);
    }
    | term DIV primary_expression
    {
        $$ = tora_create_binary_expression(NODE_DIV, $1, $3);
    }
    ;

primary_expression
    : INT_LITERAL
    {
        TNode *node = new TNode();
        node->type = NODE_INT;
        node->int_value = $1;
        $$ = node;
    }
    | FALSE
    {
        TNode *node = new TNode();
        node->type = NODE_FALSE;
        $$ = node;
    }
    | TRUE
    {
        TNode *node = new TNode();
        node->type = NODE_TRUE;
        $$ = node;
    }
    | STRING_LITERAL
    {
        TNode *node = new TNode();
        node->type = NODE_STRING;
        node->str_value = $1;
        $$ = node;
    }

identifier
    : IDENTIFIER
    {
        TNode *node = new TNode();
        node->type = NODE_IDENTIFIER;
        node->str_value = $1;
        $$ = node;
    }
    ;

%%

#define TORA_VERSION_STR "0.0.1"

int yyerror(const char *err) {
    extern char *yytext;
    fprintf(stderr, "parser error near %s\n", yytext);
    return 0;
}

int main(int argc, char **argv) {
    extern int yyparse(void);
    extern FILE *yyin;
    yyin = stdin;

    char opt;
    bool dump_ops = false;
    while ((opt = getopt(argc, argv, "vd")) != -1) {
        switch (opt) {
        case 'v':
            printf("tora version %s\n", TORA_VERSION_STR);
            exit(EXIT_SUCCESS);
        case 'd':
            dump_ops = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-v] [srcfile]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) { // source code
        FILE *fh = fopen(argv[optind], "rb");
        if (!fh) { perror(argv[optind]); exit(EXIT_FAILURE); }
        yyin = fh;
    } else {
        yyin = stdin;
    }

    root_node = NULL;
    if (yyparse()) {
        fprintf(stderr, "Error!\n");
        exit(1);
    } else {
        assert(root_node);
        tora_compile(root_node);
        if (dump_ops) {
            vm.dump_ops();
        }
        vm.execute();
    }
}

