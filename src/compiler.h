#ifndef TORA_COMPILER_H_
#define TORA_COMPILER_H_

#include "node.h"
#include "vm.h"
#include "symbol_table.h"
#include "shared_ptr.h"
#include "op_array.h"
#include <stdarg.h>

namespace tora {

enum block_type_t {
    BLOCK_TYPE_BLOCK,
    BLOCK_TYPE_SUB,
    BLOCK_TYPE_FILE,
    BLOCK_TYPE_TRY,
    BLOCK_TYPE_FUNCDEF,
    BLOCK_TYPE_CLASS,
};

class Block : public Prim {
public:
    block_type_t type;
    std::vector<std::string*> vars;
    Block(block_type_t t) : type(t) { }
    ~Block() {
        for (size_t i=0; i<vars.size(); i++) {
            delete vars.at(i);
        }
    }
};

class Compiler {
    std::string package_;
public:
    SharedPtr<OPArray> ops;
    std::vector<SharedPtr<Block>> *blocks;
    std::vector<std::string> *global_vars;
    std::vector<std::string> *closure_vars;
    SharedPtr<SymbolTable> symbol_table;
    int error;
    bool in_try_block;
    bool dump_ops;
    bool in_class_context;

    void package(const std::string & p) { package_ = p; }
    std::string & package() { return package_; }

    Compiler(SharedPtr<SymbolTable> &symbol_table_) : in_class_context(false) {
        error = 0;
        blocks = new std::vector<SharedPtr<Block>>();
        global_vars = new std::vector<std::string>();
        ops = new OPArray();
        in_try_block = false;
        symbol_table = symbol_table_;
        dump_ops = false;
        package_ = "main";
        closure_vars = new std::vector<std::string>();
    }
    ~Compiler() {
        delete global_vars;
        delete blocks;
        delete closure_vars;
    }
    void define_my(SharedPtr<Node> node);
    void define_global_var(const char *name) {
        auto iter = global_vars->begin();
        for (; iter!=global_vars->end(); iter++) {
            if (*iter==name) {
                printf("[BUG] duplicated global variable name: %s\n", name);
                abort();
            }
        }
        global_vars->push_back(name);
    }
    int find_global_var(std::string & name) {
        auto iter = global_vars->begin();
        for (; iter!=global_vars->end(); iter++) {
            if (*iter==name) {
                return iter-global_vars->begin();
            }
        }
        return -1;
    }
    void init_globals();
    void compile(SharedPtr<Node> node);
    void push_block(block_type_t t) {
        DBG("PUSH BLOCK: %d\n", this->blocks->size());
        this->blocks->push_back(new Block(t));
    }
    void pop_block() {
        DBG("POP BLOCK: %d\n", this->blocks->size());
        this->blocks->pop_back();
    }
    void set_lvalue(SharedPtr<Node> node);
    void set_variable(std::string &varname);
    int find_localvar(std::string name, int &level, bool &need_closure);
    void define_localvar(const char* name) {
        this->define_localvar(std::string(name));
    }
    void define_localvar(std::string name) {
        SharedPtr<Block> block = this->blocks->back();
        for (size_t i=0; i<block->vars.size(); i++) {
            if (*(block->vars.at(i)) == name) {
                fprintf(stderr, "Duplicated variable: %s", name.c_str());
                return;
            }
        }
        block->vars.push_back(new std::string(name));
        DBG("Defined local variable: %s\n", name.c_str());
    }
    void dump_localvars() {
        printf("-- dump_localvars --\n");
        printf("Levels: %zd\n", this->blocks->size());
        for (size_t level = 0; level < this->blocks->size(); ++level) {
            SharedPtr<Block> block = this->blocks->at(level);
            printf("[%zd]\n", level);
            for (size_t i=0; i<block->vars.size(); i++) {
                printf("    %s\n", block->vars.at(i)->c_str());
            }
        }
        printf("--------------------\n");
    }
    void fail(const char *format, ...) {
        fprintf(stderr, "Compilation failed:\n");
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);

        error++;
    }

    bool is_builtin(const std::string &s);

    /**
     * This is RAII guard object.
     */
    class TryGuard {
    private:
        Compiler *c;
        bool orig;
    public:
        TryGuard(Compiler *c_, bool b) : c(c_) {
            orig = c_->in_try_block;
            c->in_try_block = b;
        }
        ~TryGuard() {
            c->in_try_block = orig; //restore
        }
    };
};

};


#endif // TORA_COMPILER_H_
