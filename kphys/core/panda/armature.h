#ifndef PANDA_ARMATURE_H
#define PANDA_ARMATURE_H

#include "nodePath.h"
#include "pandaNode.h"
#include "texture.h"

#ifdef CPPPARSER  // interrogate
union LMatrix4Array;
#else  // normal compiler
#include "ik/ik.h"
#endif

#include "kphys/core/panda/ik.h"
#include "kphys/core/panda/types.h"


class EXPORT_CLASS ArmatureNode: public PandaNode {
PUBLISHED:
    ArmatureNode(const char* name="armature");
    ~ArmatureNode();
    void reset();
    void rebuild_bind_pose();
    void rebuild_ik();
    void update_ik();
    void update_ik(unsigned int priority);
    void update_shader_inputs();

private:
    // initial local-space matrices
    LMatrix4Array _bone_init_local;
    // initial world-space inverted (inverse bind) matrices
    LMatrix4Array _bone_init_inv;
    // current world-space matrices
    LMatrix4Array _bone_transform;
    Texture* _bone_transform_tex;
    struct ik_solver_t* _ik_solver;
    static TypeHandle _type_handle;

    void _update_matrices(NodePath np, LMatrix4 parent_mat, int is_current);

public:
    void solve_ik(unsigned int priority);
    void sync_p2ik_recursive();
    void sync_ik2p_chains();

    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        if (ik.init() == IK_OK) {
            if (ik.log.init() == IK_OK) {
#ifdef IK_DEBUG
                ik.log.set_severity(IK_DEBUG);
#else
                ik.log.set_severity(IK_FATAL);
#endif
            } else {
                // TODO: assert
            }
        } else {
            // TODO: assert
        }

        PandaNode::init_type();
        register_type(_type_handle, "ArmatureNode", PandaNode::get_class_type());
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
