#ifndef TORA_PACKAGE_MAP_H_
#define TORA_PACKAGE_MAP_H_

#include "prim.h"
#include "shared_ptr.h"

#include <map>

#include <boost/unordered_map.hpp>

namespace tora {

class Package;
class VM;

class PackageMap {
    PRIM_DECL()
    boost::unordered_map<ID, SharedPtr<Package>> data;
public:
    typedef boost::unordered_map<ID, SharedPtr<Package>>::iterator iterator;

    PackageMap() : refcnt(0) { }
    ~PackageMap() { }
    void dump(VM *vm);
    void set(Package* pkg);
    void set(SharedPtr<Package> &pkg);

    iterator find(ID id);
    iterator begin();
    iterator end();
};

};

#endif // TORA_PACKAGE_MAP_H_
