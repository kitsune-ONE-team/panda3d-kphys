#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/effector.h"
#include "kphys/core/panda/ik.h"
#include "kphys/core/panda/types.h"


#ifdef WITH_FABRIK
unsigned int children_rebuild_ik(
        NodePath np, struct ik_solver_t* ik_solver, unsigned int node_id) {
    int num_nps = np.get_num_children();
    for (int i = 0; i < num_nps; i++) {
        NodePath child_np = np.get_child(i);
        if (is_bone(child_np)) {
            node_id = ((BoneNode*) child_np.node())->rebuild_ik_recursive(ik_solver, node_id);
        } else if (is_effector(child_np)) {
            node_id = ((EffectorNode*) child_np.node())->rebuild_ik(ik_solver, node_id);
        }
    }
    return node_id;
}
#endif

void children_sync_p2ik(NodePath np) {
    int num_nps = np.get_num_children();
    for (int i = 0; i < num_nps; i++) {
        NodePath child_np = np.get_child(i);
        if (is_bone(child_np)) {
            ((BoneNode*) child_np.node())->sync_p2ik_recursive();
        } else if (is_effector(child_np)) {
            ((EffectorNode*) child_np.node())->sync_p2ik_local();
        }
    }
}
