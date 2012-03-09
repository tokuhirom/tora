#ifndef TORA_VALUE_UNDEF_H_
#define TORA_VALUE_UNDEF_H_

#include "../value.h"

namespace tora {

class UndefValue: public Value {
private:
    UndefValue(): Value(VALUE_TYPE_UNDEF) { }
public:
    static UndefValue *instance() {
        return new UndefValue();
    }
};

};

#endif // TORA_VALUE_UNDEF_H_
