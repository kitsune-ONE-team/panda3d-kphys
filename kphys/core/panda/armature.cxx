#include <stdio.h>
#include <string.h>

#include "kphys/core/panda/armature.h"
#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/effector.h"
#include "kphys/core/panda/ik.h"
#include "kphys/core/panda/types.h"


TypeHandle ArmatureNode::_type_handle;

ArmatureNode::ArmatureNode(const char* name):
        PandaNode(name),
        _ik_solver(NULL) {
    _bone_transform_tex.setup_buffer_texture(
        RGBA_MAT4_SIZE * MAX_BONES, Texture::T_float,
        Texture::F_rgba32, GeomEnums::UH_static);
}

void ArmatureNode::reset() {
    NodePath armature = NodePath::any_path(this);
    NodePathCollection effectors = armature.find_all_matches("**/+EffectorNode");
    NodePathCollection bones = armature.find_all_matches("**/+BoneNode");

    for (int i = 0; i < effectors.get_num_paths(); i++) {
        NodePath np = effectors.get_path(i);
        ((EffectorNode*) np.node())->sync_p2ik_local();
    }

    for (int i = 0; i < bones.get_num_paths(); i++) {
        NodePath np = bones.get_path(i);
        unsigned int bone_id = ((BoneNode*) np.node())->get_bone_id();
        np.set_mat(_bone_init_local.matrices[bone_id]);
    }

    for (int i = 0; i < effectors.get_num_paths(); i++) {
        NodePath np = effectors.get_path(i);
        ((EffectorNode*) np.node())->sync_ik2p_local();
    }
}

void ArmatureNode::rebuild_bind_pose() {
    NodePath armature = NodePath::any_path(this);
    _update_matrices(armature, LMatrix4::ident_mat(), 0);
}

void ArmatureNode::rebuild_ik() {
    if (_ik_solver != NULL) {
        ik.solver.destroy(_ik_solver);
    }
    _ik_solver = ik.solver.create(IK_FABRIK);
    _ik_solver->flags &= ~IK_ENABLE_CONSTRAINTS;
    _ik_solver->flags &= ~IK_ENABLE_TARGET_ROTATIONS;
    _ik_solver->flags |= IK_ENABLE_JOINT_ROTATIONS;
    _ik_solver->max_iterations = 5;

    NodePath armature = NodePath::any_path(this);
    children_rebuild_ik(armature, _ik_solver, 0);
}

/**
 * Update IK for the all effectors.
 */
void ArmatureNode::update_ik() {
    NodePath armature = NodePath::any_path(this);

    unsigned int priority = 0, max_priority = 0;
    NodePathCollection nps = armature.find_all_matches("**/+EffectorNode");
    for (int i = 0; i < nps.get_num_paths(); i++) {
        NodePath np = nps.get_path(i);
        priority = ((EffectorNode*) np.node())->get_priority();
        if (priority > max_priority)
            max_priority = priority;
    }

    for (int priority = 0; priority <= max_priority; priority++)
        update_ik(priority);
}

/**
 * Update IK for the effectors with specified priority.
 */
void ArmatureNode::update_ik(unsigned int priority) {
    NodePath armature = NodePath::any_path(this);

    NodePathCollection nps = armature.find_all_matches("**/+EffectorNode");
    for (int i = 0; i < nps.get_num_paths(); i++) {
        NodePath np = nps.get_path(i);
        if (((EffectorNode*) np.node())->get_priority() == priority)
            ((EffectorNode*) np.node())->set_weight(1);
        else
            ((EffectorNode*) np.node())->set_weight(0);
    }

    for (int i = 0; i < 10; i++) {
        // write all bones
        sync_p2ik_recursive();

        // solve IK problem
        solve_ik();

        // read bones from active chains
        sync_ik2p_chains();
    }
}

void ArmatureNode::update_shader_inputs() {
    NodePath armature = NodePath::any_path(this);
    _update_matrices(armature, LMatrix4::ident_mat(), 1);

    PTA_uchar data = _bone_transform_tex.modify_ram_image();
    memcpy(data.p(), _bone_transform.data, sizeof(_bone_transform.data));
    armature.set_shader_input("bone_transform_tex", &_bone_transform_tex);
}

/**
 * Fill bone matrices array with world-space bone matrices
 * by recursively walking through the node graph.
 */
void ArmatureNode::_update_matrices(NodePath np, LMatrix4 parent_mat, int is_current) {
    LMatrix4 mat = LMatrix4::ident_mat();

    if (is_bone(np)) {
        unsigned int bone_id = ((BoneNode*) np.node())->get_bone_id();
        mat = np.get_mat() * parent_mat;  // get world-space matrix

        if (is_current) {  // current matrices
            // https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_020_Skins.md#the-joint-matrices
            _bone_transform.matrices[bone_id] = _bone_init_inv.matrices[bone_id] * mat;

        } else {  // initial matrices
            _bone_init_local.matrices[bone_id] = np.get_mat();
            _bone_init_inv.matrices[bone_id].invert_from(mat);
        }
    }

    for (int i = 0; i < np.get_num_children(); i++) {
        NodePath child_np = np.get_child(i);
        if (is_bone(child_np)) {
            _update_matrices(child_np, mat, is_current);
        }
    }
}

/**
 * Solve Inverse Kinematics problem.
 */
void ArmatureNode::solve_ik() {
    NodePath armature = NodePath::any_path(this);
    NodePath root_bone = NodePath::not_found();
    for (int i = 0; i < armature.get_num_children(); i++) {
        NodePath child = armature.get_child(i);
        if (is_bone(child)) {
            root_bone = child;
            break;
        }
    }

    if (!root_bone) {
        return;
    }

    ik.solver.set_tree(_ik_solver, ((BoneNode*) root_bone.node())->get_ik_node());
    ik.solver.rebuild(_ik_solver);
    ik.solver.solve(_ik_solver);
    ik.solver.unlink_tree(_ik_solver);
}

/**
 * Recursively sync all Panda3D nodes to IK nodes and effectors.
 */
void ArmatureNode::sync_p2ik_recursive() {
    NodePath armature = NodePath::any_path(this);
    children_sync_p2ik(armature);
}

/**
 * Sync IK effector affected chains to Panda3D related nodes.
 */
void ArmatureNode::sync_ik2p_chains() {
    NodePath armature = NodePath::any_path(this);
    NodePathCollection nps = armature.find_all_matches("**/+EffectorNode");
    for (int i = 0; i < nps.get_num_paths(); i++) {
        NodePath np = nps.get_path(i);
        ((EffectorNode*) np.node())->sync_ik2p_chain_reverse();
    }
}
