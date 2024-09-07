TypeHandle SPRING_CLASS::_type_handle;

SPRING_CLASS::SPRING_CLASS(
        const BulletRigidBodyNode* node_a,
        const BulletRigidBodyNode* node_b,
        const TransformState* frame_a,
        const TransformState* frame_b,
        bool use_frame_a_ref) {
    btRigidBody* ptr_a = btRigidBody::upcast(node_a->get_object());
    btRigidBody* ptr_b = btRigidBody::upcast(node_b->get_object());

    btTransform trans_a = TransformState_to_btTrans(frame_a);
    btTransform trans_b = TransformState_to_btTrans(frame_b);

    _constraint = new BT_SPRING_CONSTRAINT(
        *ptr_a, *ptr_b, trans_a, trans_b BT_SPRING_CONSTRAINT_ARGS);
}

SPRING_CLASS::~SPRING_CLASS() {
    delete (BT_SPRING_CONSTRAINT*) _constraint;
}

void SPRING_CLASS::set_spring(int dof, bool enabled) {
    ((BT_SPRING_CONSTRAINT*) _constraint)->enableSpring(dof, enabled);
}

void SPRING_CLASS::set_stiffness(int dof, double value) {
    ((BT_SPRING_CONSTRAINT*) _constraint)->setStiffness(dof, (btScalar) value);
}

void SPRING_CLASS::set_damping(int dof, double value) {
    ((BT_SPRING_CONSTRAINT*) _constraint)->setDamping(dof, (btScalar) value);
}

/**
 * Set constraint limits for translation and rotation.

 * :param dof: Degree Of Freedom to set (0-5)
 * :type dof: int

 * :param min: min value (in degrees for rotation angles)
 * :type min: double

 * :param max: max value (in degrees for rotation angles)
 * :type max: double
 */
void SPRING_CLASS::set_limit(int dof, double min, double max) {
    if (dof < 3) {  // translation
        ((BT_SPRING_CONSTRAINT*) _constraint)->setLimit(dof, (btScalar) min, (btScalar) max);
    } else {  // rotation - degrees to radians
        ((BT_SPRING_CONSTRAINT*) _constraint)->setLimit(
            dof, (btScalar) (min * M_PI / 180), (btScalar) (max * M_PI / 180));
    }
}

void SPRING_CLASS::set_equilibrium_point(int dof) {
    ((BT_SPRING_CONSTRAINT*) _constraint)->setEquilibriumPoint(dof);
}

void SPRING_CLASS::set_equilibrium_point(int dof, double value) {
    ((BT_SPRING_CONSTRAINT*) _constraint)->setEquilibriumPoint(dof, (btScalar) value);
}
