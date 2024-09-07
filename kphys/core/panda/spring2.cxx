#include "kphys/core/panda/spring2.h"

#define SPRING_CLASS Spring2Constraint
#define BT_SPRING_CONSTRAINT btGeneric6DofSpring2Constraint
#define BT_SPRING_CONSTRAINT_ARGS
#include "kphys/core/panda/spring_common.cxx"
#undef SPRING_CLASS
#undef BT_SPRING_CONSTRAINT
#undef BT_SPRING_CONSTRAINT_ARGS

void Spring2Constraint::set_rotation_order(int order) {
    ((btGeneric6DofSpring2Constraint*) _constraint)->setRotationOrder((RotateOrder) order);
}

void Spring2Constraint::set_bounce(int dof, double value) {
    ((btGeneric6DofSpring2Constraint*) _constraint)->setBounce(dof, (btScalar) value);
}

void Spring2Constraint::set_motor(int dof, bool enabled) {
    ((btGeneric6DofSpring2Constraint*) _constraint)->enableMotor(dof, enabled);
}

void Spring2Constraint::set_motor_servo(int dof, bool enabled) {
    ((btGeneric6DofSpring2Constraint*) _constraint)->setServo(dof, enabled);
}

void Spring2Constraint::set_target_velocity(int dof, double value) {
    ((btGeneric6DofSpring2Constraint*) _constraint)->setTargetVelocity(dof, (btScalar) value);
}

void Spring2Constraint::set_servo_target(int dof, double value) {
    ((btGeneric6DofSpring2Constraint*) _constraint)->setServoTarget(dof, (btScalar) value);
}

void Spring2Constraint::set_max_motor_force(int dof, double value) {
    ((btGeneric6DofSpring2Constraint*) _constraint)->setMaxMotorForce(dof, (btScalar) value);
}
