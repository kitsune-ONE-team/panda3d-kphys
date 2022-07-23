#ifndef PANDA_BONE_H
#define PANDA_BONE_H

#include "pandaNode.h"

#ifdef CPPPARSER  // interrogate
#else  // normal compiler
#include "ik/ik.h"
#endif

#include "kphys/core/panda/ik.h"


class EXPORT_CLASS BoneNode: public PandaNode {
PUBLISHED:
    explicit BoneNode(const char* name, unsigned int bone_id);
    unsigned int get_bone_id();

private:
    unsigned int _bone_id;
    struct ik_node_t* _ik_node;
    static TypeHandle _type_handle;

public:
    struct ik_node_t* get_ik_node();
    unsigned int rebuild_ik(struct ik_solver_t* ik_solver, unsigned int node_id);
    void sync_p2ik_recursive();
    void sync_ik2p_local();

    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        PandaNode::init_type();
        register_type(_type_handle, "BoneNode", PandaNode::get_class_type());
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
