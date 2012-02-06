#ifndef TORA_COMPILER_H_
#define TORA_COMPILER_H_

#include "node.h"
#include "vm.h"

namespace tora {

class Block : public Prim {
public:
    std::vector<std::string*> vars;
    ~Block() {
        for (size_t i=0; i<vars.size(); i++) {
            delete vars.at(i);
        }
    }
};

class Compiler {
public:
    std::vector<SharedPtr<OP>> *ops;
    std::vector<Block*> *blocks;
    bool error;
    Compiler() {
        error = false;
        blocks = new std::vector<Block*>();
        ops = new std::vector<SharedPtr<OP>>();
    }
    ~Compiler() {
        delete ops;

        {
            auto iter = blocks->begin();
            for (; iter!=blocks->end(); iter++) {
                (*iter)->release();
            }
            delete blocks;
        }
    }
    void compile(Node *node);
    void push_block() {
        DBG("PUSH BLOCK: %d\n", this->blocks->size());
        this->blocks->push_back(new Block());
    }
    void pop_block() {
        DBG("POP BLOCK: %d\n", this->blocks->size());
        // delete this->blocks->back();
        this->blocks->pop_back();
    }
    int find_localvar(std::string name, int &level);
    void define_localvar(std::string name) {
        Block *block = this->blocks->back();
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
        printf("Levels: %d\n", this->blocks->size());
        for (size_t level = 0; level < this->blocks->size(); ++level) {
            Block *block = this->blocks->at(level);
            printf("[%d]\n", level);
            for (size_t i=0; i<block->vars.size(); i++) {
                printf("    %s\n", block->vars.at(i)->c_str());
            }
        }
        printf("--------------------\n");
    }
};

};


#endif // TORA_COMPILER_H_
