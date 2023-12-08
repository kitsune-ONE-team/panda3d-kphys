#include <cxxtest/TestSuite.h>

#include "kphys/core/panda/hit.h"
#include "kphys/core/panda/hitbox.h"
#include "bulletBoxShape.h"
#include "bulletGhostNode.h"
#include "pandaNode.h"
#include "transformState.h"
#include <stdio.h>


class ChaosTest : public CxxTest::TestSuite {
public:
    void test_something(void) {
        NodePath root(new PandaNode("root"));
        NodePath ray = root.attach_new_node(new PandaNode("ray"));
        NodePath ghost = root.attach_new_node(new BulletGhostNode("ghost"));
        BulletBoxShape* box = new BulletBoxShape(LVecBase3(1, 1, 1));
        ((BulletGhostNode*) ghost.node())->add_shape(box);
        NodePath hitbox = root.attach_new_node(new HitboxNode(
            "hitbox", (BulletGhostNode*) ghost.node()));
        /* root.ls(); */

        HitboxNode* hb = (HitboxNode*) hitbox.node();
        PT(Hit) hit;

        ray.set_pos(0.2, -10, 0.1);
        hit = hb->ray_test(ray, LVecBase3(0.1, 9000.0, 0.1));
        TS_ASSERT(hit != nullptr);

        LMatrix3 aabb = hit->get_aabb();
        TS_ASSERT_DELTA(aabb.get_cell(ROW_X, COL_MIN), -1.16, 0.001);
        TS_ASSERT_DELTA(aabb.get_cell(ROW_X, COL_MAX), 0.76, 0.001);

        TS_ASSERT_DELTA(aabb.get_cell(ROW_Z, COL_MIN), -1.06, 0.001);
        TS_ASSERT_DELTA(aabb.get_cell(ROW_Z, COL_MAX), 0.86, 0.001);

        TS_ASSERT_DELTA(aabb.get_cell(ROW_Y, COL_MIN), 9.04, 0.001);
        TS_ASSERT_DELTA(aabb.get_cell(ROW_Y, COL_MAX), 10.96, 0.001);

        ray.set_pos(2, -10, 1);
        hit = hb->ray_test(ray, LVecBase3(0.1, 9000.0, 0.1));
        TS_ASSERT(hit == nullptr);
    }
};
