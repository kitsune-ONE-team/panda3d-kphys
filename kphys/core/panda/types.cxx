#include "bulletRigidBodyNode.h"

#include "kphys/core/panda/animator.h"
#include "kphys/core/panda/armature.h"
#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/wigglebone.h"
#include "kphys/core/panda/effector.h"
#include "kphys/core/panda/multianimator.h"
#include "kphys/core/panda/types.h"


LMatrix4 get_matrix(LMatrix4Array* array, unsigned short i) {
    return array->matrices[i];
}

void set_matrix(LMatrix4Array* array, unsigned short i, LMatrix4 matrix) {
    array->matrices[i] = matrix;
}

bool is_armature(NodePath np) {
    return ((PandaNode*) np.node())->is_of_type(ArmatureNode::get_class_type());
}

bool is_bone(NodePath np) {
    return ((PandaNode*) np.node())->is_of_type(BoneNode::get_class_type());
}

bool is_wiggle_bone(NodePath np) {
    return ((PandaNode*) np.node())->is_of_type(WiggleBoneNode::get_class_type());
}

bool is_any_bone(NodePath np) {
    return is_bone(np) || is_wiggle_bone(np);
}

bool is_rigid_body(NodePath np) {
    return ((PandaNode*) np.node())->is_of_type(BulletRigidBodyNode::get_class_type());
}

bool is_effector(NodePath np) {
    return ((PandaNode*) np.node())->is_of_type(EffectorNode::get_class_type());
}

bool is_animator(NodePath np) {
    return (
        ((PandaNode*) np.node())->is_of_type(AnimatorNode::get_class_type()) ||
        ((PandaNode*) np.node())->is_of_type(MultiAnimatorNode::get_class_type()));
}

NodePath get_armature(NodePath np) {
    while (np.has_parent()) {
        np = np.get_parent();
        if (is_armature(np))
            return np;
    }
    return NodePath::not_found();
}
