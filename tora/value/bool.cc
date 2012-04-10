#include "bool.h"

using namespace tora;

#ifdef ENABLE_OBJECT_POOL

boost::object_pool<BoolValue> BoolValue::pool_;

#endif

