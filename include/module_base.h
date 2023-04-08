#ifndef SRC_MODULE_BASE_H
#define SRC_MODULE_BASE_H

#include "fuse_wrapper.h"

class ModuleBase : public FuseWrapper {
public:
    virtual ~ModuleBase() = default;
    virtual void init() = 0;
    virtual void destroy() = 0;
};

#endif  // SRC_MODULE_BASE_H
