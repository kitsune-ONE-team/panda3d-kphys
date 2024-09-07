#ifndef PANDA_SPRING_H
#define PANDA_SPRING_H

#ifdef CPPPARSER  // interrogate
#include <btBulletDynamicsCommon.h>  // panda3d parser-inc
class btGeneric6DofSpring2Constraint;
#else  // normal compiler
#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.h>
#endif

#include "bulletGenericConstraint.h"
#include "bulletRigidBodyNode.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

#ifdef CPPPARSER  // interrogate
BEGIN_PUBLISH
#endif
enum SpringDOF {  // Degree Of Freedom types:
    SPRING_DOF_TX = 0,  // translation along X axis
    SPRING_DOF_TY = 1,  // translation along Y axis
    SPRING_DOF_TZ = 2,  // translation along Z axis
    SPRING_DOF_RX = 3,  // rotation around X axis
    SPRING_DOF_RY = 4,  // rotation around Y axis
    SPRING_DOF_RZ = 5,  // rotation around Z axis
};
#ifdef CPPPARSER  // interrogate
END_PUBLISH
#endif


class EXPORT_CLASS SpringConstraint: public BulletConstraint {
PUBLISHED:
    SpringConstraint(
        const BulletRigidBodyNode* node_a,
        const BulletRigidBodyNode* node_b,
        const TransformState* frame_a,
        const TransformState* frame_b,
        bool use_frame_a_ref);
    ~SpringConstraint();

    void set_spring(int dof, bool enabled);
    void set_stiffness(int dof, double value);
    void set_damping(int dof, double value);
    void set_limit(int dof, double min, double max);
    void set_equilibrium_point(int dof);
    void set_equilibrium_point(int dof, double value);

    void set_linear_lower_limit(const LVecBase3 &linear_lower);
    void set_linear_upper_limit(const LVecBase3 &linearUpper);

public:
    virtual btTypedConstraint* ptr() const {
        return (btGeneric6DofSpringConstraint*) _constraint;
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
        register_type(_type_handle, "SpringConstraint", BulletConstraint::get_class_type());
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
