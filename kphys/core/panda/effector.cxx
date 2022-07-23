#include <stdio.h>

#include "nodePath.h"

#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/converters.h"
#include "kphys/core/panda/effector.h"
#include "kphys/core/panda/ik.h"
#include "kphys/core/panda/types.h"


TypeHandle EffectorNode::_type_handle;

EffectorNode::EffectorNode(
        const char* name, unsigned int chain_length, unsigned int priority):
    PandaNode(name),
    _chain_length(chain_length),
    _priority(priority),
    _ik_effector(NULL) {}

unsigned int EffectorNode::get_chain_length() {
    return _chain_length;
}

NodePath EffectorNode::get_chain_root() {
    NodePath effector = NodePath::any_path(this);
    NodePath chain_root = effector;
    for (int i = 0; i < _chain_length + 2; i++)
        chain_root = chain_root.get_parent();
    return chain_root;
}

unsigned int EffectorNode::get_priority() {
    return _priority;
}

double EffectorNode::get_weight() {
    if (_ik_effector == NULL)
        return -1;
    else
        return _ik_effector->weight;
}

void EffectorNode::set_weight(double weight) {
    if (_ik_effector != NULL)
        _ik_effector->weight = weight;
}

unsigned int EffectorNode::rebuild_ik(struct ik_solver_t* ik_solver, unsigned int node_id) {
    NodePath effector = NodePath::any_path(this);
    NodePath parent_bone = effector.get_parent();

    if (is_bone(parent_bone)) {
        _ik_effector = ik_solver->effector->create();
        ik_solver->effector->attach(
            _ik_effector,
            ((BoneNode*) parent_bone.node())->get_ik_node());

        _ik_effector->chain_length = _chain_length;
        _ik_effector->weight = 1;
    }

    return node_id;
}

void EffectorNode::sync_p2ik_local() {
    NodePath effector = NodePath::any_path(this);
    NodePath armature = get_armature(effector);
    NodePath chain_root = get_chain_root();

    // IK positions and rotations are relative to chain root
    _ik_effector->target_position = LVecBase3_to_IKVec3(effector.get_pos(chain_root));
    _ik_effector->target_rotation = LQuaternion_to_IKQuat(effector.get_quat(chain_root));

    // save world-space position and rotation
    _position = effector.get_pos(armature);
    // _rotation = effector.get_quat(armature);
}

void EffectorNode::sync_ik2p_local() {
    NodePath effector = NodePath::any_path(this);
    NodePath armature = get_armature(effector);

    // restore world-space position and rotation
    effector.set_pos(armature, _position);
    effector.set_quat(armature, LQuaternion::ident_quat());
}

/**
 * Sync IK effector affected chain starting from IK effector.
 */
void EffectorNode::sync_ik2p_chain_reverse() {
    NodePath effector = NodePath::any_path(this);
    NodePath armature = get_armature(effector);

    if (get_weight()) {
        // set chain starting from effector
        NodePath chain[256];
        NodePath chain_root = effector;
        for (int i = 0; i < _chain_length + 2; i++) {
            chain_root = chain_root.get_parent();
            chain[i] = chain_root;
        }

        // update chain starting from chain root
        for (int i = _chain_length; i >= 0; i--)
            ((BoneNode*) chain[i].node())->sync_ik2p_local();
    }

    sync_ik2p_local();
}
