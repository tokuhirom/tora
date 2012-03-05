#include "vm.h"
#include "package_map.h"

using namespace tora;

PackageMap::iterator PackageMap::find(ID id) {
    return data.find(id);
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
