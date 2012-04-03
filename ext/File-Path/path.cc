#include "tora.h"
#include <vm.h>
#include <util.h>
#include <value/array.h>
#include <value/hash.h>
#include <value/exception.h>
#include <package.h>

#include <boost/filesystem.hpp>

using namespace tora;

/**
 * File::Path::mkpath(Str $name) : Undef
 *
 */
static SharedPtr<Value> File_Path_mkpath(VM *vm, Value *name) {
    try {
        boost::filesystem::create_directories(name->to_s()->c_str());
        return UndefValue::instance();
    } catch (boost::filesystem::filesystem_error& ex) {
        throw new ExceptionValue(ex.what());
    };
}

/**
 * File::Path::rmpath(Str $name) : Undef
 */
static SharedPtr<Value> File_Path_rmpath(VM *vm, Value *name) {
    try {
        boost::filesystem::remove_all(name->to_s()->c_str());
        return UndefValue::instance();
    } catch (boost::filesystem::filesystem_error& ex) {
        throw new ExceptionValue(ex.what());
    };
}

extern "C" {

void Init_File_Path(VM* vm) {
    SharedPtr<Package> pkg = vm->find_package("File::Path");
    pkg->add_method("mkpath", new CallbackFunction(File_Path_mkpath));
    pkg->add_method("rmpath", new CallbackFunction(File_Path_rmpath));
}

}
