#ifndef TORA_H_
#define TORA_H_
#define TORA_VERSION_STR "0.0.1"

#include <vector>
#include "value.h"

// Debug Macro
#ifdef DEBUG
#define DBG(fmt, ...) printf(fmt, __VA_ARGS__)
#define DBG2(fmt) printf(fmt)
#else
#define DBG(...)
#define DBG2(...)
#endif

#endif // TORA_H_

