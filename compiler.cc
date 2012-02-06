#include "nodes.gen.h"
#include "node.h"
#include "compiler.h"
#include "code.h"

int tora::Compiler::find_localvar(std::string name, int &level) {
    DBG("FIND LOCAL VAR %d\n", 0);
    for (level = 0; level<this->blocks->size(); level++) {
        SharedPtr<Block> block = this->blocks->at(this->blocks->size()-1-level);
        for (size_t i=0; i<block->vars.size(); i++) {
            if (*(block->vars.at(i)) == name) {
                return i;
            }
        }
    }
    return -1;
}

void tora::Compiler::compile(Node *node) {
    switch (node->type) {
    case NODE_ROOT: {
        this->push_block();
        this->compile(node->to_node_node()->node);

        {
            OP * tmp = new OP;
            tmp->op_type = OP_END;
            ops->push_back(tmp);
        }

        this->pop_block();

        break;
    }
    case NODE_MY: {
        const char *name = node->to_node_node()->node->to_str_node()->str_value;
        this->define_localvar(std::string(name));
        // this->compile(node->to_node_node()->node);
        break;
    }
    case NODE_RETURN: {
        this->compile(node->to_node_node()->node);
        break;
    }
    case NODE_BLOCK: {
        this->push_block();

        SharedPtr<OP> enter = new OP;
        enter->op_type = OP_ENTER;
        ops->push_back(enter);

        this->compile(node->to_node_node()->node);

        ops->push_back(new OP(OP_LEAVE));

        this->pop_block();

        break;
    }
    case NODE_FUNCDEF: {
        /*
        struct {
            Node *name;
            std::vector<Node*> *params;
            Node *block;
        } funcdef;
        
        putcodevalue v
        make_function
        */

        auto funcdef_node = node->to_funcdef_node();

        // function name
        const char *funcname = funcdef_node->name->to_str_node()->str_value;

        auto params = new std::vector<std::string *>();
        for (size_t i=0; i<funcdef_node->params->size(); i++) {
            params->push_back(new std::string(funcdef_node->params->at(i)->to_str_node()->str_value));
            this->define_localvar(std::string(funcdef_node->params->at(i)->to_str_node()->str_value));
        }

        Compiler funccomp;
        funccomp.blocks = new std::vector<SharedPtr<Block>>(*(this->blocks));
        funccomp.compile(funcdef_node->block);

        OP * ret = new OP;
        ret->op_type = OP_RETURN;
        funccomp.ops->push_back(ret);

        CodeValue *code = new CodeValue();
        code->code_name = strdup(funcname);
        code->code_params = params;
        code->code_opcodes = new std::vector<SharedPtr<OP>>(*funccomp.ops);

        ValueOP * putval = new ValueOP(OP_PUSH_VALUE, code);
        ops->push_back(putval);

        StrValue *funcname_value = new StrValue(strdup(funcname));
        ValueOP * define_method = new ValueOP(OP_DEFINE_METHOD, funcname_value);
        ops->push_back(define_method);

        break;
    }
    case NODE_STRING: {
        StrValue *sv = new StrValue(strdup(node->to_str_node()->str_value));
        ValueOP * tmp = new ValueOP(OP_PUSH_STRING, sv);
        ops->push_back(tmp);
        break;
    }
    case NODE_INT: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.int_value = node->to_int_node()->int_value;
        ops->push_back(tmp);
        break;
    }
    case NODE_DOUBLE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_DOUBLE;
        tmp->operand.double_value = node->to_double_node()->double_value;
        ops->push_back(tmp);
        break;
    }
    case NODE_TRUE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        ops->push_back(tmp);
        break;
    }
    case NODE_FALSE: {
        OP * tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        ops->push_back(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        StrValue *sv = new StrValue(strdup(node->to_str_node()->str_value));
        ValueOP * tmp = new ValueOP(OP_PUSH_IDENTIFIER, sv);
        ops->push_back(tmp);
        break;
    }

    case NODE_FUNCALL: {
        std::vector<Node *>*args = node->to_funcall_node()->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }
        this->compile(node->to_funcall_node()->name);

        OP * tmp = new OP;
        tmp->op_type = OP_FUNCALL;
        tmp->operand.int_value = args_len; // the number of args
        ops->push_back(tmp);
        break;
    }
#define C_OP_BINARY(type) \
    { \
        this->compile(node->binary()->left); \
        this->compile(node->binary()->right); \
        OP * tmp = new OP; \
        tmp->op_type = (type); \
        ops->push_back(tmp); \
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
        this->compile(node->binary()->left);
        this->compile(node->binary()->right);
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
        auto if_node = node->to_if_node();
        this->compile(if_node->cond);

        OP * jump_else = new OP;
        jump_else->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_else);

        this->compile(if_node->if_body);

        OP * jump_end = new OP;
        jump_end->op_type = OP_JUMP;
        ops->push_back(jump_end);

        int else_label = ops->size();
        jump_else->operand.int_value = else_label;
        if (if_node->else_body) {
            this->compile(if_node->else_body);
        }

        int end_label = ops->size();
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
        int label1 = ops->size();
        this->compile(node->binary()->left); // cond

        OP * jump_if_false = new OP;
        jump_if_false->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_if_false);

        this->compile(node->binary()->right); //body

        OP * goto_ = new OP;
        goto_->op_type = OP_JUMP;
        goto_->operand.int_value = label1;
        ops->push_back(goto_);

        int label2 = ops->size();
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
        int no = this->find_localvar(std::string(node->to_str_node()->str_value), level);
        if (no<0) {
            fprintf(stderr, "There is no variable named '%s'(NODE_GETVARIABLE)\n", node->to_str_node()->str_value);
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
        ops->push_back(tmp);
        break;
    }
    case NODE_SETVARIABLE: {
        switch (node->binary()->left->type) {
        case NODE_GETVARIABLE: { // $a = $b;
            const char*varname = node->binary()->left->to_str_node()->str_value;
            int level;
            int no = this->find_localvar(std::string(varname), level);
            if (no<0) {
                fprintf(stderr, "There is no variable named %s(SETVARIABLE)\n", varname);
                error = true;
            }

            // fprintf(stderr, "set level: %d\n", level);
            this->compile(node->binary()->right);

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
            ops->push_back(tmp);
            break;
        }
        case NODE_GET_ITEM: { // $a[$b] = $c
            auto container = node->binary()->left->binary()->left;
            auto index     = node->binary()->left->binary()->right;
            auto rvalue    = node->binary()->right;
            this->compile(container);
            this->compile(index);
            this->compile(rvalue);

            OP * tmp = new OP;
            tmp->op_type = OP_SET_ITEM;
            ops->push_back(tmp);
            break;
        }
        default:
            printf("This is not lvalue\n");
            node->dump(1);
            error = true;
            break;
        }

        break;
    }
    case NODE_MAKE_ARRAY: {
        auto args = node->to_args_node()->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }

        OP * tmp = new OP;
        tmp->op_type = OP_MAKE_ARRAY;
        tmp->operand.int_value = args_len; // the number of args
        ops->push_back(tmp);
        break;
    }
    case NODE_GET_ITEM: {
        this->compile(node->binary()->left);  // container
        this->compile(node->binary()->right); // index

        OP * tmp = new OP;
        tmp->op_type = OP_GET_ITEM;
        ops->push_back(tmp);
        break;
    }

    case NODE_UNARY_NEGATIVE: {
        this->compile(node->to_node_node()->node);

        ops->push_back(new OP(OP_UNARY_NEGATIVE));

        break;
    }

    case NODE_FOR: {
        /*
        struct {
            Node *initialize;
            Node *cond;
            Node *postfix;
            Node *block;
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

        this->compile(node->to_for_node()->initialize);
        int label1 = ops->size();
        this->compile(node->to_for_node()->cond);

        OP * jump_label2 = new OP;
        jump_label2->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_label2);

        this->compile(node->to_for_node()->block);
        this->compile(node->to_for_node()->postfix);

        OP * jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        ops->push_back(jump_label1);

        int label2 = ops->size();
        jump_label2->operand.int_value = label2;
        break;
    }
    case NODE_METHOD_CALL: {
        /*
        node->method_call.object = $1;
        node->method_call.method = $3;
        node->method_call.args   = $5;
        */
        auto mcn = node->to_method_call_node();
        auto args = mcn->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }
        this->compile(mcn->method);
        this->compile(mcn->object);

        OP * tmp = new OP;
        tmp->op_type = OP_METHOD_CALL;
        tmp->operand.int_value = args_len; // the number of args
        ops->push_back(tmp);
        break;
    }

    default:
        printf("Unknown node: %s\n", node->type_name_str());
        abort();
        break;
    }
}
