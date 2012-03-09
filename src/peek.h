#ifndef TORA_PEEK_H_
#define TORA_PEEK_H_

namespace tora {

class Value;

/**
 * Dump a debug information.
 *
 * This is Devel::Peek in Perl5.
 */
void peek(const Value * v);

};

#endif // TORA_PEEK_H_
