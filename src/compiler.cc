#include "nodes.gen.h"
#include "node.h"
#include "compiler.h"
#include "value/code.h"
#include "value/symbol.h"
#include "value/regexp.h"
#include "disasm.h"
#include <boost/scope_exit.hpp>
#include <boost/foreach.hpp>

using namespace tora;

class LoopContext {
    Compiler *compiler_;
    bool orig;
public:
    LoopContext(Compiler *c, bool b) {
        compiler_ = c;
        orig = compiler_->in_loop_context;
        compiler_->in_loop_context = b;
    }
    ~LoopContext() {
        compiler_->in_loop_context = orig;
    }
};

/**
 * Count up the number of variables declared at this scope.
 */
static int count_variable_declare(const SharedPtr<Node> &node) {
    if (node->type == NODE_MY) {
        SharedPtr<ListNode>ln = node->upcast<ListNode>();
        return ln->size();
    } else {
        auto iter = node->list->begin();
        int ret = 0;
        for (; iter!=node->list->end(); iter++) {
            if ((*iter).get()) {
                ret += count_variable_declare(*iter);
            }
        }
        return ret;
    }
}

void Compiler::define_my(SharedPtr<Node> node) {
    SharedPtr<ListNode>ln = node->upcast<ListNode>();
    for (size_t i=0; i < ln->size(); i++) {
        auto target = ln->at(i);
        switch (target->type) {
        case NODE_GETVARIABLE: {
            std::string &name = ln->at(i)->upcast<StrNode>()->str_value;
            this->define_localvar(name);
            ops->push_back(new OP(OP_PUSH_UNDEF));
            break;
        }
        case NODE_TUPLE: {
            this->define_my(target);
            break;
        }
        default:
            fail("This is not a variable node: %s\n", target->type_name_str());
            target->dump();
            return;
        }
    }
}

int tora::Compiler::find_localvar(std::string name, int &level, bool &need_closure) {
    DBG("FIND LOCAL VAR %d\n", 0);
    need_closure = false;
    int seen_funcdef = -1;
    for (level = 0; level<this->blocks->size(); level++) {
        SharedPtr<Block> block = this->blocks->at(this->blocks->size()-1-level);
        if (block->type == BLOCK_TYPE_FUNCDEF && seen_funcdef == -1) {
            seen_funcdef = level;
        }
        for (size_t i=0; i<block->vars.size(); i++) {
            if (*(block->vars.at(i)) == name) {
                if (seen_funcdef != -1 && seen_funcdef < level) {
                    need_closure = true;
                }
                return i;
            }
        }
    }
    return -1;
}

void tora::Compiler::init_globals() {
    this->define_global_var("$ARGV");
    this->define_global_var("$ENV");
    this->define_global_var("$LIBPATH");
    this->define_global_var("$REQUIRED");
}

void tora::Compiler::set_variable(std::string &varname) {
    int level;
    bool need_closure;
    int no = this->find_localvar(varname, level, need_closure);
    if (no<0) {
        fprintf(stderr, "There is no variable named %s(SETVARIABLE)\n", varname.c_str());
        this->error++;
        return;
    }

    SharedPtr<OP> tmp = new OP;
    if (need_closure) {
        DBG2("LOCAL\n");
        tmp->op_type = OP_SETCLOSURE;
        tmp->operand.int_value = -1;
        auto iter = closure_vars->begin();
        for (; iter!=closure_vars->end(); iter++) {
            if (*iter == varname) {
                tmp->operand.int_value = no;
                break;
            }
        }
        if (tmp->operand.int_value == -1) {
            closure_vars->push_back(varname);
            tmp->operand.int_value = closure_vars->size()-1;
        }
    } else {
        if (level == 0) {
            DBG2("LOCAL\n");
            tmp->op_type = OP_SETLOCAL;
            tmp->operand.int_value = no;
        } else {
            DBG2("DYNAMIC\n");
            tmp->op_type = OP_SETDYNAMIC;
            tmp->operand.int_value = (((level)&0x0000ffff) << 16) | (no&0x0000ffff);
        }
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
        auto container = node->upcast<BinaryNode>()->left();
        auto index     = node->upcast<BinaryNode>()->right();
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
                    auto container = ln->at(i)->upcast<BinaryNode>()->left();
                    auto index     = ln->at(i)->upcast<BinaryNode>()->right();
                    this->compile(container);
                    this->compile(index);
                    ops->push_back(new OP(OP_SET_ITEM));
                    break;
                }
                default: {
                    fprintf(stderr, "\nCompilation failed. This is not a variable node:\n");
                    ln->at(i)->dump(1);
                    this->error++;
                    break;
                }
            }
            ops->push_back(new OP(OP_POP_TOP));
        }
        break;
    }
    case NODE_MY: {
        SharedPtr<ListNode>ln = node->upcast<ListNode>();
        for (size_t i=0; i < ln->size(); i++) {
            if (ln->at(i)->type == NODE_TUPLE) {
                SharedPtr<ListNode> ln2 = ln->at(i)->upcast<ListNode>();
                for (size_t i=0; i < ln2->size(); i++) {
                    std::string &name = ln2->at(i)->upcast<StrNode>()->str_value;
                    this->define_localvar(name);
                }
            } else {
                std::string &name = ln->at(i)->upcast<StrNode>()->str_value;
                this->define_localvar(name);
            }
        }
        if (ln->size() == 1) {
            this->set_lvalue(ln->at(0));
        } else {
            printf("This is not lvalue(MY):\n");
            node->dump(1);
            this->error++;
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

class OptimizableEnterScope {
    Compiler *compiler_;
    OP *enter;
public:
    OptimizableEnterScope(Compiler *compiler, const SharedPtr<Node> &node) {
        int decvar_in_cond = count_variable_declare(node);
        compiler_ = compiler;
        if (decvar_in_cond) {
            enter = new OP(OP_ENTER);
            compiler->ops->push_back(enter);
            compiler->push_block(BLOCK_TYPE_BLOCK);
        } else {
            enter = NULL;
        }
    }
    ~OptimizableEnterScope() {
        if (enter) {
            compiler_->ops->push_back(new OP(OP_LEAVE));
            enter->operand.int_value = compiler_->blocks->back()->vars.size();
            compiler_->pop_block();
        }
    }
};

bool tora::Compiler::is_builtin(const std::string &s) {
    const char *bs[] = {
        "say",
        "__PACKAGE__",
        "p",
        "require",
        "eval",
        "do",
        "print",
        "self",
        "opendir",
        "typeof",
        "rand",
        "open",
        "exit",
        "dump_stack",
        NULL
    };
    for (int i=0; bs[i]; i++) {
        if (s == bs[i]) {
            return true;
        }
    }
    return false;
}

void tora::Compiler::compile(SharedPtr<Node> node) {
    switch (node->type) {
    case NODE_ROOT: {
        this->push_block(BLOCK_TYPE_FILE);
        ID package_id = this->symbol_table->get_id(this->package());
        OP *pkg = new OP(OP_PACKAGE_ENTER);
        pkg->operand.int_value = package_id;
        ops->push_back(pkg);

        this->push_block(BLOCK_TYPE_BLOCK);
        OP *enter = new OP(OP_ENTER);
        ops->push_back(enter);

        this->compile(node->upcast<NodeNode>()->node());

        ops->push_back(new OP(OP_END));

        enter->operand.int_value = this->blocks->back()->vars.size();
        this->pop_block();

        ops->push_back(new OP(OP_PACKAGE_LEAVE));
        this->pop_block();

        break;
    }
    case NODE_MY: {
        this->define_my(node);
        break;
    }
    case NODE_RETURN: {
        SharedPtr<ListNode>ln = node->upcast<ListNode>();

        if (this->in_try_block) {
            ln->list->insert(ln->list->begin(), new VoidNode(NODE_UNDEF));
        }

        if (ln->size() == 1) {
            this->compile(ln->at(0));
            ops->push_back(new OP(OP_RETURN));
        } else if (ln->size() == 0) {
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
        {
            OptimizableEnterScope es(this, node->upcast<NodeNode>()->node());
            this->compile(node->upcast<NodeNode>()->node());
        }

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

        Compiler::TryGuard guard(this, false);

        auto funcdef_node = node->upcast<FuncdefNode>();

        // function name
        std::string &funcname = funcdef_node->name()->upcast<StrNode>()->str_value;

        this->push_block(BLOCK_TYPE_FUNCDEF);
        auto params = new std::vector<std::string *>();
        for (size_t i=0; i<funcdef_node->params()->size(); i++) {
            params->push_back(new std::string(funcdef_node->params()->at(i)->upcast<StrNode>()->str_value));
            this->define_localvar(std::string(funcdef_node->params()->at(i)->upcast<StrNode>()->str_value));
        }

        Compiler funccomp(this->symbol_table);
        if (funccomp.blocks) {
            delete funccomp.blocks;
            funccomp.blocks = NULL;
        }
        funccomp.package(this->package());
        funccomp.blocks = new std::vector<SharedPtr<Block>>(*(this->blocks));
        funccomp.compile(funcdef_node->block());
        this->pop_block();

        funccomp.ops->push_back(new OP(OP_PUSH_UNDEF));
        funccomp.ops->push_back(new OP(OP_RETURN));
        if (this->dump_ops) {
            Disasm::disasm(funccomp.ops);
        }

        // printf("CLOSURE VARS: %d\n", funccomp.closure_vars->size());

        SharedPtr<CodeValue> code = new CodeValue();
        code->package_id = this->symbol_table->get_id(this->package());
        code->func_name_id = this->symbol_table->get_id(funcname);
        code->code_id = this->symbol_table->get_id(this->package() + "::" + funcname);
        code->code_params = params;
        code->code_opcodes = funccomp.ops;
        code->closure_var_names = new std::vector<std::string>(*funccomp.closure_vars);

        SharedPtr<StrValue> funcname_value = new StrValue(funcname);
        if (funccomp.closure_vars->size() > 0) {
            // create closure
            // push variables  to stack.
            auto iter = funccomp.closure_vars->begin();
            for (; iter!=funccomp.closure_vars->end(); iter++) {
                this->compile(new StrNode(NODE_GETVARIABLE, *iter));
            }

            SharedPtr<ValueOP> putval = new ValueOP(OP_PUSH_VALUE, code);
            ops->push_back(putval);

            // define method.
            SharedPtr<OP> op = new OP(OP_CLOSUREDEF);
            op->operand.int_value = funccomp.closure_vars->size();
            ops->push_back(op);
        } else {
            SharedPtr<ValueOP> putval = new ValueOP(OP_PUSH_VALUE, code);
            ops->push_back(putval);

            // create normal function
            SharedPtr<OP> define_method = new OP(OP_FUNCDEF);
            ops->push_back(define_method);
        }

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
        this->compile(node->upcast<BinaryNode>()->right());
        this->compile(node->upcast<BinaryNode>()->left());
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
    case NODE_UNDEF: {
        ops->push_back(new OP(OP_PUSH_UNDEF));
        break;
    }
    case NODE_FALSE: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        ops->push_back(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        ID id = this->symbol_table->get_id(node->upcast<StrNode>()->str_value);
        SharedPtr<OP> o = new OP(OP_PUSH_IDENTIFIER);
        o->operand.int_value = id;
        ops->push_back(o);
        break;
    }

    case NODE_FUNCALL: {
        LoopContext lc(this, false);

        auto args = node->upcast<FuncallNode>()->args();
        int args_len = args->size();
        while (args->size() > 0) {
            assert(args->back());
            this->compile(args->back());
            args->pop_back();
        }

        std::string funcname = node->upcast<FuncallNode>()->name()->upcast<StrNode>()->str_value;

        if (this->is_builtin(funcname)) {
            ID id = this->symbol_table->get_id(node->upcast<FuncallNode>()->name()->upcast<StrNode>()->str_value);
            SharedPtr<ValueOP> o = new ValueOP(OP_PUSH_VALUE, new SymbolValue(id));
            ops->push_back(o);
            SharedPtr<OP> tmp = new OP;
            tmp->op_type = OP_BUILTIN_FUNCALL;
            tmp->operand.int_value = args_len; // the number of args
            ops->push_back(tmp);
        } else {
            /*
            std::string funcname2 = funcname.find("::")==std::string::npos
                ? this->package() + "::" + funcname
                : funcname;
            */
            std::string funcname2 = funcname;
            ID id = this->symbol_table->get_id(funcname2);
            SharedPtr<ValueOP> o = new ValueOP(OP_PUSH_VALUE, new SymbolValue(id));
            ops->push_back(o);
            SharedPtr<OP> tmp = new OP;
            tmp->op_type = OP_FUNCALL;
            tmp->operand.int_value = args_len; // the number of args
            ops->push_back(tmp);
        }
        break;
    }
#define C_OP_BINARY(type) \
    { \
        this->compile(node->upcast<BinaryNode>()->left()); \
        this->compile(node->upcast<BinaryNode>()->right()); \
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
    case NODE_NE:  C_OP_BINARY(OP_NE);
#undef C_OP_BINARY

    // TODO: deprecate?
    case NODE_STMTS: {
        ops->push_back(new OP(OP_NEXTSTATE));
        this->compile(node->upcast<BinaryNode>()->left());
        ops->push_back(new OP(OP_NEXTSTATE));
        this->compile(node->upcast<BinaryNode>()->right());
        break;
    }
    case NODE_STMTS_LIST: {
        SharedPtr<ListNode> ln = node->upcast<ListNode>();
        for (int i=0; i<ln->size(); i++) {
            ops->push_back(new OP(OP_NEXTSTATE));
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

        OptimizableEnterScope oes(this, if_node->cond());

        {

            this->compile(if_node->cond());

            SharedPtr<OP> jump_else = new OP;
            jump_else->op_type = OP_JUMP_IF_FALSE;
            ops->push_back(jump_else);

            if (if_node->if_body()) {
                this->compile(if_node->if_body());
            }

            SharedPtr<OP> jump_end = new OP;
            jump_end->op_type = OP_JUMP;
            ops->push_back(jump_end);

            int else_label = ops->size();
            jump_else->operand.int_value = else_label;
            if (if_node->else_body()) {
                this->compile(if_node->else_body());
            }

            int end_label = ops->size();
            jump_end->operand.int_value = end_label;

        }

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
        this->compile(node->upcast<BinaryNode>()->left()); // cond

        LoopContext lc(this, true);

        SharedPtr<OP> jump_if_false = new OP;
        jump_if_false->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_if_false);

        OP *enter = new OP(OP_ENTER_WHILE);
        ops->push_back(enter);
        this->push_block(BLOCK_TYPE_BLOCK);

            this->compile(node->upcast<BinaryNode>()->right()); //body

        ops->push_back(new OP(OP_LEAVE));
        enter->operand.int_value = this->blocks->back()->vars.size();
        this->pop_block();

        SharedPtr<OP> goto_ = new OP;
        goto_->op_type = OP_JUMP;
        goto_->operand.int_value = label1;
        ops->push_back(goto_);

        int label2 = ops->size();
        jump_if_false->operand.int_value = label2;

        BOOST_FOREACH(auto n, last_labels) {
            *n = label2;
        }
        last_labels.clear();

        break;
    }
    case NODE_VOID:
        // nop
        break;
    case NODE_NEWLINE:
        // nop
        break;
    case NODE_GETVARIABLE: {
        std::string & varname = node->upcast<StrNode>()->str_value;
        int global = this->find_global_var(varname);
        if (global >= 0) {
            SharedPtr<OP> tmp = new OP();
            tmp->op_type = OP_GETGLOBAL;
            tmp->operand.int_value = global;
            ops->push_back(tmp);
        } else {
            // find local variable
            int level;
            bool need_closure;
            int no = this->find_localvar(std::string(node->upcast<StrNode>()->str_value), level, need_closure);
            // dump_localvars();
            if (no<0) {
                fprintf(stderr, "There is no variable named '%s'(NODE_GETVARIABLE)\n", node->upcast<StrNode>()->str_value.c_str());
                this->error++;
                return;
            }

            if (need_closure) {
                SharedPtr<OP> tmp = new OP;
                tmp->op_type = OP_GETCLOSURE;
                tmp->operand.int_value = -1;
                auto iter = closure_vars->begin();
                for (; iter!=closure_vars->end(); iter++) {
                    if (*iter == varname) {
                        tmp->operand.int_value = no;
                        break;
                    }
                }
                if (tmp->operand.int_value == -1) {
                    closure_vars->push_back(varname);
                    tmp->operand.int_value = closure_vars->size()-1;
                }
                ops->push_back(tmp);
            } else {
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
        }
        break;
    }
    case NODE_DIV_ASSIGN: {
        // '$x /= 3;' => '$x = $x / 3'
        // TODO: optimize
        SharedPtr<BinaryNode> r = new BinaryNode(NODE_DIV,
            node->upcast<BinaryNode>()->left(),
            node->upcast<BinaryNode>()->right()
        );
        SharedPtr<BinaryNode> p = new BinaryNode(NODE_SETVARIABLE, &(*(node->upcast<BinaryNode>()->left())), &(*(r->upcast<Node>())));
        this->compile(p);
        break;
    }
    case NODE_SETVARIABLE: {
        this->compile(node->upcast<BinaryNode>()->right());
        this->set_lvalue(node->upcast<BinaryNode>()->left());
        break;
    }
    case NODE_SETVARIABLE_MULTI: {
        this->compile(node->upcast<BinaryNode>()->right());

        SharedPtr<ListNode>ln = node->upcast<BinaryNode>()->left()->upcast<ListNode>();

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
                    auto container = ln->at(i)->upcast<BinaryNode>()->left();
                    auto index     = ln->at(i)->upcast<BinaryNode>()->right();
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
        auto args = node->upcast<ListNode>();
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
        auto args = node->upcast<ListNode>();
        int args_len = args->size();
        for (int i=0; i<args_len; i+=2) {
            this->compile(args->at(i+1));
            this->compile(args->at(i));
        }

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_MAKE_HASH;
        tmp->operand.int_value = args_len; // the number of args
        ops->push_back(tmp);
        break;
    }
    case NODE_GET_ITEM: {
        this->compile(node->upcast<BinaryNode>()->left());  // container
        this->compile(node->upcast<BinaryNode>()->right()); // index

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_GET_ITEM;
        ops->push_back(tmp);
        break;
    }

    case NODE_UNARY_NEGATIVE: {
        this->compile(node->upcast<NodeNode>()->node());

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

            ENTER_FOR
            (initialize)
        LABEL1:
            (cond)
            jump_if_false LABEL2
            (block)
            (postfix)
            goto LABEL1
        LABEL2:
            LEAVE
        */

        OP *enter = new OP(OP_ENTER_FOR);
        ops->push_back(enter);
        this->push_block(BLOCK_TYPE_BLOCK);

        this->compile(node->upcast<ForNode>()->initialize());
        int label1 = ops->size();
        this->compile(node->upcast<ForNode>()->cond());

        SharedPtr<OP> jump_label2 = new OP;
        jump_label2->op_type = OP_JUMP_IF_FALSE;
        ops->push_back(jump_label2);

        {
            LoopContext lc(this, true);
            this->compile(node->upcast<ForNode>()->block());
        }

        this->compile(node->upcast<ForNode>()->postfix());

        SharedPtr<OP> jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        ops->push_back(jump_label1);

        int label2 = ops->size();
        jump_label2->operand.int_value = label2;

        ops->push_back(new OP(OP_LEAVE));
        enter->operand.int_value = this->blocks->back()->vars.size();
        this->pop_block();

        BOOST_FOREACH(auto n, last_labels) {
            *n = label2;
        }
        last_labels.clear();

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

        LoopContext lc(this, true);
        this->compile(node->upcast<ForEachNode>()->source());

        this->push_block(BLOCK_TYPE_BLOCK);
        OP *enter_foreach = new OP(OP_ENTER_FOREACH);
        ops->push_back(enter_foreach);

        size_t label1 = ops->size();

        ops->push_back(new OP(OP_FOR_ITER));
        SharedPtr<OP> jump_label2 = new OP();
        jump_label2->op_type = OP_JUMP_IF_STOP_EXCEPTION;
        ops->push_back(jump_label2); // FIX ME?

        // store variables
        this->set_lvalue(node->upcast<ForEachNode>()->vars());

        this->compile(node->upcast<ForEachNode>()->block());

        SharedPtr<OP> jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        ops->push_back(jump_label1);

        size_t label2 = ops->size();
        jump_label2->operand.int_value = label2;

        enter_foreach->operand.int_value = this->blocks->back()->vars.size();
        this->pop_block();

        ops->push_back(new OP(OP_LEAVE));

        BOOST_FOREACH(auto n, last_labels) {
            *n = label2;
        }
        last_labels.clear();

        break;
    }
    case NODE_METHOD_CALL: {
        /*
        node->method_call.object = $1;
        node->method_call.method = $3;
        node->method_call.args   = $5;
        */

        LoopContext lc(this, false);

        auto mcn = node->upcast<MethodCallNode>();
        auto args = mcn->args();
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }
        if (mcn->method()->type == NODE_IDENTIFIER) {
            ID id = this->symbol_table->get_id(mcn->method()->upcast<StrNode>()->str_value);
            SharedPtr<ValueOP> o = new ValueOP(OP_PUSH_VALUE, new SymbolValue(id));
            ops->push_back(o);
        } else {
            fprintf(stderr, "Compilation error. This is not a id.\n");
            error++;
            break;
        }
        this->compile(mcn->object());

        SharedPtr<OP> op = new OP(OP_METHOD_CALL);
        op->operand.int_value = args_len;
        ops->push_back(op);
        break;
    }
    case NODE_NOT: {
        // ! $val
        this->compile(node->upcast<NodeNode>()->node());
        ops->push_back(new OP(OP_NOT));
        break;
    }
    case NODE_FILE_TEST: {
        // -f $file
        OP *op = new OP(OP_FILE_TEST);
        op->operand.int_value = node->upcast<BinaryNode>()->left()->upcast<IntNode>()->int_value;
        this->compile(node->upcast<BinaryNode>()->right());
        ops->push_back(op);
        break;
    }
    case NODE_POST_DECREMENT: {
        // $i--
        this->compile(node->upcast<NodeNode>()->node());
        ops->push_back(new OP(OP_POST_DECREMENT));
        break;
    }
    case NODE_PRE_DECREMENT: {
        // --$i
        this->compile(node->upcast<NodeNode>()->node());
        ops->push_back(new OP(OP_PRE_DECREMENT));
        break;
    }
    case NODE_POST_INCREMENT: {
        // $i++
        this->compile(node->upcast<NodeNode>()->node());
        ops->push_back(new OP(OP_POST_INCREMENT));
        break;
    }
    case NODE_PRE_INCREMENT: {
        // ++$i
        this->compile(node->upcast<NodeNode>()->node());
        ops->push_back(new OP(OP_PRE_INCREMENT));
        break;
    }
    case NODE_TUPLE: {
        SharedPtr<ListNode>ln = node->upcast<ListNode>();
        for (size_t i=0; i < ln->size(); i++) {
            this->compile(ln->at(i));
        }
        OP *op = new OP(OP_MAKE_TUPLE);
        op->operand.int_value = ln->size();
        ops->push_back(op);
        break;
    }
    case NODE_DOTDOTDOT: {
        ops->push_back(new OP(OP_DOTDOTDOT));
        break;
    }
    case NODE_TRY: {
        LoopContext lc(this, false);

        Compiler::TryGuard guard(this, true);

        OP *try_op = new OP(OP_TRY);
        ops->push_back(try_op);

        this->push_block(BLOCK_TYPE_TRY);
        OP *enter_op = new OP(OP_ENTER);
        ops->push_back(enter_op);

        SharedPtr<NodeNode> n = node->upcast<NodeNode>();
        this->compile(n->node());

        ops->push_back(new OP(OP_PUSH_UNDEF));
        ops->push_back(new OP(OP_PUSH_UNDEF));
        OP *op = new OP(OP_MAKE_TUPLE);
        op->operand.int_value = 2;
        ops->push_back(op);
        ops->push_back(new OP(OP_RETURN));

        enter_op->operand.int_value = this->blocks->back()->vars.size();

        this->pop_block();

        try_op->operand.int_value = ops->size();

        break;
    }
    case NODE_DIE: {
        this->compile(node->upcast<NodeNode>()->node());
        ops->push_back(new OP(OP_DIE));
        break;
    }
    case NODE_USE: {
        // use Test::More;           # => 0
        // use Test::More *;         # => 1
        // use Test::More qw(ok is); # => 1
        this->compile(node->upcast<BinaryNode>()->right());
        this->compile(node->upcast<BinaryNode>()->left());
        ops->push_back(new OP(OP_USE));
        break;
    }

    case NODE_CLASS: {
        SharedPtr<ClassNode> n = node->upcast<ClassNode>();

        if (this->in_class_context) {
            // allow nested class?
            this->fail("You cannot nest classes.");
            abort();
        }
        this->in_class_context = true;

        this->push_block(BLOCK_TYPE_CLASS);
        std::string klass_name = n->klass()->upcast<StrNode>()->str_value;
        ID package_id = this->symbol_table->get_id(
              this->package() == "main"
            ? klass_name
            : this->package() + "::" + klass_name
        );
        OP *pkg = new OP(OP_PACKAGE_ENTER);
        pkg->operand.int_value = package_id;
        ops->push_back(pkg);

        if (n->block() != NULL) {
            this->compile(n->block());
        }

        ops->push_back(new OP(OP_PACKAGE_LEAVE));
        this->pop_block();

        this->in_class_context = false;

        break;
    }
    case NODE_LAST: {
        // break from for/foreach/while
        if (!in_loop_context) {
            fail("You can't put \"last\" statement out of loop block.\n");
        } else {
            OP * op = new OP(OP_LAST);
            this->last_labels.push_back(&(op->operand.int_value));
            ops->push_back(op);
        }
        break;
    }

    default:
        this->error++;
        printf("Unknown node: %s\n", node->type_name_str());
        abort();
        break;
    }
}
