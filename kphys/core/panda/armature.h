#ifndef PANDA_ARMATURE_H
#define PANDA_ARMATURE_H

#include "nodePath.h"
#include "pandaNode.h"
#include "pmap.h"
#include "texture.h"

#ifdef CPPPARSER  // interrogate
union LMatrix4Array;
#else  // normal compiler
#ifdef WITH_FABRIK
#include "ik/ik.h"
#endif
#endif

#include "kphys/core/panda/ik.h"
#include "kphys/core/panda/types.h"
#include "kphys/core/panda/frame.h"


BEGIN_PUBLISH
enum IK_ENGINE {
    IK_ENGINE_IK = 0,     // https://github.com/TheComet/ik
    IK_ENGINE_CCDIK = 1,  // https://github.com/Germanunkol/CCD-IK-Panda3D
};
END_PUBLISH

class EXPORT_CLASS ArmatureNode: public PandaNode {
PUBLISHED:
    ArmatureNode(const char* name="armature");
    ~ArmatureNode();
    void set_raw_transform(bool is_enabled);
    void cleanup();
    void reset();
    void rebuild_bind_pose();
    void rebuild_bind_pose(NodePath np);
    void rebuild_ik(unsigned int ik_engine=IK_ENGINE_IK, unsigned int max_iterations=10);
    void update_ik();
    void update_ik(unsigned int priority);
    void update_shader_inputs();
    void update_shader_inputs(NodePath np);
    NodePath find_bone(const char* name);
    void apply(PointerTo<Frame> frame, bool local_space=true);

private:
    unsigned int _ik_engine;
    unsigned int _ik_max_iterations;
    bool _is_raw_transform;
    LMatrix4Array* _bone_init_local;  // initial local-space matrices
    LMatrix4Array* _bone_init_inv;  // initial world-space inverted (inverse bind) matrices
    LMatrix4Array* _bone_transform;  // current world-space matrices
    float _bone_id_tree[MAX_BONES][MAX_BONES];
    pmap<std::string, NodePath> _bones;
    PointerTo<Texture> _bone_init_inv_tex;
    PointerTo<Texture> _bone_id_tree_tex;
    PointerTo<Texture> _bone_transform_tex;
    PointerTo<Texture> _bone_prev_transform_tex;
    int _frame_transform_indices[MAX_BONES];
#ifdef WITH_FABRIK
    struct ik_solver_t* _ik_solver;  // [IK] solver engine
#endif
    static TypeHandle _type_handle;

    void _update_matrices(NodePath np, LMatrix4 parent_mat, bool is_current=true);
    void _update_id_tree(NodePath np);

public:
    void solve_ik(unsigned int priority);
    void sync_p2ik_recursive();
    void sync_ik2p_chains();

    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
#ifdef WITH_FABRIK
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
#endif

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
