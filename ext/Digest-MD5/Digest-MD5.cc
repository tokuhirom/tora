#include "tora.h"
#include "md5.h"
#include <vm.h>
#include <value/array.h>
#include <value/hash.h>
#include <value/exception.h>
#include <package.h>

using namespace tora;

inline char int2hexchar(int n) {
    if (0 <= n && n <= 9) {
        return '0' + n;
    } else {
        return 'a' + n - 10;
    }
}

static SharedPtr<Value> Digest_MD5_md5_hex(VM *vm, Value *str) {
    if (str->value_type != VALUE_TYPE_STR) {
        throw new ExceptionValue("You passed non-string value for Digest::MD5::md5_hex() : %s", str->type_str());
    }

    md5_state_t pms;
    md5_byte_t digest[16];
    md5_init(&pms);
    md5_append(&pms, (const md5_byte_t*)str->upcast<StrValue>()->str_value().c_str(), str->upcast<StrValue>()->str_value().size());
    md5_finish(&pms, digest);
    std::string ret;
    for (int i=0; i<16; i++) {
        ret += int2hexchar((digest[i] >> 4) & 0xf);
        ret += int2hexchar(digest[i] & 0x0f);
    }
    return new StrValue(ret);
}

extern "C" {

TORA_EXPORT
void Init_Digest_MD5(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("Digest::MD5");
    pkg->add_method(vm->symbol_table->get_id("md5_hex"), new CallbackFunction(Digest_MD5_md5_hex));
}

}
