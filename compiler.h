#ifndef COMPILER_H_
#define COMPILER_H_

#include "vm.h"

// Debug Macro
#ifdef DEBUG
#define DBG(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define DBG(...)
#endif

namespace tora {

class Block {
public:
    std::vector<std::string*> vars;
    ~Block() {
        for (size_t i=0; i<vars.size(); i++) {
            delete vars.at(i);
        }
    }
};

class Compiler {
    VM *vm;
    std::vector<Block*> blocks;
public:
    bool error;
    Compiler(VM*vm_) {
        vm = vm_;
        error = false;
    }
    void compile(TNode *node);
    void push_block() {
        this->blocks.push_back(new Block());
    }
    void pop_block() {
        delete this->blocks.back();
        this->blocks.pop_back();
    }
    int find_localvar(std::string name) {
        std::vector<Block*>::iterator iter = this->blocks.begin();
        for (; iter != this->blocks.end(); iter++) {
            Block *block = *iter;
            for (size_t i=0; i<block->vars.size(); i++) {
                if (*(block->vars.at(i)) == name) {
                    return i;
                }
            }
        }
        return -1;
    }
    void define_localvar(std::string name) {
        Block *block = this->blocks.back();
        for (size_t i=0; i<block->vars.size(); i++) {
            if (*(block->vars.at(i)) == name) {
                fprintf(stderr, "Duplicated variable: %s", name.c_str());
                return;
            }
        }
        block->vars.push_back(new std::string(name));
        DBG("Defined local variable: %s\n", name.c_str());
    }
};

};


#endif // COMPILER_H_
