#ifndef PANDA_BVHQ_H
#define PANDA_BVHQ_H

#include <vector>

#include "nodePath.h"
#include "texture.h"

#include "kphys/core/panda/animation.h"


class BVHQJoint {
public:
    char* name;
    std::vector<char*> channels;
};


class EXPORT_CLASS BVHQ: public Animation {
PUBLISHED:
    BVHQ(const char* name, const char* data);
    ~BVHQ();

private:
    unsigned long _offset;
    char* _data;
    std::vector<BVHQJoint*> _hierarchy;

    char _readword(char* word, unsigned long size);

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        Animation::init_type();
        register_type(_type_handle, "BVHQ", PandaNode::get_class_type());
    }
    virtual TypeHandle get_type() const {
        return get_class_type();
    }
    virtual TypeHandle force_init_type() {
        init_type();
        return get_class_type();
    }
};

#endif
