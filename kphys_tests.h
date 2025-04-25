#include <cxxtest/TestSuite.h>

#include "kphys/core/panda/hit.h"
#include "kphys/core/panda/frame.h"
#include "kphys/core/panda/hitbox.h"
#include "bulletBoxShape.h"
#include "bulletGhostNode.h"
#include "pandaNode.h"
#include "transformState.h"
#include <stdio.h>


class ChaosTest : public CxxTest::TestSuite {
public:
    void test_core(void) {
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

    void testMix_Identity() {
        PointerTo<Frame> frameA = new Frame();
        PointerTo<Frame> frameB = new Frame();

        PointerTo<Frame> result = frameA->mix(frameB, 0.0);
        TS_ASSERT_EQUALS(result.p(), frameA.p());
    }

    void testMix_Other() {
        PointerTo<Frame> frameA = new Frame();
        PointerTo<Frame> frameB = new Frame();

        PointerTo<Frame> result = frameA->mix(frameB, 1.0);
        TS_ASSERT_EQUALS(result.p(), frameB.p());
    }

    void testMix_Interpolate() {
        PointerTo<Frame> frameA = new Frame();
        PointerTo<Frame> frameB = new Frame();

        // Set up some transforms in frameA and frameB
        ConstPointerTo<TransformState> transformA = TransformState::make_pos(LVecBase3(1, 2, 3));
        ConstPointerTo<TransformState> transformB = TransformState::make_pos(LVecBase3(4, 5, 6));
        frameA->set_transform("bone1", transformA, 0, 1.0);
        frameB->set_transform("bone1", transformB, 0, 1.0);

        PointerTo<Frame> result = frameA->mix(frameB, 0.5);

        // Check that the result is an interpolated transform
        ConstPointerTo<TransformState> transform = result->get_transform("bone1");
        TS_ASSERT_EQUALS(transform->has_pos(), true);
        // TS_ASSERT_EQUALS(transform->has_hpr(), false);
        // TS_ASSERT_EQUALS(transform->has_quat(), false);
        // LVecBase3 pos;
        // printf("%s\n", transform);
        // transform->get_pos();
        // TS_ASSERT_DELTA(pos.get_x(), 2.5, 1e-6);
        // TS_ASSERT_DELTA(pos.get_y(), 3.5, 1e-6);
        // TS_ASSERT_DELTA(pos.get_z(), 4.5, 1e-6);
    }

    // void testMix_MultipleBones() {
    //     Frame frameA;
    //     Frame frameB;
    //
    //     // Set up some transforms in frameA and frameB
    //     frameA.set_transform("bone1", TransformState::make_pos(LVecBase3(1, 2, 3)), 0, 1.0);
    //     frameA.set_transform("bone2", TransformState::make_hpr(LVecBase3(10, 20, 30)), 0, 1.0);
    //     frameB.set_transform("bone1", TransformState::make_pos(LVecBase3(4, 5, 6)), 0, 1.0);
    //     frameB.set_transform("bone2", TransformState::make_hpr(LVecBase3(40, 50, 60)), 0, 1.0);
    //
    //     Frame* result = frameA.mix(&frameB, 0.5);
    //
    //     // Check that the result is an interpolated transform for both bones
    //     ConstPointerTo<TransformState> transform1 = result->get_transform("bone1");
    //     TS_ASSERT_DELTA(transform1->get_pos().get_x(), 2.5, 1e-6);
    //     TS_ASSERT_DELTA(transform1->get_pos().get_y(), 3.5, 1e-6);
    //     TS_ASSERT_DELTA(transform1->get_pos().get_z(), 4.5, 1e-6);
    //
    //     ConstPointerTo<TransformState> transform2 = result->get_transform("bone2");
    //     TS_ASSERT_DELTA(transform2->get_hpr().get_x(), 25, 1e-6);
    //     TS_ASSERT_DELTA(transform2->get_hpr().get_y(), 35, 1e-6);
    //     TS_ASSERT_DELTA(transform2->get_hpr().get_z(), 45, 1e-6);
    // }
    //
};
