#include "compiler.h"

int tora::Compiler::find_localvar(std::string name, int &level) {
    DBG("FIND LOCAL VAR %d\n", 0);
    for (level = 0; level<this->blocks->size(); level++) {
        Block *block = this->blocks->at(this->blocks->size()-1-level);
        for (size_t i=0; i<block->vars.size(); i++) {
            if (*(block->vars.at(i)) == name) {
                return i;
            }
        }
    }
    return -1;
}

void tora::Compiler::compile(TNode *node) {
    switch (node->type) {
    case NODE_ROOT: {
        this->push_block();
        this->compile(node->node);

        {
            OP * tmp = new OP;
            tmp->op_type = OP_END;
            vm->ops->push_back(tmp);
        }

        this->pop_block();

        break;
    }
    case NODE_MY: {
        const char *name = node->node->str_value;
        this->define_localvar(std::string(name));
        // this->compile(node->node);
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
        vm->ops->push_back(enter);

        this->compile(node->node);

        vm->ops->push_back(new OP(OP_LEAVE));

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
        
        putcodevalue v
        make_function
        */

        // function name
        const char *funcname = node->funcdef.name->str_value;


        std::vector<std::string *>* params = new std::vector<std::string *>();
        for (size_t i=0; i<node->funcdef.params->size(); i++) {
            params->push_back(new std::string(node->funcdef.params->at(i)->str_value));
            this->define_localvar(std::string(node->funcdef.params->at(i)->str_value));
        }

        // TODO: better Compiler class definition
        VM vm_;
        Compiler funccomp(&vm_);
        funccomp.blocks = this->blocks;
        funccomp.compile(node->funcdef.block);

        OP * ret = new OP;
        ret->op_type = OP_RETURN;
        vm_.ops->push_back(ret);

        CodeValue *code = new CodeValue();
        code->code_name = strdup(funcname);
        code->code_params = params;
        code->code_opcodes = vm_.ops; // TODO: clone

        OP * putval = new OP;
        putval->op_type = OP_PUSH_VALUE;
        putval->operand.value = (Value*)code;
        vm->ops->push_back(putval);

        OP * define_method = new OP;
        define_method->op_type = OP_DEFINE_METHOD;
        define_method->operand.str_value = strdup(funcname);
        vm->ops->push_back(define_method);

        break;
    }
    case NODE_STRING: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_STRING;
        tmp->operand.str_value = strdup(node->str_value);
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_INT: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.int_value = node->int_value;
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_TRUE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_FALSE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_IDENTIFIER;
        tmp->operand.str_value = strdup(node->str_value);
        vm->ops->push_back(tmp);
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
        vm->ops->push_back(tmp);
        break;
    }
#define C_OP_BINARY(type) \
    { \
        this->compile(node->binary.left); \
        this->compile(node->binary.right); \
        OP * tmp = new OP; \
        tmp->op_type = (type); \
        vm->ops->push_back(tmp); \
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
        vm->ops->push_back(jump_else);

        this->compile(node->binary.right); // block

        OP * jump_end = new OP;
        jump_end->op_type = OP_JUMP;
        vm->ops->push_back(jump_end);

        int else_label = vm->ops->size();
        jump_else->operand.int_value = else_label;
        if (node->if_stmt.else_body) {
            this->compile(node->if_stmt.else_body);
        }

        int end_label = vm->ops->size();
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
        int label1 = vm->ops->size();
        this->compile(node->binary.left); // cond

        OP * jump_if_false = new OP;
        jump_if_false->op_type = OP_JUMP_IF_FALSE;
        vm->ops->push_back(jump_if_false);

        this->compile(node->binary.right); //body

        OP * goto_ = new OP;
        goto_->op_type = OP_JUMP;
        goto_->operand.int_value = label1;
        vm->ops->push_back(goto_);

        int label2 = vm->ops->size();
        jump_if_false->operand.int_value = label2;

        break;
    }
    case NODE_VOID:
        // nop
        break;
    case NODE_NEWLINE:
        // nop
        break;
    case NODE_GETVARIABLE: {
        int level;
        int no = this->find_localvar(std::string(node->str_value), level);
        if (no<0) {
            fprintf(stderr, "There is no variable named '%s'(NODE_GETVARIABLE)\n", node->str_value);
            error = true;
        }

        OP * tmp = new OP;
        if (level == 0) {
            DBG2("LOCAL\n");
            tmp->op_type = OP_GETLOCAL;
            tmp->operand.int_value = no;
        } else {
            DBG2("DYNAMIC\n");
            tmp->op_type = OP_GETDYNAMIC;
            tmp->operand.int_value = (((level)&0x0000ffff) << 16) | (no&0x0000ffff);
        }
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_SETVARIABLE: {
        const char*varname = node->set_value.lvalue->str_value;
        int level;
        int no = this->find_localvar(std::string(varname), level);
        if (no<0) {
            fprintf(stderr, "There is no variable named %s\n", varname);
            error = true;
        }

        // fprintf(stderr, "set level: %d\n", level);
        this->compile(node->set_value.rvalue);

        OP * tmp = new OP;
        if (level == 0) {
            DBG2("LOCAL\n");
            tmp->op_type = OP_SETLOCAL;
            tmp->operand.int_value = no;
        } else {
            DBG2("DYNAMIC\n");
            tmp->op_type = OP_SETDYNAMIC;
            tmp->operand.int_value = (((level)&0x0000ffff) << 16) | (no&0x0000ffff);
        }
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_MAKE_ARRAY: {
        std::vector<TNode *>*args = node->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }

        OP * tmp = new OP;
        tmp->op_type = OP_MAKE_ARRAY;
        tmp->operand.int_value = args_len; // the number of args
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_GET_ITEM: {
        this->compile(node->binary.left);  // container
        this->compile(node->binary.right); // index

        OP * tmp = new OP;
        tmp->op_type = OP_GET_ITEM;
        vm->ops->push_back(tmp);
        break;
    }
    case NODE_SET_ITEM: {
        this->compile(node->set_item.container);
        this->compile(node->set_item.index);
        this->compile(node->set_item.rvalue);

        OP * tmp = new OP;
        tmp->op_type = OP_SET_ITEM;
        vm->ops->push_back(tmp);
        break;
    }

    case NODE_UNARY_NEGATIVE: {
        this->compile(node->node);

        vm->ops->push_back(new OP(OP_UNARY_NEGATIVE));

        break;
    }

    case NODE_FOR: {
        /*
        struct {
            struct TNode *initialize;
            struct TNode *cond;
            struct TNode *postfix;
            struct TNode *block;
        } for_stmt;

            (initialize)
        LABEL1:
            (cond)
            jump_if_false LABEL2
            (block)
            (postfix)
            goto LABEL1
        LABEL2:
        */

        this->compile(node->for_stmt.initialize);
        int label1 = vm->ops->size();
        this->compile(node->for_stmt.cond);

        OP * jump_label2 = new OP;
        jump_label2->op_type = OP_JUMP_IF_FALSE;
        vm->ops->push_back(jump_label2);

        this->compile(node->for_stmt.block);
        this->compile(node->for_stmt.postfix);

        OP * jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        vm->ops->push_back(jump_label1);

        int label2 = vm->ops->size();
        jump_label2->operand.int_value = label2;
        break;
    }

    default:
        printf("Unknown node: %s\n", node->type_name_str());
        abort();
        break;
    }
}
