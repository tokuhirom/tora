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
    this->data.insert(std::make_pair(pkg->id(), pkg));
}

void PackageMap::set(SharedPtr<Package> &pkg) {
    this->data.insert(std::make_pair(pkg->id(), pkg));
}

void PackageMap::dump(VM *vm) {
    printf("[dump] PackageMap(%zd)\n", data.size());
    for (auto iter = data.begin(); iter != data.end(); iter++) {
        iter->second->dump(vm, 1);
    }
}

