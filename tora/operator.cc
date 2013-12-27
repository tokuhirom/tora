#include "operator.h"
#include "value.h"
#include "value/hash.h"
#include "value/object.h"
#include "value/array.h"
#include "value/bool.h"
#include "value/double.h"
#include "value/exception.h"
#include "ops.gen.h"
#include "value/bytes.h"
#include "value/int.h"
#include <math.h>

using namespace tora;

/**
 * binary addition operator
 */
SharedValue tora::op_add(Value*lhs, Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_INT) {
    if (rhs->value_type == VALUE_TYPE_DOUBLE) {
      // upgrade to double type
      MortalDoubleValue d(lhs->to_double() + rhs->to_double());
      return d.get();
    } else {
      int i = rhs->to_int();
      MortalIntValue miv(get_int_value(*lhs) + i); return miv.get();
    }
  } else if (lhs->value_type == VALUE_TYPE_STR) {
    std::string s = rhs->to_s();
    return new_str_value(*(get_str_value(lhs)) + s);
  } else if (lhs->value_type == VALUE_TYPE_BYTES) {
    if (type(rhs) == VALUE_TYPE_BYTES) {
      MortalBytesValue b(*get_bytes_value(lhs) + *get_bytes_value(rhs));
      return b.get();
    } else {
      throw new ExceptionValue("You can't concatenate '%s' with bytes.\n", rhs->type_str());
    }
  } else if (lhs->value_type == VALUE_TYPE_DOUBLE) {
    MortalDoubleValue d(lhs->to_double() + rhs->to_double());
    return d.get();
  } else {
    std::string s = lhs->to_s();
    throw new ExceptionValue("'%s' is not numeric or string.\n", s.c_str());
  }
  abort();
}

/**
 * subtract lhs and rhs.
 * return value must be return by caller.
 */
SharedValue tora::op_sub(Value*lhs, Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_DOUBLE) {
    MortalDoubleValue d(lhs->to_double() - rhs->to_double());
    return d.get();
  } else if (lhs->value_type == VALUE_TYPE_INT) {
    if (rhs->value_type == VALUE_TYPE_DOUBLE) {  // upgrade
      MortalDoubleValue d(lhs->to_double() - rhs->to_double());
      return d.get();
    } else {
      MortalIntValue miv(get_int_value(*lhs) - rhs->to_int()); return miv.get();
    }
  } else {
    std::string s(lhs->to_s());
    throw new ExceptionValue("'%s' is not numeric. You cannot subtract.",
                             s.c_str());
  }
  abort();
}

SharedValue tora::op_div(Value*lhs, Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_DOUBLE) {
    double rhs_double = rhs->to_double();
    if (rhs_double == 0) {
      throw new ZeroDividedExceptionExceptionValue();
    }
    MortalDoubleValue d(lhs->to_double() / rhs_double);
    return d.get();
  } else if (lhs->value_type == VALUE_TYPE_INT) {
    if (rhs->value_type == VALUE_TYPE_DOUBLE) {  // upgrade
      double rhs_double = rhs->to_double();
      if (rhs_double == 0) {
        throw new ZeroDividedExceptionExceptionValue();
      }
      MortalDoubleValue d(lhs->to_double() / rhs_double);
      return d.get();
    } else {
      double rhs_int = rhs->to_int();
      if (rhs_int == 0) {
        throw new ZeroDividedExceptionExceptionValue();
      }
      MortalIntValue miv(get_int_value(*lhs) / rhs_int); return miv.get();
    }
  } else {
    std::string s(lhs->to_s());
    throw new ExceptionValue("'%s' is not numeric. You cannot divide.",
                             s.c_str());
  }
  abort();
}

SharedValue tora::op_mul(Value*lhs, Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_DOUBLE) {
    MortalDoubleValue d(lhs->to_double() * rhs->to_double());
    return d.get();
  } else if (lhs->value_type == VALUE_TYPE_INT) {
    if (rhs->value_type == VALUE_TYPE_DOUBLE) {  // upgrade
      MortalDoubleValue d(lhs->to_double() * rhs->to_double());
      return d.get();
    } else {
      MortalIntValue miv(get_int_value(*lhs) * get_int_value(*rhs)); return miv.get();
    }
  } else if (lhs->value_type == VALUE_TYPE_STR) {
    std::ostringstream buf;
    int n = rhs->to_int();
    for (int i = 0; i < n; i++) {
      buf << *get_str_value(lhs);
    }
    return new_str_value(buf.str());
  } else {
    std::string s(lhs->to_s());
    throw new ExceptionValue("'%s' is not numeric. You cannot multiply.",
                             s.c_str());
  }
  abort();
}

SharedValue tora::op_pow(Value*lhs, Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_DOUBLE) {
    MortalDoubleValue d(pow(lhs->to_double(), rhs->to_double()));
    return d;
  } else if (lhs->value_type == VALUE_TYPE_INT) {
    if (rhs->value_type == VALUE_TYPE_DOUBLE) {  // upgrade
      MortalDoubleValue d(pow(lhs->to_double(), rhs->to_double()));
      return d;
    } else {
      MortalIntValue miv((int)pow(lhs->to_double(), rhs->to_double())); return miv.get();
    }
  } else {
    StringImpl s(lhs->to_s());
    throw new ExceptionValue("'%s' is not numeric. You cannot pow.", s.c_str());
  }
  abort();
}

SharedValue tora::op_bitand(Value*lhs,
                       Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_INT) {
    MortalIntValue miv(lhs->to_int() & rhs->to_int()); return miv.get();
  } else {
    StringImpl s(lhs->to_s());
    throw new ExceptionValue("'%s' is not integer. You cannot and.", s.c_str());
  }
  abort();
}

SharedValue tora::op_bitor(Value*lhs,
                      Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_INT) {
    MortalIntValue miv(lhs->to_int() | rhs->to_int()); return miv.get();
  } else {
    StringImpl s(lhs->to_s());
    throw new ExceptionValue("'%s' is not integer. You cannot bit or.",
                             s.c_str());
  }
  abort();
}

SharedValue tora::op_bitxor(Value*lhs,
                       Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_INT) {
    MortalIntValue miv(lhs->to_int() ^ rhs->to_int()); return miv.get();
  } else {
    StringImpl s(lhs->to_s());
    throw new ExceptionValue("'%s' is not integer. You cannot xor.", s.c_str());
  }
  abort();
}

SharedValue tora::op_bitlshift(Value*lhs,
                          Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_INT) {
    MortalIntValue miv(lhs->to_int() << rhs->to_int()); return miv.get();
  } else {
    StringImpl s(lhs->to_s());
    throw new ExceptionValue("'%s' is not integer. You cannot <<.", s.c_str());
  }
  abort();
}

SharedValue tora::op_bitrshift(Value*lhs,
                          Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_INT) {
    MortalIntValue miv(lhs->to_int() >> rhs->to_int()); return miv.get();
  } else {
    StringImpl s(lhs->to_s());
    throw new ExceptionValue("'%s' is not integer. You cannot >>.", s.c_str());
  }
  abort();
}

SharedValue tora::op_modulo(Value*lhs,
                       Value*rhs) {
  if (lhs->value_type == VALUE_TYPE_INT) {
    double x = lhs->to_int();
    double y = rhs->to_int();
    MortalIntValue miv(x - y * floor(x / y)); return miv.get();
  } else {
    StringImpl s(lhs->to_s());
    throw new ExceptionValue("'%s' is not integer. You cannot >>.", s.c_str());
  }
  abort();
}

SharedValue tora::op_unary_negative(Value*v) {
  switch (v->value_type) {
    case VALUE_TYPE_INT:
      {
        MortalIntValue miv(-(get_int_value(*v))); return miv.get();
      }
    case VALUE_TYPE_DOUBLE:
      {
        MortalDoubleValue d(-get_double_value(v));
        return d.get();
      }
    case VALUE_TYPE_OBJECT:
      TODO();
    default:
      throw new ExceptionValue(
          "%s is not a numeric. You cannot apply unary negative operator.\n",
          v->type_str());
  }
}

SharedPtr<Value> tora::op_get_item(Value*container,
                                   Value*index) {
  switch (container->value_type) {
    case VALUE_TYPE_HASH: {
      std::string index_s = index->to_s();
      return hash_get_item(container, index_s);
    }
    case VALUE_TYPE_ARRAY:
      return array_get_item(container, index->to_int());
    case VALUE_TYPE_OBJECT:
      return object_get_item(container, index).get();
    default:
      throw new ExceptionValue("%s is not a container.\n",
                               container->type_str());
  }
}

template <class operationI, class operationD, class OperationS>
bool tora::cmpop(operationI operation_i, operationD operation_d,
                 OperationS operation_s, Value*lhs,
                 Value*rhs) {
#ifndef NDEBUG
  if (rhs->value_type == VALUE_TYPE_SYMBOL) {
    fprintf(stderr, "[BUG] Do not compare with symbol value.\n");
    abort();
  }
#endif

  switch (lhs->value_type) {
    case VALUE_TYPE_INT: {
      int ie2 = rhs->to_int();
      return operation_i(get_int_value(*lhs), ie2);
    }
    case VALUE_TYPE_STR: {
      StringImpl s2(rhs->to_s());
      return (operation_s(*(get_str_value(lhs)), s2));
    }
    case VALUE_TYPE_BYTES: {
      if (rhs->value_type == VALUE_TYPE_BYTES) {
        return (operation_s(*(get_bytes_value(lhs)),
                            (*get_bytes_value(rhs))));
      } else {
        return false;
      }
    }
    case VALUE_TYPE_DOUBLE: {
      switch (rhs->value_type) {
        case VALUE_TYPE_INT: {
          return (
              operation_d(get_double_value(lhs), (double)get_int_value(*rhs)));
        }
        case VALUE_TYPE_DOUBLE: {
          return (operation_d(get_double_value(lhs), get_double_value(rhs)));
        }
        default: {
          TODO();  // throw exception
          abort();
        }
      }
      break;
    }
    case VALUE_TYPE_BOOL: {
      return get_bool_value(lhs) == rhs->to_bool();
    }
    case VALUE_TYPE_UNDEF: { return rhs->value_type == VALUE_TYPE_UNDEF; }
    default:
      // TODO: support object comparation
      std::string err("UNKNOWN MATCHING PATTERN lhs = '");
      err += lhs->type_str();
      err += "'\n";
      throw new ExceptionValue(err);
  }
  abort();
}

template bool tora::cmpop(std::equal_to<int>, std::equal_to<double>,
                          std::equal_to<std::string>, Value*,
                          Value*);
template bool tora::cmpop(std::greater<int>, std::greater<double>,
                          std::greater<std::string>, Value*,
                          Value*);
template bool tora::cmpop(std::greater_equal<int>, std::greater_equal<double>,
                          std::greater_equal<std::string>,
                          Value*, Value*);
template bool tora::cmpop(std::less<int>, std::less<double>,
                          std::less<std::string>, Value*,
                          Value*);
template bool tora::cmpop(std::less_equal<int>, std::less_equal<double>,
                          std::less_equal<std::string>,
                          Value*, Value*);
template bool tora::cmpop(std::not_equal_to<int>, std::not_equal_to<double>,
                          std::not_equal_to<std::string>,
                          Value*, Value*);
