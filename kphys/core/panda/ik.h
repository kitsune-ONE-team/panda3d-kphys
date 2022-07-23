#ifndef PANDA_IK_H
#define PANDA_IK_H

#include "nodePath.h"

#include "ik/ik.h"

/* #define IK_DEBUG */

unsigned int children_rebuild_ik(NodePath np, struct ik_solver_t* ik_solver, unsigned int node_id);
void children_sync_p2ik(NodePath np);

#endif
