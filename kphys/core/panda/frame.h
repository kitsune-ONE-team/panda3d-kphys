#ifndef PANDA_FRAME_H
#define PANDA_FRAME_H

#include "nodePath.h"
#include "pandaNode.h"
#include "pmap.h"
#include "pvector.h"
#include "transformState.h"

#include "kphys/core/panda/types.h"


class EXPORT_CLASS Frame: public PandaNode {
PUBLISHED:
    Frame(const char* name);
    Frame(const char* name, unsigned long iframe);
    ~Frame();
    unsigned int get_num_transforms();
    char* get_bone_name(unsigned int i);
    ConstPointerTo<TransformState> get_transform(unsigned int i);
    ConstPointerTo<TransformState> get_transform(char* name);

private:
    unsigned long _iframe;
    pvector<char*> _bone_names;
    pmap<char*, ConstPointerTo<TransformState>> _transforms;

    static TypeHandle _type_handle;

public:
    void add_transform(char* name, ConstPointerTo<TransformState> transform);

    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        PandaNode::init_type();
        register_type(_type_handle, "Frame", PandaNode::get_class_type());
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
