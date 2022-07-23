#include "kphys/core/panda/armature.h"
#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/effector.h"
#include "kphys/core/panda/types.h"


int is_armature(NodePath np) {
    if (dynamic_cast<const ArmatureNode*>(np.node()) != nullptr)
        return 1;
    else
        return 0;
}

int is_bone(NodePath np) {
    if (dynamic_cast<const BoneNode*>(np.node()) != nullptr)
        return 1;
    else
        return 0;
}

int is_effector(NodePath np) {
    if (dynamic_cast<const EffectorNode*>(np.node()) != nullptr)
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
