#include "vm.h"
#include "package_map.h"
#include "package.h"

using namespace tora;

PackageMap::iterator PackageMap::find(ID id) {
    return data.find(id);
}

PackageMap::iterator PackageMap::begin() {
    return data.end();
}

PackageMap::iterator PackageMap::end() {
    return data.end();
}

void PackageMap::set(Package* pkg) {
    ID id = pkg->id();
    this->data[id] = pkg;
}

void PackageMap::set(SharedPtr<Package> &pkg) {
    this->data[pkg->id()] = pkg;
}

void PackageMap::dump(VM *vm) {
    printf("[dump] PackageMap(%zd)\n", data.size());
    for (auto iter : data) {
        iter.second->dump(vm, 1);
    }
}

