#include "kphys/core/panda/armature.h"
#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/effector.h"
#include "kphys/core/panda/types.h"


LMatrix4 get_matrix(LMatrix4Array array, unsigned short i) {
    float* matrix = array.matrices[i];
    return LMatrix4(
        matrix[0], matrix[1], matrix[2], matrix[3],
        matrix[4], matrix[5], matrix[6], matrix[7],
        matrix[8], matrix[9], matrix[10], matrix[11],
        matrix[12], matrix[13], matrix[14], matrix[15]);
}

void set_matrix(LMatrix4Array array, unsigned short i, LMatrix4 matrix) {
    memcpy(array.matrices[i], matrix.get_data(), sizeof(float) * 4 * 4);
}

int is_armature(NodePath np) {
    if (((PandaNode*) np.node())->is_of_type(ArmatureNode::get_class_type()))
        return 1;
    else
        return 0;
}

int is_bone(NodePath np) {
    if (((PandaNode*) np.node())->is_of_type(BoneNode::get_class_type()))
        return 1;
    else
        return 0;
}

int is_effector(NodePath np) {
    if (((PandaNode*) np.node())->is_of_type(EffectorNode::get_class_type()))
        return 1;
    else
        return 0;
}

NodePath get_armature(NodePath np) {
    while (np.has_parent()) {
        np = np.get_parent();
        if (is_armature(np))
            return np;
    }
    return NodePath::not_found();
}
