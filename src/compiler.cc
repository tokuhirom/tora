#include "nodes.gen.h"
#include "node.h"
#include "compiler.h"
#include "value/code.h"
#include "regexp.h"

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

void tora::Compiler::init_globals() {
    this->define_global_var("$ARGV");
    this->define_global_var("$ENV");
}

void tora::Compiler::set_variable(std::string &varname) {
    int level;
    int no = this->find_localvar(varname, level);
    if (no<0) {
        fprintf(stderr, "There is no variable named %s(SETVARIABLE)\n", varname.c_str());
        this->error++;
        return;
    }

    SharedPtr<OP> tmp = new OP;
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
}

void tora::Compiler::set_lvalue(SharedPtr<Node> node) {
    switch (node->type) {
    case NODE_GETVARIABLE: { // $a = $b;
        // fprintf(stderr, "set level: %d\n", level);

        std::string varname = node->upcast<StrNode>()->str_value;
        this->set_variable(varname);
        break;
    }
    case NODE_GET_ITEM: { // $a[$b] = $c
        auto container = node->upcast<BinaryNode>()->left;
        auto index     = node->upcast<BinaryNode>()->right;
        this->compile(container);
        this->compile(index);

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_SET_ITEM;
        ops->push_back(tmp);
        break;
    }
    case NODE_TUPLE: { // ($a, $b, $c[0]) = $d
        SharedPtr<ListNode>ln = node->upcast<ListNode>();

        // extract
        OP* op = new OP(OP_EXTRACT_TUPLE);
        op->operand.int_value = ln->size();
        ops->push_back(op);

        // and set to variables
        for (size_t i=0; i < ln->size(); i++) {
            // ($a, $b) = foo();
            // ($a[0], $b) = foo();
            switch (ln->at(i)->type) {
                case NODE_GETVARIABLE: { // $a = $b;
                    std::string varname = ln->at(i)->upcast<StrNode>()->str_value;
                    this->set_variable(varname);
                    break;
                }
                case NODE_GET_ITEM: { // $a[$b] = $c
                    auto container = ln->at(i)->upcast<BinaryNode>()->left;
                    auto index     = ln->at(i)->upcast<BinaryNode>()->right;
                    this->compile(container);
                    this->compile(index);
                    ops->push_back(new OP(OP_SET_ITEM));
                    break;
                }
                default: {
                    fprintf(stderr, "Compilation failed\n");
                    this->error++;
                    break;
                }
            }
            ops->push_back(new OP(OP_POP_TOP));
        }
        break;
    }
    default:
        printf("This is not lvalue:\n");
        node->dump(1);
        this->error++;
        break;
    }
}

void tora::Compiler::compile(SharedPtr<Node> node) {
    switch (node->type) {
    case NODE_ROOT: {
        this->push_block();
        this->compile(node->upcast<NodeNode>()->node);

        ops->push_back(new OP(OP_END));

        this->pop_block();

        break;
    }
    case NODE_MY: {
        SharedPtr<ListNode>ln = node->upcast<ListNode>();
        for (size_t i=0; i < ln->size(); i++) {
            std::string &name = ln->at(i)->upcast<StrNode>()->str_value;
            this->define_localvar(name);
        }
        // this->compile(node->upcast<NodeNode>()->node);
        break;
    }
    case NODE_RETURN: {
        SharedPtr<ListNode>ln = node->upcast<ListNode>();
if (ln->size() == 1) {
    this->compile(ln->at(0));
    // ops->push_back(new OP(OP_RETURN));
} else if (ln->size() == 1) {
    ops->push_back(new OP(OP_PUSH_UNDEF));
    ops->push_back(new OP(OP_RETURN));
} else {
        for (size_t i=0; i < ln->size(); i++) {
            this->compile(ln->at(i));
        }

        SharedPtr<OP> op = new OP(OP_MAKE_TUPLE);
        op->operand.int_value = ln->size();
        ops->push_back(op);

        ops->push_back(new OP(OP_RETURN));
}

        break;
    }
    case NODE_BLOCK: {
        this->push_block();

        ops->push_back(new OP(OP_ENTER));

        this->compile(node->upcast<NodeNode>()->node);

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

        auto funcdef_node = node->upcast<FuncdefNode>();

        // function name
        std::string &funcname = funcdef_node->name->upcast<StrNode>()->str_value;

        auto params = new std::vector<std::string *>();
        for (size_t i=0; i<funcdef_node->params->size(); i++) {
            params->push_back(new std::string(funcdef_node->params->at(i)->upcast<StrNode>()->str_value));
            this->define_localvar(std::string(funcdef_node->params->at(i)->upcast<StrNode>()->str_value));
        }

        Compiler funccomp;
        funccomp.blocks = new std::vector<SharedPtr<Block>>(*(this->blocks));
        funccomp.compile(funcdef_node->block);

        // funccomp.ops->push_back(new OP(OP_PUSH_UNDEF));
        funccomp.ops->push_back(new OP(OP_RETURN));

        SharedPtr<CodeValue> code = new CodeValue();
        code->code_name = funcname;
        code->code_params = params;
        // TODO memory leaks?
        code->code_opcodes = new std::vector<SharedPtr<OP>>(*funccomp.ops);

        SharedPtr<ValueOP> putval = new ValueOP(OP_PUSH_VALUE, code);
        ops->push_back(putval);

        SharedPtr<StrValue> funcname_value = new StrValue(funcname);
        SharedPtr<ValueOP> define_method = new ValueOP(OP_DEFINE_METHOD, funcname_value);
        ops->push_back(define_method);

        break;
    }
    case NODE_STRING: {
        SharedPtr<StrValue> sv = new StrValue(node->upcast<StrNode>()->str_value);
        SharedPtr<ValueOP> tmp = new ValueOP(OP_PUSH_STRING, sv);
        ops->push_back(tmp);
        break;
    }
    case NODE_REGEXP: {
        SharedPtr<AbstractRegexpValue> sv = new RE2RegexpValue(node->upcast<RegexpNode>()->regexp_value);
        if (!sv->ok()) {
            fprintf(stderr, "Regexp compilation failed: /%s/ : %s\n", sv->pattern().c_str(), sv->error().c_str());
            this->error++;
            break;
        }
        ops->push_back(new ValueOP(OP_PUSH_VALUE, sv));
        break;
    }
    case NODE_RANGE: {
        this->compile(node->upcast<BinaryNode>()->right);
        this->compile(node->upcast<BinaryNode>()->left);
        ops->push_back(new OP(OP_NEW_RANGE));
        break;
    }
    case NODE_INT: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.int_value = node->upcast<IntNode>()->int_value;
        ops->push_back(tmp);
        break;
    }
    case NODE_DOUBLE: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_DOUBLE;
        tmp->operand.double_value = node->upcast<DoubleNode>()->double_value;
        ops->push_back(tmp);
        break;
    }
    case NODE_TRUE: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        ops->push_back(tmp);
        break;
    }
    case NODE_FALSE: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        ops->push_back(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        SharedPtr<StrValue>sv = new StrValue(node->upcast<StrNode>()->str_value);
        SharedPtr<ValueOP> tmp = new ValueOP(OP_PUSH_IDENTIFIER, sv);
        ops->push_back(tmp);
        break;
    }

    case NODE_FUNCALL: {
        auto args = node->upcast<FuncallNode>()->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }
        this->compile(node->upcast<FuncallNode>()->name);

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_FUNCALL;
        tmp->operand.int_value = args_len; // the number of args
        ops->push_back(tmp);
        break;
    }
#define C_OP_BINARY(type) \
    { \
        this->compile(node->upcast<BinaryNode>()->left); \
        this->compile(node->upcast<BinaryNode>()->right); \
        SharedPtr<OP> tmp = new OP; \
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

    // TODO: deprecate?
    case NODE_STMTS: {
        this->compile(node->upcast<BinaryNode>()->left);
        this->compile(node->upcast<BinaryNode>()->right);
        break;
    }
    case NODE_STMTS_LIST: {
        SharedPtr<ListNode> ln = node->upcast<ListNode>();
        for (int i=0; i<ln->size(); i++) {
            this->compile(ln->at(i));
        }
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
        auto if_node = node->upcast<IfNode>();
        this->compile(if_node->cond);

        SharedPtr<OP> jump_else = new OP;
        jump_else->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_else);

        this->compile(if_node->if_body);

        SharedPtr<OP> jump_end = new OP;
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
        this->compile(node->upcast<BinaryNode>()->left); // cond

        SharedPtr<OP> jump_if_false = new OP;
        jump_if_false->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_if_false);

        this->compile(node->upcast<BinaryNode>()->right); //body

        SharedPtr<OP> goto_ = new OP;
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
        int global = this->find_global_var(node->upcast<StrNode>()->str_value);
        if (global >= 0) {
            SharedPtr<OP> tmp = new OP();
            tmp->op_type = OP_GETGLOBAL;
            tmp->operand.int_value = global;
            ops->push_back(tmp);
        } else {
            // find local variable
            int level;
            int no = this->find_localvar(std::string(node->upcast<StrNode>()->str_value), level);
            if (no<0) {
                fprintf(stderr, "There is no variable named '%s'(NODE_GETVARIABLE)\n", node->upcast<StrNode>()->str_value.c_str());
                this->error++;
                return;
            }

            if (level == 0) {
                DBG2("LOCAL\n");
                SharedPtr<OP> tmp = new OP;
                tmp->op_type = OP_GETLOCAL;
                tmp->operand.int_value = no;
                ops->push_back(tmp);
            } else {
                SharedPtr<OP> tmp = new OP;
                DBG2("DYNAMIC\n");
                tmp->op_type = OP_GETDYNAMIC;
                tmp->operand.int_value = (((level)&0x0000ffff) << 16) | (no&0x0000ffff);
                ops->push_back(tmp);
            }
        }
        break;
    }
    case NODE_DIV_ASSIGN: {
        // '$x /= 3;' => '$x = $x / 3'
        // TODO: optimize
        SharedPtr<BinaryNode> r = new BinaryNode(NODE_DIV,
            node->upcast<BinaryNode>()->left,
            node->upcast<BinaryNode>()->right
        );
        SharedPtr<BinaryNode> p = new BinaryNode(NODE_SETVARIABLE, &(*(node->upcast<BinaryNode>()->left)), &(*(r->upcast<Node>())));
        this->compile(p);
        break;
    }
    case NODE_SETVARIABLE: {
        this->compile(node->upcast<BinaryNode>()->right);
        this->set_lvalue(node->upcast<BinaryNode>()->left);
        break;
    }
    case NODE_SETVARIABLE_MULTI: {
        this->compile(node->upcast<BinaryNode>()->right);

        SharedPtr<ListNode>ln = node->upcast<BinaryNode>()->left->upcast<ListNode>();

        // extract
        OP* op = new OP(OP_EXTRACT_TUPLE);
        op->operand.int_value = ln->size();
        ops->push_back(op);

        // and set to variables
        // TODO use set_lvalue method?
        for (size_t i=0; i < ln->size(); i++) {
            // ($a, $b) = foo();
            // ($a[0], $b) = foo();
            switch (ln->at(i)->type) {
                case NODE_GETVARIABLE: { // $a = $b;
                    std::string varname = ln->at(i)->upcast<StrNode>()->str_value;
                    this->set_variable(varname);
                    break;
                }
                case NODE_GET_ITEM: { // $a[$b] = $c
                    auto container = ln->at(i)->upcast<BinaryNode>()->left;
                    auto index     = ln->at(i)->upcast<BinaryNode>()->right;
                    this->compile(container);
                    this->compile(index);
                    ops->push_back(new OP(OP_SET_ITEM));
                    break;
                }
                default: {
                    fprintf(stderr, "Compilation failed\n");
                    this->error++;
                    break;
                }
            }
            ops->push_back(new OP(OP_POP_TOP));
        }

        break;
    }
    case NODE_MAKE_ARRAY: {
        auto args = node->upcast<ArgsNode>()->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_MAKE_ARRAY;
        tmp->operand.int_value = args_len; // the number of args
        ops->push_back(tmp);
        break;
    }
    case NODE_MAKE_HASH: {
        auto args = node->upcast<ArgsNode>()->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_MAKE_HASH;
        tmp->operand.int_value = args_len; // the number of args
        ops->push_back(tmp);
        break;
    }
    case NODE_GET_ITEM: {
        this->compile(node->upcast<BinaryNode>()->left);  // container
        this->compile(node->upcast<BinaryNode>()->right); // index

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_GET_ITEM;
        ops->push_back(tmp);
        break;
    }

    case NODE_UNARY_NEGATIVE: {
        this->compile(node->upcast<NodeNode>()->node);

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

        this->compile(node->upcast<ForNode>()->initialize);
        int label1 = ops->size();
        this->compile(node->upcast<ForNode>()->cond);

        SharedPtr<OP> jump_label2 = new OP;
        jump_label2->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_label2);

        this->compile(node->upcast<ForNode>()->block);
        this->compile(node->upcast<ForNode>()->postfix);

        SharedPtr<OP> jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        ops->push_back(jump_label1);

        int label2 = ops->size();
        jump_label2->operand.int_value = label2;
        break;
    }
    case NODE_FOREACH: {
        /*
        struct {
            Node *vars;
            Node *source;
            Node *block;
        } for_stmt;

        for ($x IN $array) { ... }

            GET_ITER source
        LABEL1:
            FOR_ITER iter
            jump_if_stop LABEL2
            STORE $x
            (block)
            goto LABEL1
        LABEL2:
        */

        this->compile(node->upcast<ForEachNode>()->source);
        ops->push_back(new OP(OP_GET_ITER));

        size_t label1 = ops->size();

        ops->push_back(new OP(OP_FOR_ITER));
        SharedPtr<OP> jump_label2 = new OP();
        jump_label2->op_type = OP_JUMP_IF_STOP_EXCEPTION;
        ops->push_back(jump_label2); // FIX ME?

        // store variables
        this->set_lvalue(node->upcast<ForEachNode>()->vars);

        this->compile(node->upcast<ForEachNode>()->block);

        SharedPtr<OP> jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        ops->push_back(jump_label1);

        size_t label2 = ops->size();
        jump_label2->operand.int_value = label2;

        break;
    }
    case NODE_METHOD_CALL: {
        /*
        node->method_call.object = $1;
        node->method_call.method = $3;
        node->method_call.args   = $5;
        */
        auto mcn = node->upcast<MethodCallNode>();
        auto args = mcn->args;
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }
        this->compile(mcn->method);
        this->compile(mcn->object);

        SharedPtr<OP> op = new OP(OP_METHOD_CALL);
        op->operand.int_value = args_len;
        ops->push_back(op);
        break;
    }
    case NODE_UNARY_INCREMENT: {
        // ++$i
        this->compile(node->upcast<NodeNode>()->node);
        ops->push_back(new OP(OP_UNARY_INCREMENT));
        break;
    }

    default:
        this->error++;
        printf("Unknown node: %s\n", node->type_name_str());
        abort();
        break;
    }
}
