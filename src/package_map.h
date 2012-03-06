#ifndef TORA_PACKAGE_MAP_H_
#define TORA_PACKAGE_MAP_H_

#include "prim.h"
#include "shared_ptr.h"

#include <map>

#include <boost/unordered_map.hpp>

namespace tora {

class Package;

class PackageMap : public Prim {
    boost::unordered_map<ID, SharedPtr<Package>> data;
public:
    typedef boost::unordered_map<ID, SharedPtr<Package>>::iterator iterator;

    PackageMap() : Prim() { }
    ~PackageMap() { }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] PackageMap(%zd)\n", data.size());
    }
    void set(Package* pkg);
    void set(SharedPtr<Package> &pkg);

    iterator find(ID id);
    iterator end();
};

};

#endif // TORA_PACKAGE_MAP_H_
