#include "compiler.h"

void tora::Compiler::compile(TNode *node) {
    switch (node->type) {
    case NODE_ROOT: {
        this->push_block();
        this->compile(node->node);

        {
            OP * tmp = new OP;
            tmp->op_type = OP_END;
            vm->ops.push_back(tmp);
        }

        this->pop_block();

        break;
    }
    case NODE_MY: {
        const char *name = node->node->str_value;
        this->define_localvar(std::string(name));
        this->compile(node->node);
        break;
    }
    case NODE_RETURN: {
        this->compile(node->node);
        break;
    }
    case NODE_BLOCK: {
        this->push_block();

        OP * enter = new OP;
        enter->op_type = OP_ENTER;
        vm->ops.push_back(enter);

        this->compile(node->node);

        vm->ops.push_back(new OP(OP_LEAVE));

        this->pop_block();

        break;
    }
    case NODE_FUNCDEF: {
        /*
        struct {
            struct TNode *name;
            std::vector<struct TNode*> *params;
            struct TNode *block;
        } funcdef;
        */

        // function name
        const char *funcname = node->funcdef.name->str_value;

        OP * jump = new OP;
        jump->op_type = OP_JUMP;
        vm->ops.push_back(jump);

        std::vector<std::string *>* params = new std::vector<std::string *>();
        for (size_t i=0; i<node->funcdef.params->size(); i++) {
            params->push_back(new std::string(node->funcdef.params->at(i)->str_value));
            this->define_localvar(std::string(node->funcdef.params->at(i)->str_value));
        }
        Value *code = new Value();
        code->value_type = VALUE_TYPE_CODE;
        code->value.code_value.name = strdup(funcname);
        code->value.code_value.start = vm->ops.size();
        code->value.code_value.params = params;

        vm->add_function(funcname, code);

        this->compile(node->funcdef.block);

        OP * ret = new OP;
        ret->op_type = OP_RETURN;
        vm->ops.push_back(ret);

        jump->operand.int_value = vm->ops.size();

        break;
    }
    case NODE_STRING: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_STRING;
        tmp->operand.str_value = strdup(node->str_value);
        vm->ops.push_back(tmp);
        break;
    }
    case NODE_INT: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.int_value = node->int_value;
        vm->ops.push_back(tmp);
        break;
    }
    case NODE_TRUE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        vm->ops.push_back(tmp);
        break;
    }
    case NODE_FALSE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        vm->ops.push_back(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_IDENTIFIER;
        tmp->operand.str_value = strdup(node->str_value);
        vm->ops.push_back(tmp);
        break;
    }

    case NODE_FUNCALL: {
        std::vector<TNode *>*args = node->funcall.args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }
        this->compile(node->funcall.name);

        OP * tmp = new OP;
        tmp->op_type = OP_FUNCALL;
        tmp->operand.int_value = args_len; // the number of args
        vm->ops.push_back(tmp);
        break;
    }
#define C_OP_BINARY(type) \
    { \
        this->compile(node->binary.left); \
        this->compile(node->binary.right); \
        OP * tmp = new OP; \
        tmp->op_type = (type); \
        vm->ops.push_back(tmp); \
        break; \
    }

    case NODE_ADD: C_OP_BINARY(OP_ADD);
    case NODE_SUB: C_OP_BINARY(OP_SUB);
    case NODE_MUL: C_OP_BINARY(OP_MUL);
    case NODE_DIV: C_OP_BINARY(OP_DIV);
    case NODE_LT:  C_OP_BINARY(OP_LT);
    case NODE_GT:  C_OP_BINARY(OP_GT);
    case NODE_LE:  C_OP_BINARY(OP_LE);
    case NODE_GE:  C_OP_BINARY(OP_GE);
    case NODE_EQ:  C_OP_BINARY(OP_EQ);
    case NODE_ASSIGN:  C_OP_BINARY(OP_ASSIGN);
#undef C_OP_BINARY

    case NODE_STMTS: {
        this->compile(node->binary.left);
        this->compile(node->binary.right);
        break;
    }
    case NODE_IF: {
        /*
            if (cond) {
                if_body
            } else {
                else_body
            }

            run(cond)
            jump_if_false ELSE_LABEL
            run_if_body
            jump END_LABEL
        ELSE_LABEL:
            run_else_body
        END_LABEL:

            run(cond)
            jump_if_false ELSE_LABEL
            run_if_body
            jump END_LABEL
        ELSE_LABEL:
        END_LABEL:
        */
        this->compile(node->binary.left); // jouken

        OP * jump_else = new OP;
        jump_else->op_type = OP_JUMP_IF_FALSE;
        vm->ops.push_back(jump_else);

        this->compile(node->binary.right); // block

        OP * jump_end = new OP;
        jump_end->op_type = OP_JUMP;
        vm->ops.push_back(jump_end);

        int else_label = vm->ops.size();
        jump_else->operand.int_value = else_label;
        if (node->if_stmt.else_body) {
            this->compile(node->if_stmt.else_body);
        }

        int end_label = vm->ops.size();
        jump_end->operand.int_value = end_label;

        break;
    }
    case NODE_WHILE: {
        /*
        LABEL1:
          cond
          jump_if_false LABEL2
          body
          goto LABEL1
        LABEL2:
        */
        int label1 = vm->ops.size();
        this->compile(node->binary.left); // cond

        OP * jump_if_false = new OP;
        jump_if_false->op_type = OP_JUMP_IF_FALSE;
        vm->ops.push_back(jump_if_false);

        this->compile(node->binary.right); //body

        OP * goto_ = new OP;
        goto_->op_type = OP_JUMP;
        goto_->operand.int_value = label1;
        vm->ops.push_back(goto_);

        int label2 = vm->ops.size();
        jump_if_false->operand.int_value = label2;

        break;
    }
    case NODE_VOID:
        // nop
        break;
    case NODE_NEWLINE:
        // nop
        break;
    case NODE_VARIABLE: {
        int no = this->find_localvar(std::string(node->str_value));
        if (no<0) {
            fprintf(stderr, "There is no variable named %s\n", node->str_value);
            error = true;
        }
        OP * tmp = new OP;
        tmp->op_type = OP_VARIABLE;
        tmp->operand.str_value = node->str_value;
        vm->ops.push_back(tmp);
        break;
    }

    default:
        printf("Unknown node: %d\n", node->type);
        abort();
        break;
    }
}
