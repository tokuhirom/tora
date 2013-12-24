#include "tora.h"
#include <vm.h>
#include <util.h>
#include <value/array.h>
#include <value/hash.h>
#include <value/exception.h>

#include <boost/filesystem.hpp>

using namespace tora;

/**
 * File::Path::mkpath(Str $name) : Undef
 *
 */
static SharedPtr<Value> File_Path_mkpath(VM *vm, Value *name) {
    try {
        boost::filesystem::create_directories(name->to_s()->c_str());
        return new_undef_value();
    } catch (boost::filesystem::filesystem_error& ex) {
        throw new ExceptionValue(ex.what());
    };
}

/**
 * File::Path::rmtree(Str $name) : Undef
 */
static SharedPtr<Value> File_Path_rmtree(VM *vm, Value *name) {
    try {
        boost::filesystem::remove_all(name->to_s()->c_str());
        return new_undef_value();
    } catch (boost::filesystem::filesystem_error& ex) {
        throw new ExceptionValue(ex.what());
    };
}

extern "C" {

TORA_EXPORT
void Init_File_Path(VM* vm) {
    vm->add_function("mkpath", new CallbackFunction(File_Path_mkpath));
    vm->add_function("rmtree", new CallbackFunction(File_Path_rmtree));
}

}
