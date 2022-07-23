#include <stdio.h>

#include "nodePath.h"

#include "ik/ik.h"
#include "kphys/core/panda/armature.h"
#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/converters.h"
#include "kphys/core/panda/ik.h"
#include "kphys/core/panda/types.h"


TypeHandle BoneNode::_type_handle;

BoneNode::BoneNode(const char* name, unsigned int bone_id):
    PandaNode(name),
    _bone_id(bone_id),
    _ik_node(NULL) {}

unsigned int BoneNode::get_bone_id() {
    return _bone_id;
}

struct ik_node_t* BoneNode::get_ik_node() {
    return _ik_node;
}

unsigned int BoneNode::rebuild_ik(struct ik_solver_t* ik_solver, unsigned int node_id) {
    NodePath bone = NodePath::any_path(this);
    NodePath parent_bone = bone.get_parent();

    if (is_armature(parent_bone)) {
        _ik_node = ik_solver->node->create(node_id);
        node_id++;

    } else if (is_bone(parent_bone)) {
        // parent is bone, so this bone becomes child bone
        _ik_node = ik_solver->node->create_child(
            ((BoneNode*) parent_bone.node())->get_ik_node(), node_id);
        node_id++;
    }

    node_id = children_rebuild_ik(bone, ik_solver, node_id);
    return node_id;
}

void BoneNode::sync_p2ik_recursive() {
    NodePath bone = NodePath::any_path(this);

    _ik_node->position = LVecBase3_to_IKVec3(bone.get_pos());
    _ik_node->rotation = LQuaternion_to_IKQuat(bone.get_quat());

    children_sync_p2ik(bone);
}

void BoneNode::sync_ik2p_local() {
    NodePath bone = NodePath::any_path(this);

    // bone.set_pos(IKVec3_to_LVecBase3(_ik_node->position));
    bone.set_quat(IKQuat_to_LQuaternion(_ik_node->rotation));
}
