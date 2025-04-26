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

    void test_copy_into(void) {
        PointerTo<Frame> frame_a = new Frame();
        PointerTo<Frame> frame_b = new Frame();

        // Set up some transforms in frame_a
        frame_a->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(1, 2, 3)), TRANSFORM_POS);
        frame_a->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(45, 0, 0)), TRANSFORM_HPR);

        // Copy frame_a into frame_b
        frame_a->copy_into(*frame_b.p());

        // Verify that frame_b has the same transforms as frame_a
        TS_ASSERT_EQUALS(frame_b->get_transform("bone1"), frame_a->get_transform("bone1"));
        TS_ASSERT_EQUALS(frame_b->get_transform("bone2"), frame_a->get_transform("bone2"));

        frame_a->reset();
        frame_b->reset();
    }

    void test_mix_into(void) {
        PointerTo<Frame> frame_a = new Frame();
        PointerTo<Frame> frame_b = new Frame();
        PointerTo<Frame> frame_dest = new Frame();

        // Set up some transforms in frame_a and frame_b
        frame_a->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(1, 2, 3)), TRANSFORM_POS);
        frame_a->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(45, 0, 0)), TRANSFORM_HPR);
        frame_b->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(4, 5, 6)), TRANSFORM_POS);
        frame_b->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(90, 0, 0)), TRANSFORM_HPR);

        // Mix frame_a and frame_b into frame_dest with a factor of 0.5
        frame_a->mix_into(*frame_dest.p(), frame_b, 0.5);

        // Verify that frame_dest has the mixed transforms
        TS_ASSERT_EQUALS(frame_dest->get_num_transforms(), 2);
        LVecBase3 expected_pos = LVecBase3(2.5, 3.5, 4.5);
        LVecBase3 expected_hpr = LVecBase3(67.5, 0, 0);
        TS_ASSERT_EQUALS(frame_dest->get_transform("bone1")->get_pos(), expected_pos);
        TS_ASSERT_EQUALS(frame_dest->get_transform("bone2")->get_hpr(), expected_hpr);

        frame_a->reset();
        frame_b->reset();
        frame_dest->reset();
    }

    void test_mix_into_combine(void) {
        PointerTo<Frame> frame_a = new Frame();
        PointerTo<Frame> frame_b = new Frame();
        PointerTo<Frame> frame_dest = new Frame();

        // Set up some transforms in frame_a and frame_b
        frame_a->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(1, 2, 3)), TRANSFORM_POS);
        frame_b->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(90, 0, 0)), TRANSFORM_HPR);

        // Mix frame_a and frame_b into frame_dest
        frame_a->mix_into(*frame_dest.p(), frame_b);

        // Verify that frame_dest has the mixed transforms
        TS_ASSERT_EQUALS(frame_dest->get_num_transforms(), 1);
        LVecBase3 expected_hpr = LVecBase3(90, 0, 0);
        TS_ASSERT_EQUALS(frame_dest->get_transform("bone2")->get_hpr(), expected_hpr);

        frame_a->reset();
        frame_b->reset();
        frame_dest->reset();
    }

    void test_mix_into_factor_0(void) {
        PointerTo<Frame> frame_a = new Frame();
        PointerTo<Frame> frame_b = new Frame();
        PointerTo<Frame> frame_dest = new Frame();

        // Set up some transforms in frame_a and frame_b
        frame_a->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(1, 2, 3)), TRANSFORM_POS);
        frame_a->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(45, 0, 0)), TRANSFORM_HPR);
        frame_b->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(4, 5, 6)), TRANSFORM_POS);
        frame_b->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(90, 0, 0)), TRANSFORM_HPR);

        // Mix frame_a and frame_b into frame_dest with a factor of 0.0
        frame_a->mix_into(*frame_dest.p(), frame_b, 0.0);

        // Verify that frame_dest is the same as frame_a
        TS_ASSERT_EQUALS(frame_dest->get_transform("bone1"), frame_a->get_transform("bone1"));
        TS_ASSERT_EQUALS(frame_dest->get_transform("bone2"), frame_a->get_transform("bone2"));

        frame_a->reset();
        frame_b->reset();
        frame_dest->reset();
    }

    void test_mix_into_factor_1(void) {
        PointerTo<Frame> frame_a = new Frame();
        PointerTo<Frame> frame_b = new Frame();
        PointerTo<Frame> frame_dest = new Frame();

        // Set up some transforms in frame_a and frame_b
        frame_a->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(1, 2, 3)), TRANSFORM_POS);
        frame_a->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(45, 0, 0)), TRANSFORM_HPR);
        frame_b->set_transform(
            "bone1", TransformState::make_pos(LVecBase3(4, 5, 6)), TRANSFORM_POS);
        frame_b->set_transform(
            "bone2", TransformState::make_hpr(LVecBase3(90, 0, 0)), TRANSFORM_HPR);

        // Mix frame_a and frame_b into frame_dest with a factor of 1.0
        frame_a->mix_into(*frame_dest.p(), frame_b, 1.0);

        // Verify that frame_dest is the same as frame_b
        TS_ASSERT_EQUALS(frame_dest->get_transform("bone1"), frame_b->get_transform("bone1"));
        TS_ASSERT_EQUALS(frame_dest->get_transform("bone2"), frame_b->get_transform("bone2"));

        frame_a->reset();
        frame_b->reset();
        frame_dest->reset();
    }
};
