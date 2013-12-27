#include "inspector.h"
#include "vm.h"
#include "value.h"
#include "value/array.h"
#include "value/hash.h"
#include "value/range.h"
#include "value/object.h"
#include "value/regexp.h"
#include "value/bytes.h"
#include "value/class.h"

using namespace tora;

Inspector::Inspector(VM* vm) : vm_(vm) { assert(vm); }

std::string Inspector::inspect(Value* v) const {
  switch (v->value_type) {
    case VALUE_TYPE_UNDEF:
      return "undef";
    case VALUE_TYPE_INT:
    case VALUE_TYPE_BOOL:
    case VALUE_TYPE_DOUBLE:
      return v->to_s();
    case VALUE_TYPE_STR: {
      std::string ret("\"");
      ret += v->to_s();
      ret += '"';
      return ret;
    }
    case VALUE_TYPE_CODE:
      return "sub { \"DUMMY\" }";
    case VALUE_TYPE_ARRAY: {
      std::string ret("[");
      MortalArrayIteratorValue iter(v);
      bool first = true;
      while (!array_iter_finished(iter.get())) {
        if (!first) {
          ret += ',';
        }
        ret += this->inspect(array_iter_get(iter.get()));
        first = false;
        array_iter_next(iter.get());
      }
      ret += "]";
      return ret;
    }
    case VALUE_TYPE_REGEXP: {
      // TODO: escape '/' character
      std::string ret("/");
      const AbstractRegexpValue* rv =
          static_cast<const AbstractRegexpValue*>(v);
      ret += rv->pattern();
      ret += "/";
      return ret;
    }
    case VALUE_TYPE_TUPLE:
      return "#<Tuple>";  // TODO
    case VALUE_TYPE_FILE:
      return "#<File>";
    case VALUE_TYPE_RANGE: {
      std::ostringstream os;
      os << range_left(v) << ".." << range_right(v);
      return os.str();
    }
    case VALUE_TYPE_ARRAY_ITERATOR:
      return "#<Array::Iterator>";
    case VALUE_TYPE_RANGE_ITERATOR:
      return "#<Range::Iterator>";
    case VALUE_TYPE_HASH_ITERATOR:
      return "#<Hash::Iterator>";
    case VALUE_TYPE_EXCEPTION:
      return "#<Exception>";
    case VALUE_TYPE_SYMBOL:
      return "#<Symbol>";
    case VALUE_TYPE_BYTES: {
      std::ostringstream os;
      std::string buf = *get_bytes_value(v);

      os << "b\"";
      for (auto iter = buf.begin(); iter != buf.end(); iter++) {
        const unsigned char c = *iter;
        os << "\\x";
        os << "0123456789abcdef"[c >> 4 & 0x0f];
        os << "0123456789abcdef"[c & 0x0f];
      }
      os << '"';
      return os.str();
    }
    case VALUE_TYPE_HASH: {
      std::string ret("{");
      MortalHashIteratorValue iter(v);
      bool first = true;
      while (!hash_iter_finished(iter.get())) {
        if (!first) {
          ret += ',';
        }
        ret += "\"";
        ret += hash_iter_getkey(iter.get());
        ret += "\" => ";
        ret += this->inspect(hash_iter_getval(iter.get()));
        first = false;
        hash_iter_increment(iter.get());
      }
      ret += "}";
      return ret;
    }
    case VALUE_TYPE_CLASS: { return class_name(v); }
    case VALUE_TYPE_OBJECT: {
      std::string ret;
      assert(vm_);
      assert(vm_->symbol_table);
      ret += v->type_str();
      ret += ".bless(";
      ret += this->inspect(object_data(v));
      ret += ")";
      return ret;
    }
    case VALUE_TYPE_POINTER:
      return "#<Pointer>";  // TODO
    case VALUE_TYPE_REFERENCE:
      return "#<Reference>";  // TODO
    case VALUE_TYPE_FILE_PACKAGE:
      return "#<FilePackage>";
  }
  printf("[BUG] unknown value type");
  abort();
}
