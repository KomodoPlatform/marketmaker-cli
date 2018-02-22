#ifndef MARKETMAKER_CLI_PROPERTY_UTILS_H
#define MARKETMAKER_CLI_PROPERTY_UTILS_H

#include <property.h>

#include <cstring>
#include <iostream>

static inline std::ostream& operator<<(std::ostream &os, const PropertyGroup &group) {
    os << "PropertyGroup[" << group.size << "] {";
    for (int i = 0; i < group.size; i++) {
        if (i > 0)
            os << ", ";
        Property &prop = group.properties[i];
        os << "{ " << prop.key << "=" << prop.value << "}";
    }
    os << "}";
    return os;
}

static bool operator==(const Property &lhs, const Property &rhs);

static bool operator==(const PropertyGroup &lhs, const PropertyGroup &rhs);

bool operator==(const PropertyGroup &lhs, const PropertyGroup &rhs) {
    if (lhs.size != rhs.size) {
        return false;
    }
    for (int i = 0; i < lhs.size; i++) {
        bool cmp = lhs.properties[i] == rhs.properties[i];
        if (! cmp) {
            return false;
        }
    }
    return true;
}

bool operator==(const Property &lhs, const Property &rhs) {
    int cmp = strcmp(lhs.key, rhs.key);
    if (cmp == 0) {
        if ((lhs.value == NULL) || (rhs.value == NULL)) {
            return lhs.value == rhs.value;
        }
        cmp = strcmp(lhs.value, rhs.value);
    }
    return cmp == 0;
}

#endif //MARKETMAKER_CLI_PROPERTY_UTILS_H
