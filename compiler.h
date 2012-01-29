#ifndef COMPILER_H_
#define COMPILER_H_

#include "vm.h"

namespace tora {

class Compiler {
    VM *vm;
public:
    Compiler(VM*vm_) {
        vm = vm_;
    }
    void compile(TNode *node);
};

};


#endif // COMPILER_H_
