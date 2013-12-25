#include "../object.h"
#include "../vm.h"
#include "../value/array.h"
#include "../value/object.h"
#include "../value/code.h"
#include "../value/class.h"
#include "../value/file_package.h"
#include "../peek.h"
#include "../symbols.gen.h"

using namespace tora;

/**
 * class FilePackage
 *
 * This is a FilePackage class.
 */

void tora::Init_FilePackage(VM* vm) {
  SharedPtr<ClassValue> klass = new ClassValue(vm, SYMBOL_FILEPACKAGE_CLASS);
  vm->add_builtin_class(klass);
}
