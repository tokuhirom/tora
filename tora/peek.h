#ifndef TORA_PEEK_H_
#define TORA_PEEK_H_

#include "shared_ptr.h"

namespace tora {

class Value;
class VM;

/**
 * Dump a debug information.
 *
 * This is Devel::Peek in Perl5.
 */
void peek(VM *vm, Value *v);
};

#endif  // TORA_PEEK_H_
