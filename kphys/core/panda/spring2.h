#ifndef PANDA_SPRING2_H
#define PANDA_SPRING2_H

#include "kphys/core/panda/spring.h"

#ifdef CPPPARSER  // interrogate
BEGIN_PUBLISH
#endif
enum SpringRotateOrder {
    SPRING_RO_XYZ = 0,
    SPRING_RO_XZY = 1,
    SPRING_RO_YXZ = 2,
    SPRING_RO_YZX = 3,
    SPRING_RO_ZXY = 4,
    SPRING_RO_ZYX = 5,
};
#ifdef CPPPARSER  // interrogate
END_PUBLISH
#endif


class EXPORT_CLASS Spring2Constraint: public BulletConstraint {
PUBLISHED:
    Spring2Constraint(
        const BulletRigidBodyNode* node_a,
        const BulletRigidBodyNode* node_b,
        const TransformState* frame_a,
        const TransformState* frame_b,
        bool use_frame_a_ref);
    ~Spring2Constraint();

    void set_spring(int dof, bool enabled);
    void set_stiffness(int dof, double value);
    void set_damping(int dof, double value);
    void set_limit(int dof, double min, double max);
    void set_equilibrium_point(int dof);
    void set_equilibrium_point(int dof, double value);

    void set_rotation_order(int order);
    void set_bounce(int dof, double value);
    void set_motor(int dof, bool enabled);
    void set_motor_servo(int dof, bool enabled);
    void set_target_velocity(int dof, double value);
    void set_servo_target(int dof, double value);
    void set_max_motor_force(int dof, double value);

public:
    virtual btTypedConstraint* ptr() const {
        return (btGeneric6DofSpring2Constraint*) _constraint;
    }

private:
    void* _constraint;
    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        BulletConstraint::init_type();
        register_type(_type_handle, "Spring2Constraint", BulletConstraint::get_class_type());
    }
    virtual TypeHandle get_type() const {
        return get_class_type();
    }
    virtual TypeHandle force_init_type() {
        init_type();
        return get_class_type();
    }
};

#endif
