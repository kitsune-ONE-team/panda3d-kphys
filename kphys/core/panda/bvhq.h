#ifndef PANDA_BVHQ_H
#define PANDA_BVHQ_H

#include "filename.h"
#include "nodePath.h"
#include "pvector.h"
#include "texture.h"

#include "kphys/core/panda/animation.h"


class EXPORT_CLASS BVHQJoint: public TypedReferenceCount, public Namable {
PUBLISHED:
    BVHQJoint(const char* name);
    ~BVHQJoint();
    unsigned long get_num_channels();
    char* get_channel(unsigned long i);

private:
    pvector<char*> _channels;

    static TypeHandle _type_handle;

public:
    void add_channel(char* name);

    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedReferenceCount::init_type();
        register_type(_type_handle, "BVHQJoint", TypedReferenceCount::get_class_type());
    }
    virtual TypeHandle get_type() const {
        return get_class_type();
    }
    virtual TypeHandle force_init_type() {
        init_type();
        return get_class_type();
    }
};


class EXPORT_CLASS BVHQ: public Animation {
PUBLISHED:
    BVHQ(const char* name, Filename filename, bool local_space=true, bool debug=false);
    ~BVHQ();

private:
    std::istream* _istream;
    pvector<PointerTo<BVHQJoint>> _hierarchy;

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
