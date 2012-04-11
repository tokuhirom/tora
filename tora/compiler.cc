#include "compiler.h"
#include "op_array.h"
#include "nodes.gen.h"
#include "node.h"
#include "value/code.h"
#include "value/symbol.h"
#include "value/regexp.h"
#include "symbol_table.h"
#include "disasm.h"
#include <boost/scope_exit.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

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

void Compiler::dump_localvars() {
    printf("-- dump_localvars --\n");
    printf("Levels: %ld\n", (long int) this->blocks->size());
    for (size_t level = 0; level < this->blocks->size(); ++level) {
        Block & block = this->blocks->at(level);
        printf("[%ld] %d\n", (long int) level, block.type);
        for (size_t i=0; i<block.vars.size(); i++) {
            printf("    %s\n", block.vars.at(i).c_str());
        }
    }
    printf("--------------------\n");
}

/**
 * Count up the number of variables declared at this scope.
 */
static int count_variable_declare(const SharedPtr<Node> &node) {
    if (node->type == NODE_MY) {
        SharedPtr<Node>ln = node->upcast<Node>();
        return ln->size();
    } else if (node->type == NODE_LOCAL) {
        return 1;
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

/**
 * make the node to string node if it's bareword.
 *
 * $foo[word]       =>    $foo['word']
 * $foo[word] = 3   =>    $foo['word'] = 3
 * {foo => 'bar'}   =>    {'foo' => 'bar'}
 */
inline static SharedPtr<Node> STRING_IF_BAREWORD(SharedPtr<Node>& node) {
    if (node->type == NODE_INSTANCIATE_IDENTIFIER) {
        node->type = NODE_STRING;
        return node;
    } else {
        return node;
    }
}

Compiler::Compiler(const SharedPtr<SymbolTable> &symbol_table_, const std::string &filename) : filename_(filename), in_class_context(false) {
    error = 0;
    blocks = new std::vector<Block>();
    global_vars = new std::vector<std::string>();
    ops = new OPArray();
    in_try_block = false;
    symbol_table = symbol_table_;
    dump_ops = false;
    package_ = "main";
    closure_vars.reset(new std::vector<std::string>());
    in_loop_context = false;
}
Compiler::~Compiler() {
    delete global_vars;
    delete blocks;
}

void Compiler::push_op(OP * op) {
    assert(op);
    assert(this->current_node);
    this->ops->push_back(op, this->current_node->lineno);
}

void Compiler::define_my(SharedPtr<Node> node) {
    SharedPtr<Node>ln = node->upcast<Node>();
    for (size_t i=0; i < ln->size(); i++) {
        auto target = ln->at(i);
        switch (target->type) {
        case NODE_GETVARIABLE: {
            std::string &name = ln->at(i)->upcast<StrNode>()->str_value;
            this->define_localvar(name);
            push_op(new OP(OP_PUSH_UNDEF));
            this->set_variable(name);
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

/**
 * Find local vars level in compilation phase.
 *
 * @return $n > 0 when found var. return -1 when var is not found.
 */
int tora::Compiler::find_localvar(std::string name, int &level, bool &need_closure, bool &is_arg, int &funcdef_level) {
    DBG("FIND LOCAL VAR %d\n", 0);
    need_closure = false;
    int seen_funcdef = -1;
    for (level = 0; level<this->blocks->size(); ++level) {
        Block & block = this->blocks->at(this->blocks->size()-1-level);
        if (block.type == BLOCK_TYPE_FUNCDEF && seen_funcdef == -1) {
            seen_funcdef = level;
        }
        for (size_t i=0; i<block.vars.size(); i++) {
            if (block.vars.at(i) == name) {
                if (seen_funcdef != -1 && seen_funcdef < level) {
                    funcdef_level = seen_funcdef;
                    need_closure = true;
                }
                if (block.type == BLOCK_TYPE_FUNCDEF) {
                    is_arg = true;
                } else {
                    is_arg = false;
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
    this->define_global_var("$STDIN");
    this->define_global_var("$STDOUT");
    this->define_global_var("$STDERR");
}


void tora::Compiler::set_variable(std::string &varname) {
    {
        auto pos = varname.rfind("::");
        if (pos != std::string::npos) {
            // package variable $Foo::Bar.
            std::string pkgname = varname.substr(1, pos-1);
            std::string detail = varname.substr(pos+2);
            push_op(new OP(OP_PUSH_IDENTIFIER, symbol_table->get_id(pkgname)));
            push_op(new OP(OP_SET_PACKAGE_VARIABLE, symbol_table->get_id(std::string("$") + detail)));
            return;
        }
    }

    int level;
    bool need_closure;
    bool is_arg;
    int funcdef_level;
    int no = this->find_localvar(varname, level, need_closure, is_arg, funcdef_level);
    if (no<0) {
        fprintf(stderr, "There is no variable named %s(SETVARIABLE)\n", varname.c_str());
        this->error++;
        return;
    }

    if (1 && need_closure) {
        DBG2("LOCAL\n");
#ifdef PERLISH_CLOSURE
        int pos = -1;
        auto iter = closure_vars->begin();
        for (; iter!=closure_vars->end(); iter++) {
            if (*iter == varname) {
                pos = no;
                break;
            }
        }
        if (pos == -1) {
            closure_vars->push_back(varname);
            pos = closure_vars->size()-1;
        }
        push_op(new OP(OP_SETCLOSURE, pos));
#else
        push_op(new OP(OP_SETCLOSURE, level-funcdef_level, no));
        closure_vars->push_back(varname);
#endif
    } else {
        if (is_arg) {
            // SharedPtr<OP> tmp = new OP(OP_SETARG, no);
            push_op(new OP(OP_SETARG, no));
        } else if (level == 0) {
            DBG2("LOCAL\n");
            push_op(new OP(OP_SETLOCAL, no));
        } else {
            DBG2("DYNAMIC\n");
            push_op(new OP(OP_SETDYNAMIC, level, no));
        }
    }
}

/**
 * assign Top of Stack to lvalue.
 *
 * @args node: lvalue node.
 * @return none.
 */
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
        this->compile(STRING_IF_BAREWORD(index));

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_SET_ITEM;
        push_op(tmp);
        break;
    }
    case NODE_TUPLE: { // ($a, $b, $c[0]) = $d
        SharedPtr<Node>ln = node->upcast<Node>();

        // extract
        OP* op = new OP(OP_EXTRACT_TUPLE);
        op->operand.int_value = ln->size();
        push_op(op);

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
                    this->compile(STRING_IF_BAREWORD(index));
                    push_op(new OP(OP_SET_ITEM));
                    break;
                }
                default: {
                    fprintf(stderr, "\nCompilation failed. This is not a variable node:\n");
                    ln->at(i)->dump(1);
                    this->error++;
                    break;
                }
            }
            push_op(new OP(OP_POP_TOP));
        }
        break;
    }
    case NODE_MY: {
        SharedPtr<Node>ln = node->upcast<Node>();
        for (size_t i=0; i < ln->size(); i++) {
            if (ln->at(i)->type == NODE_TUPLE) {
                SharedPtr<Node> ln2 = ln->at(i)->upcast<Node>();
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
    case NODE_LOCAL: {
        this->fail("Straight assign for local variable is not supported yet.");
        /*
        if (node->size() == 1) {
            push_op(new OP(OP_GET_PACKAGE_VARIABLE));
            push_op(new OP(OP_LOCAL));

            this->set_lvalue(node->at(0));
        } else {
            printf("This is not lvalue(MY):\n");
            node->dump(1);
            this->error++;
        }
        */
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
            compiler->push_op(enter);
            compiler->push_block(BLOCK_TYPE_BLOCK);
        } else {
            enter = NULL;
        }
    }
    ~OptimizableEnterScope() {
        if (enter) {
            compiler_->push_op(new OP(OP_LEAVE));
            enter->operand.int_value = compiler_->blocks->back().vars.size();
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
        "caller",
        "callee",
        "getcwd",
        "getpid",
        "getppid",
        "system",
        "abs", "atan2", "cos", "exp", "hex", "int", "log", "oct", "rand",
        "sin", "sqrt", "srand",
        "sprintf", "printf",
        NULL
    };
    for (int i=0; bs[i]; i++) {
        if (s == bs[i]) {
            return true;
        }
    }
    return false;
}

void tora::Compiler::fail(const char *format, ...) {
    fprintf(stderr, "Compilation failed: ");

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, " at %s line %d\n", filename_.c_str(), current_node->lineno);

    error++;
}

void tora::Compiler::compile(const SharedPtr<Node> &node) {
    this->current_node.reset(node.get());

    switch (node->type) {
    case NODE_ROOT: {
        this->push_block(BLOCK_TYPE_FILE);
        ID package_id = this->symbol_table->get_id(this->package());
        OP *pkg = new OP(OP_PACKAGE_ENTER);
        pkg->operand.int_value = package_id;
        push_op(pkg);

        this->push_block(BLOCK_TYPE_BLOCK);
        OP *enter = new OP(OP_ENTER);
        push_op(enter);

        this->compile(node->at(0));

        push_op(new OP(OP_END));

        enter->operand.int_value = this->blocks->back().vars.size();
        this->pop_block();

        push_op(new OP(OP_PACKAGE_LEAVE));
        this->pop_block();

        break;
    }
    case NODE_LOCAL: {
        if (node->at(0)->type != NODE_GET_PACKAGE_VARIABLE && node->at(0)->type != NODE_GETVARIABLE) {
            this->fail("You cannot localize %s\n", node->at(0)->type_name_str());
            break;
        }
        const std::string & name = node->at(0)->upcast<StrNode>()->str_value;

        int level; bool need_closure; bool is_arg; int funcdef_level;
        int ret = find_localvar(name, level, need_closure, is_arg, funcdef_level);
        if (ret >= 0) {
            this->fail("You cannot localize lexical vars: %s\n", name.c_str());
            break;
        }

        SharedPtr<StrValue> sv = new StrValue(node->at(0)->upcast<StrNode>()->str_value);
        SharedPtr<ValueOP> tmp = new ValueOP(OP_PUSH_STRING, sv);
        push_op(tmp);
        push_op(new OP(OP_LOCAL));

        if (node->at(1)) {
            // local $Foo::Bar(at0) = $val(at1);
            this->compile(node->at(1));
            this->set_lvalue(node->at(0));
        } else {
            // local $Foo::Bar(at0);
            this->compile(node->at(0));
        }

        break;
    }
    case NODE_RETURN: {
        if (this->in_try_block) {
            node->list->insert(node->list->begin(), new Node(NODE_UNDEF));
        }

        if (node->size() == 1) {
            this->compile(node->at(0));
            push_op(new OP(OP_RETURN));
        } else if (node->size() == 0) {
            push_op(new OP(OP_PUSH_UNDEF));
            push_op(new OP(OP_RETURN));
        } else {
            for (size_t i=0; i < node->size(); i++) {
                this->compile(node->at(i));
            }
            push_op(new OP(OP_MAKE_TUPLE, node->size()));
            push_op(new OP(OP_RETURN));
        }

        break;
    }
    case NODE_BLOCK: {
        {
            OptimizableEnterScope es(this, node->at(0));
            this->compile(node->at(0));
        }

        break;
    }
    case NODE_LAMBDA: {
        /*
        struct {
            std::vector<Node*> *params;
            Node *block;
        } funcdef;
        
        putcodevalue v
        make_function
        */

        Compiler::TryGuard guard(this, false);

        auto funcdef_node = node->upcast<FuncdefNode>();

        // function name

        this->push_block(BLOCK_TYPE_FUNCDEF);
        boost::shared_ptr<std::vector<std::string>> params(new std::vector<std::string>());
        if (funcdef_node->params() && funcdef_node->params()->size() > 0) {
            for (size_t i=0; i<funcdef_node->params()->size(); i++) {
                params->push_back(funcdef_node->params()->at(i)->upcast<StrNode>()->str_value);
                this->define_localvar(std::string(funcdef_node->params()->at(i)->upcast<StrNode>()->str_value));
            }
        } else {
            params->push_back(std::string("$_"));
            this->define_localvar(std::string("$_"));
        }

        Compiler funccomp(this->symbol_table, filename_);
        funccomp.init_globals();
        if (funccomp.blocks) {
            delete funccomp.blocks;
            funccomp.blocks = NULL;
        }
        funccomp.package(this->package());
        funccomp.blocks = new std::vector<Block>(*(this->blocks));
        if (funcdef_node->block()) {
            funccomp.compile(funcdef_node->block());
        }
        this->pop_block();

        funccomp.push_op(new OP(OP_RETURN));
        if (this->dump_ops) {
            Disasm::disasm(funccomp.ops);
        }

        // printf("CLOSURE VARS: %d\n", funccomp.closure_vars->size());

        SharedPtr<CodeValue> code = new CodeValue(
            this->symbol_table->get_id("<lambda>"), // package id
            this->symbol_table->get_id("<lambda>"), // func name id
            filename_,
            node->lineno,
            params
        );
        assert(params);
        code->code_opcodes(funccomp.ops);
        code->closure_var_names(funccomp.closure_vars);

        // if (funccomp.closure_vars->size() > 0) {
            // create closure
            // push variables  to stack.
#ifdef PERLISH_CLOSURE
            {
                auto iter = funccomp.closure_vars->begin();
                for (; iter!=funccomp.closure_vars->end(); iter++) {
                    this->compile(new StrNode(NODE_GETVARIABLE, *iter));
                }
            }
#endif

            SharedPtr<ValueOP> putval = new ValueOP(OP_PUSH_VALUE, code);
            push_op(putval);

            // define method.
            push_op(new OP(OP_CLOSUREDEF, funccomp.closure_vars->size()));
        // } else {
            /*
            SharedPtr<ValueOP> putval = new ValueOP(OP_PUSH_VALUE, code);
            push_op(putval);

            // create normal function
            SharedPtr<OP> define_method = new OP(OP_FUNCDEF);
            push_op(define_method);
            */
        // }

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
        std::string &name = funcdef_node->name()->upcast<StrNode>()->str_value;

        this->push_block(BLOCK_TYPE_FUNCDEF);

        // setup parameters
        boost::shared_ptr<std::vector<std::string>> params;
        if (funcdef_node->have_params()) {
            // 'sub foo () { }' form.
            params.reset(new std::vector<std::string>());
            for (size_t i=0; i<funcdef_node->params()->size(); i++) {
                assert(funcdef_node->params()->at(i)->list->size() == 2);
                const SharedPtr<Node>& param_name = funcdef_node->params()->at(i)->at(0);

                params->push_back(param_name->upcast<StrNode>()->str_value);
                this->define_localvar(std::string(param_name->upcast<StrNode>()->str_value));
            }
        } else {
            // 'sub foo { }' form.
            // params->push_back(std::string("$_"));
            this->define_localvar(std::string("$_"));
        }

        std::string package(this->package());
        std::string funcname(name);
        tora::split_package_funname(name, package, funcname);

        Compiler funccomp(this->symbol_table, filename_);
        funccomp.init_globals();
        if (funccomp.blocks) {
            delete funccomp.blocks;
            funccomp.blocks = NULL;
        }
        funccomp.package(package);
        funccomp.blocks = new std::vector<Block>(*(this->blocks));

        boost::shared_ptr<std::vector<int>> defaults(new std::vector<int>());
        if (funcdef_node->have_params()) {
            // process default values.
            funccomp.current_node.reset(node.get());
            OP * skip_defvars = new OP(OP_JUMP);
            funccomp.push_op(skip_defvars);
            for (size_t i=0; i<funcdef_node->params()->size(); i++) {
                assert(funcdef_node->params()->at(i)->list->size() == 2);
                const SharedPtr<Node>& default_node = funcdef_node->params()->at(i)->at(1);
                if (default_node.get()) {
                    // printf("FOUND DEFAULT VALUE: %ld\n", (long int) i);
                    // compile arguments to anonymous function.
                    defaults->push_back(funccomp.ops->size());
                    funccomp.compile(default_node);
                    funccomp.push_op(new OP(OP_END));
                } else {
                    defaults->push_back(-1);
                }
            }
            skip_defvars->operand.int_value = funccomp.ops->size();
        }

        funccomp.compile(funcdef_node->block());
        this->pop_block();

        funccomp.push_op(new OP(OP_RETURN));
        if (this->dump_ops) {
            Disasm::disasm(funccomp.ops);
        }
        this->error += funccomp.error;

        // printf("CLOSURE VARS: %d\n", funccomp.closure_vars->size());

        SharedPtr<CodeValue> code(new CodeValue(
            this->symbol_table->get_id(package), // package id
            this->symbol_table->get_id(funcname),        // func name id
            filename_,
            node->lineno,
            params
        ));
        // code->code_id = this->symbol_table->get_id(package + "::" + funcname);
        code->code_defaults(defaults);
        code->code_opcodes(funccomp.ops);
        code->closure_var_names(funccomp.closure_vars);

        SharedPtr<StrValue> funcname_value = new StrValue(funcname);
        if (1 && funccomp.closure_vars->size() > 0) {
            // create closure
            // push variables  to stack.
#ifdef PERLISH_CLOSURE
            {
                auto iter = funccomp.closure_vars->rbegin();
                for (; iter!=funccomp.closure_vars->rend(); iter++) {
                    this->compile(new StrNode(NODE_GETVARIABLE, *iter));
                }
            }
#endif

            // define method.
            push_op(new ValueOP(OP_PUSH_VALUE, code));
            push_op(new OP(OP_CLOSUREDEF, funccomp.closure_vars->size()));
        } else {
            // create normal function
            push_op(new ValueOP(OP_PUSH_VALUE, code));
            push_op(new OP(OP_FUNCDEF));
        }

        break;
    }
    case NODE_STRING: {
        SharedPtr<StrValue> sv = new StrValue(node->upcast<StrNode>()->str_value);
        SharedPtr<ValueOP> tmp = new ValueOP(OP_PUSH_STRING, sv);
        push_op(tmp);
        break;
    }
    case NODE_BYTES: {
        SharedPtr<BytesValue> sv = new BytesValue(node->upcast<StrNode>()->str_value);
        SharedPtr<ValueOP> tmp = new ValueOP(OP_PUSH_VALUE, sv);
        push_op(tmp);
        break;
    }
    case NODE_REGEXP: {
        auto re = node->upcast<RegexpNode>();
        SharedPtr<AbstractRegexpValue> sv = new RE2RegexpValue(re->regexp_value, re->flags);
        if (!sv->ok()) {
            this->fail("Regexp compilation failed: /%s/ : %s\n", sv->pattern().c_str(), sv->error().c_str());
            break;
        }
        push_op(new ValueOP(OP_PUSH_VALUE, sv));
        break;
    }
    case NODE_RANGE: {
        this->compile(node->upcast<BinaryNode>()->right());
        this->compile(node->upcast<BinaryNode>()->left());
        push_op(new OP(OP_NEW_RANGE));
        break;
    }
    case NODE_INT: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_INT;
        tmp->operand.int_value = node->upcast<IntNode>()->int_value;
        push_op(tmp);
        break;
    }
    case NODE_DOUBLE: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_DOUBLE;
        tmp->operand.double_value = node->upcast<DoubleNode>()->double_value;
        push_op(tmp);
        break;
    }
    case NODE_SELF: {
        push_op(new OP(OP_PUSH_SELF));
        break;
    }
    case NODE_TRUE: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_TRUE;
        push_op(tmp);
        break;
    }
    case NODE_UNDEF: {
        push_op(new OP(OP_PUSH_UNDEF));
        break;
    }
    case NODE_FALSE: {
        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_PUSH_FALSE;
        push_op(tmp);
        break;
    }
    case NODE_IDENTIFIER: {
        ID id = this->symbol_table->get_id(node->upcast<StrNode>()->str_value);
        push_op(new OP(OP_PUSH_IDENTIFIER, id));
        break;
    }
    case NODE_INSTANCIATE_IDENTIFIER: {
        ID id = this->symbol_table->get_id(node->upcast<StrNode>()->str_value);
        push_op(new OP(OP_INSTANCIATE_IDENTIFIER, id));
        break;
    }

    case NODE_FUNCALL: {
        LoopContext lc(this, false);

        auto args = node->upcast<FuncallNode>()->args();
        int args_len = args->size();
        for (auto iter=args->rbegin(); iter!=args->rend(); iter++) {
            this->compile(*iter);
        }

        std::string funcname = node->upcast<FuncallNode>()->name()->upcast<StrNode>()->str_value;

        if (this->is_builtin(funcname)) {
            ID id = this->symbol_table->get_id(node->upcast<FuncallNode>()->name()->upcast<StrNode>()->str_value);
            SharedPtr<ValueOP> o = new ValueOP(OP_PUSH_VALUE, new SymbolValue(id));
            push_op(o);
            SharedPtr<OP> tmp = new OP;
            tmp->op_type = OP_BUILTIN_FUNCALL;
            tmp->operand.int_value = args_len; // the number of args
            push_op(tmp);
        } else {
            /*
            std::string funcname2 = funcname.find("::")==std::string::npos
                ? this->package() + "::" + funcname
                : funcname;
            */
            std::string funcname2(funcname);
            std::string pkgname;
            bool splitted = tora::split_package_funname(funcname, pkgname, funcname2);
            ID id = this->symbol_table->get_id(funcname2);
            SharedPtr<Value> v = new SymbolValue(id);
            push_op(new ValueOP(OP_PUSH_VALUE, v));

            push_op(new OP(
                OP_FUNCALL,
                args_len,
                splitted ? symbol_table->get_id(pkgname) : symbol_table->get_id(this->package_)
            ));
        }
        break;
    }
#define C_OP_BINARY(type) \
    { \
        this->compile(node->upcast<BinaryNode>()->left()); \
        this->compile(node->upcast<BinaryNode>()->right()); \
        SharedPtr<OP> tmp = new OP; \
        tmp->op_type = (type); \
        push_op(tmp); \
        break; \
    }

    case NODE_ADD: C_OP_BINARY(OP_ADD);
    case NODE_SUB: C_OP_BINARY(OP_SUB);
    case NODE_MUL: C_OP_BINARY(OP_MUL);
    case NODE_MOD: C_OP_BINARY(OP_MOD);
    case NODE_DIV: C_OP_BINARY(OP_DIV);
    case NODE_POW: C_OP_BINARY(OP_POW);
    case NODE_LT:  C_OP_BINARY(OP_LT);
    case NODE_GT:  C_OP_BINARY(OP_GT);
    case NODE_LE:  C_OP_BINARY(OP_LE);
    case NODE_GE:  C_OP_BINARY(OP_GE);
    case NODE_EQ:  C_OP_BINARY(OP_EQ);
    case NODE_NE:  C_OP_BINARY(OP_NE);
    case NODE_BITAND: C_OP_BINARY(OP_BITAND);
    case NODE_BITOR:  C_OP_BINARY(OP_BITOR);
    case NODE_BITXOR: C_OP_BINARY(OP_BITXOR);
    case NODE_BITLSHIFT: C_OP_BINARY(OP_BITLSHIFT);
    case NODE_BITRSHIFT: C_OP_BINARY(OP_BITRSHIFT);
#undef C_OP_BINARY

    // TODO: deprecate?
    case NODE_STMTS: {
        push_op(new OP(OP_NEXTSTATE));
        this->compile(node->upcast<BinaryNode>()->left());
        push_op(new OP(OP_NEXTSTATE));
        this->compile(node->upcast<BinaryNode>()->right());
        break;
    }
    case NODE_STMTS_LIST: {
        SharedPtr<Node> ln = node->upcast<Node>();
        for (int i=0; i<ln->size(); i++) {
            push_op(new OP(OP_NEXTSTATE));
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
            push_op(jump_else);

            if (if_node->if_body()) {
                this->compile(if_node->if_body());
            }

            SharedPtr<OP> jump_end = new OP;
            jump_end->op_type = OP_JUMP;
            push_op(jump_end);

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
        push_op(jump_if_false);

        OP *enter = new OP(OP_ENTER_WHILE);
        push_op(enter);
        this->push_block(BLOCK_TYPE_BLOCK);

            this->compile(node->upcast<BinaryNode>()->right()); //body

        push_op(new OP(OP_LEAVE));
        enter->operand.int_value = this->blocks->back().vars.size();
        this->pop_block();

        push_op(new OP(OP_JUMP, label1));

        int label2 = ops->size();
        jump_if_false->operand.int_value = label2;

        BOOST_FOREACH(auto n, last_labels) {
            *n = label2 - 1;
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
        auto pos = varname.rfind("::");
        if (global >= 0) {
            SharedPtr<OP> tmp = new OP();
            tmp->op_type = OP_GETGLOBAL;
            tmp->operand.int_value = global;
            push_op(tmp);
        } else if (pos != std::string::npos) {
            // package variable $Foo::Bar.
            std::string pkgname = varname.substr(1, pos-1);
            std::string detail = varname.substr(pos+2);
            push_op(new OP(OP_PUSH_IDENTIFIER, symbol_table->get_id(pkgname)));
            push_op(new OP(OP_GET_PACKAGE_VARIABLE, symbol_table->get_id(std::string("$") + detail)));
        } else {
            int level;
            bool need_closure;
            bool is_arg;
            int funcdef_level;
            int no = this->find_localvar(std::string(node->upcast<StrNode>()->str_value), level, need_closure, is_arg, funcdef_level);
            // dump_localvars();
            // printf("-- %s, no: %d, level: %d\n", varname.c_str(), no, level);
            if (no<0) {
                this->fail("There is no variable named '%s'(NODE_GETVARIABLE)\n", node->upcast<StrNode>()->str_value.c_str());
                return;
            }

            if (1 && need_closure) {
#ifdef PERLISH_CLOSURE
                int pos = -1;
                // find from closure_vars.
                auto iter = closure_vars->begin();
                for (; iter!=closure_vars->end(); iter++) {
                    if (*iter == varname) {
                        pos = no;
                        break;
                    }
                }
                if (pos == -1) {
                    // the variable is not registered for closure_vars yet.
                    // register it.
                    closure_vars->push_back(varname);
                    pos = closure_vars->size()-1;
                }
                push_op(new OP(OP_GETCLOSURE, pos));
#else
                closure_vars->push_back(varname);
                push_op(new OP(OP_GETCLOSURE, level-funcdef_level, no));
#endif
            } else {
                if (is_arg) {
                    SharedPtr<OP> tmp = new OP(OP_GETARG, no);
                    push_op(tmp);
                } else if (level == 0) {
                    DBG2("LOCAL\n");
                    push_op(new OP(OP_GETLOCAL, no));
                } else {
                    SharedPtr<OP> tmp = new OP;
                    DBG2("DYNAMIC\n");
                    tmp->op_type = OP_GETDYNAMIC;
                    tmp->operand.int_value = (((level)&0x0000ffff) << 16) | (no&0x0000ffff);
                    push_op(tmp);
                }
            }
        }
        break;
    }
    // '$x /= 3;' => '$x = $x / 3'
    // TODO: optimize
#define BINARY_ASSIGN(ccc, nnn) \
    case ccc: { \
        SharedPtr<BinaryNode> r = new BinaryNode(nnn, \
            node->upcast<BinaryNode>()->left(), \
            node->upcast<BinaryNode>()->right() \
        ); \
        SharedPtr<BinaryNode> p = new BinaryNode(NODE_SETVARIABLE, &(*(node->upcast<BinaryNode>()->left())), &(*(r->upcast<Node>()))); \
        this->compile(p); \
        break; \
    }
    BINARY_ASSIGN(NODE_ADD_ASSIGN, NODE_ADD)
    BINARY_ASSIGN(NODE_SUB_ASSIGN, NODE_SUB)
    BINARY_ASSIGN(NODE_MUL_ASSIGN, NODE_MUL)
    BINARY_ASSIGN(NODE_DIV_ASSIGN, NODE_DIV)
    BINARY_ASSIGN(NODE_AND_ASSIGN, NODE_BITAND)
    BINARY_ASSIGN(NODE_OR_ASSIGN,  NODE_BITOR)
    BINARY_ASSIGN(NODE_OROR_ASSIGN, NODE_LOGICAL_OR)
    BINARY_ASSIGN(NODE_XOR_ASSIGN, NODE_BITXOR)
    BINARY_ASSIGN(NODE_MOD_ASSIGN, NODE_MOD)
#undef BINARY_ASSIGN
    case NODE_SETVARIABLE: {
        // compile rvalue.
        this->compile(node->upcast<BinaryNode>()->right());
        // and assign it to lvalue.
        this->set_lvalue(node->upcast<BinaryNode>()->left());
        break;
    }
    case NODE_SETVARIABLE_MULTI: {
        this->compile(node->upcast<BinaryNode>()->right());

        SharedPtr<Node>ln = node->upcast<BinaryNode>()->left()->upcast<Node>();

        // extract
        OP* op = new OP(OP_EXTRACT_TUPLE);
        op->operand.int_value = ln->size();
        push_op(op);

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
                    this->compile(STRING_IF_BAREWORD(index));
                    push_op(new OP(OP_SET_ITEM));
                    break;
                }
                default: {
                    fprintf(stderr, "Compilation failed\n");
                    this->error++;
                    break;
                }
            }
            push_op(new OP(OP_POP_TOP));
        }

        break;
    }
    case NODE_MAKE_ARRAY: {
        auto args = node->upcast<Node>();
        int args_len = args->size();
        while (args->size() > 0) {
            this->compile(args->back());
            args->pop_back();
        }

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_MAKE_ARRAY;
        tmp->operand.int_value = args_len; // the number of args
        push_op(tmp);
        break;
    }
    case NODE_MAKE_HASH: {
        auto args = node->upcast<Node>();
        int args_len = args->size();
        for (int i=0; i<args_len; i+=2) {
            // val
            this->compile(args->at(i+1));

            // key
            // in hash key, bareword is string literal.
            SharedPtr<Node> key = args->at(i);
            this->compile(STRING_IF_BAREWORD(key));
        }

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_MAKE_HASH;
        tmp->operand.int_value = args_len; // the number of args
        push_op(tmp);
        break;
    }
    case NODE_GET_ITEM: {
        this->compile(node->upcast<BinaryNode>()->left());  // container

        SharedPtr<Node> index = node->upcast<BinaryNode>()->right();
        this->compile(STRING_IF_BAREWORD(index));

        SharedPtr<OP> tmp = new OP;
        tmp->op_type = OP_GET_ITEM;
        push_op(tmp);
        break;
    }

    case NODE_UNARY_NEGATIVE: {
        this->compile(node->at(0));

        push_op(new OP(OP_UNARY_NEGATIVE));

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
        push_op(enter);
        this->push_block(BLOCK_TYPE_BLOCK);

        this->compile(node->upcast<ForNode>()->initialize());
        int label1 = ops->size();
        this->compile(node->upcast<ForNode>()->cond());

        SharedPtr<OP> jump_label2 = new OP;
        jump_label2->op_type = OP_JUMP_IF_FALSE;
        push_op(jump_label2);

        {
            LoopContext lc(this, true);
            this->compile(node->upcast<ForNode>()->block());
        }

        this->compile(node->upcast<ForNode>()->postfix());

        SharedPtr<OP> jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        push_op(jump_label1);

        int label2 = ops->size();
        jump_label2->operand.int_value = label2;

        push_op(new OP(OP_LEAVE));
        enter->operand.int_value = this->blocks->back().vars.size();
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
        push_op(enter_foreach);

        size_t label1 = ops->size();

        push_op(new OP(OP_FOR_ITER));
        SharedPtr<OP> jump_label2 = new OP();
        jump_label2->op_type = OP_JUMP_IF_STOP_EXCEPTION;
        push_op(jump_label2); // FIX ME?

        // store variables
        if (node->upcast<ForEachNode>()->vars()) {
            SharedPtr<Node> n;
            if (node->upcast<ForEachNode>()->vars()->size() == 1) {
                n = node->upcast<ForEachNode>()->vars()->at(0);
            } else {
                n = node->upcast<ForEachNode>()->vars();
                n->type = NODE_TUPLE;
            }
            SharedPtr<Node> nl = new Node(NODE_MY);
            nl->push_back(n);
            this->set_lvalue(nl);
        } else {
            SharedPtr<Node> nl = new Node(NODE_MY);
            nl->push_back(new StrNode(NODE_GETVARIABLE, "$_"));
            this->set_lvalue(nl.get());
        }

        this->compile(node->upcast<ForEachNode>()->block());

        SharedPtr<OP> jump_label1 = new OP;
        jump_label1->op_type = OP_JUMP;
        jump_label1->operand.int_value = label1;
        push_op(jump_label1);

        size_t label2 = ops->size();
        jump_label2->operand.int_value = label2;

        enter_foreach->operand.int_value = this->blocks->back().vars.size();
        this->pop_block();

        push_op(new OP(OP_LEAVE));

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
        if (mcn->method()) {
            if (mcn->method()->type == NODE_IDENTIFIER) {
                ID id = this->symbol_table->get_id(mcn->method()->upcast<StrNode>()->str_value);
                SharedPtr<ValueOP> o = new ValueOP(OP_PUSH_VALUE, new SymbolValue(id));
                push_op(o);
            } else {
                fprintf(stderr, "Compilation error. This is not a id.\n");
                error++;
                break;
            }
        } else {
            // closure call
            ID id = symbol_table->get_id("()");
            SharedPtr<ValueOP> o = new ValueOP(OP_PUSH_VALUE, new SymbolValue(id));
            push_op(o);
        }
        this->compile(mcn->object());

        SharedPtr<OP> op = new OP(OP_METHOD_CALL);
        op->operand.int_value = args_len;
        push_op(op);
        break;
    }
    case NODE_NOT: {
        // ! $val
        this->compile(node->at(0));
        push_op(new OP(OP_NOT));
        break;
    }
    case NODE_FILE_TEST: {
        // -f $file
        OP *op = new OP(OP_FILE_TEST);
        op->operand.int_value = node->upcast<BinaryNode>()->left()->upcast<IntNode>()->int_value;
        this->compile(node->upcast<BinaryNode>()->right());
        push_op(op);
        break;
    }
    case NODE_POST_DECREMENT: {
        // $i--
        this->compile(node->at(0));
        push_op(new OP(OP_POST_DECREMENT));
        break;
    }
    case NODE_PRE_DECREMENT: {
        // --$i
        this->compile(node->at(0));
        push_op(new OP(OP_PRE_DECREMENT));
        break;
    }
    case NODE_POST_INCREMENT: {
        // $i++
        this->compile(node->at(0));
        push_op(new OP(OP_POST_INCREMENT));
        break;
    }
    case NODE_PRE_INCREMENT: {
        // ++$i
        this->compile(node->at(0));
        push_op(new OP(OP_PRE_INCREMENT));
        break;
    }
    case NODE_TUPLE: {
        SharedPtr<Node>ln = node->upcast<Node>();
        for (size_t i=0; i < ln->size(); i++) {
            this->compile(ln->at(i));
        }
        OP *op = new OP(OP_MAKE_TUPLE);
        op->operand.int_value = ln->size();
        push_op(op);
        break;
    }
    case NODE_DOTDOTDOT: {
        push_op(new OP(OP_DOTDOTDOT));
        break;
    }
    case NODE_TRY: {
        LoopContext lc(this, false);

        Compiler::TryGuard guard(this, true);

        OP *try_op = new OP(OP_TRY);
        push_op(try_op);

        this->push_block(BLOCK_TYPE_TRY);
        OP *enter_op = new OP(OP_ENTER);
        push_op(enter_op);

        this->compile(node->at(0));

        push_op(new OP(OP_PUSH_UNDEF));
        push_op(new OP(OP_PUSH_UNDEF));
        OP *op = new OP(OP_MAKE_TUPLE);
        op->operand.int_value = 2;
        push_op(op);
        push_op(new OP(OP_RETURN));

        enter_op->operand.int_value = this->blocks->back().vars.size();

        this->pop_block();

        try_op->operand.int_value = ops->size();

        break;
    }
    case NODE_DIE: {
        this->compile(node->at(0));
        push_op(new OP(OP_DIE));
        break;
    }
    case NODE_USE: {
        // use Test::More;           # => 0
        // use Test::More *;         # => 1
        // use Test::More qw(ok is); # => 1
        this->compile(node->upcast<BinaryNode>()->right());
        this->compile(node->upcast<BinaryNode>()->left());
        push_op(new OP(OP_USE));
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
        std::string orig_pkgname(this->package_);
        this->package_ = klass_name;
        ID package_id = this->symbol_table->get_id(
            /*
              this->package() == "main"
            ? klass_name
            : this->package() + "::" + klass_name
            */
            klass_name
        );
        push_op(new OP(OP_CLASS_ENTER, package_id));

        if (n->parent()) {
            OP * op = new OP(OP_SET_PARENT);
            op->operand.int_value = symbol_table->get_id(n->parent()->upcast<StrNode>()->str_value);
            push_op(op);
        }

        if (n->block() != NULL) {
            this->compile(n->block());
        }

        push_op(new OP(OP_CLASS_LEAVE));
        this->pop_block();

        this->package_ = orig_pkgname;

        this->in_class_context = false;

        break;
    }
    case NODE_MY: {
        this->define_my(node);
        break;
    }
    case NODE_LAST: {
        // break from for/foreach/while
        if (!in_loop_context) {
            fail("You can't put \"last\" statement out of loop block.\n");
        } else {
            OP * op = new OP(OP_LAST);
            this->last_labels.push_back(&(op->operand.int_value));
            push_op(op);
        }
        break;
    }
    case NODE_LOGICAL_AND: {
        /**
         *    (lhs)
         *    JUMP_IF_FALSE label1
         *    POP_TOP
         *    (rhs)
         * label1:
         */

        const BinaryNode * n = static_cast<BinaryNode*>(node.get());

        this->compile(n->left());

        SharedPtr<OP> jump_else = new OP(OP_JUMP_IF_FALSE);
        push_op(jump_else);

        push_op(new OP(OP_POP_TOP));
        this->compile(n->right());

        jump_else->operand.int_value = ops->size();

        break;
    }
    case NODE_LOGICAL_OR: {
        /**
         *    (lhs)
         *    JUMP_IF_TRUE label1
         *    POP_TOP
         *    (rhs)
         * label1:
         */

        const BinaryNode * n = static_cast<BinaryNode*>(node.get());

        this->compile(n->left());

        SharedPtr<OP> jump_else = new OP(OP_JUMP_IF_TRUE);
        push_op(jump_else);

        push_op(new OP(OP_POP_TOP));
        this->compile(n->right());

        jump_else->operand.int_value = ops->size();

        break;
    }
    case NODE_DEREF: {
        this->compile(node->at(0));
        push_op(new OP(OP_DEREF));
        break;
    }
    case NODE_EXTRACT_ARRAY: {
        this->compile(node->at(0));
        push_op(new OP(OP_EXTRACT_ARRAY));
        break;
    }

    default:
        this->error++;
        printf("[BUG] Unknown node: %s\n", node->type_name_str());
        abort();
        break;
    }
}
