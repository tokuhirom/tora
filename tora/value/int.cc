#include "int.h"

using namespace tora;

#ifdef ENABLE_OBJECT_POOL
boost::object_pool<IntValue> IntValue::pool_;
#endif
