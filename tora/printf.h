#ifndef TORA_PRINTF_H_
#define TORA_PRINTF_H_

#include <vector>
#include "shared_ptr.h"

namespace tora {
    class Value;

    std::string tora_sprintf(const std::vector<SharedPtr<Value>> & args);
    void tora_printf(const std::vector<SharedPtr<Value>> & args);
};

#endif // TORA_PRINTF_H_
