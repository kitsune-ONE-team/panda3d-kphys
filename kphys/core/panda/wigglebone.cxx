#include "kphys/core/panda/frame.h"
#include "kphys/core/panda/wigglebone.h"


TypeHandle WiggleBoneNode::_type_handle;

/**
 * Adds jiggle physics to the bone.
 *
 * It reacts to animated or global motion as if it's connected with a rubber
 * band to its initial position. As it reacts to acceleration instead of velocity,
 * bones of constantly moving objects will not "lag behind" and have a more realistic behaviour.
 * Based on WiggleBone addon for Godot by Simon Schoenenberger:
 * https://github.com/detomon/wigglebone
 */
WiggleBoneNode::WiggleBoneNode(const std::string name, unsigned int bone_id):
    BoneNode(name, bone_id) {
    // properties
    _wb_mode = WIGGLEBONE_MODE_ROTATION;
    _stiffness = 0.1;
    _damping = 0.1;
    _gravity = VECTOR3_ZERO;
    _length = 0.1;
    _max_distance = 0.1;
    _max_degrees = 60.0;

    _point_mass_reset();
    _should_reset = true;
    _acceleration = VECTOR3_ZERO;  // local bone acceleration at mass center
    _prev_mass_center = VECTOR3_ZERO;
    _prev_velocity = VECTOR3_ZERO;
}

/**
 * The wiggle mode.
 */
int WiggleBoneNode::get_wigglebone_mode() {
    return _wb_mode;
}

void WiggleBoneNode::set_wigglebone_mode(int value) {
    _wb_mode = value;
    reset();
}

/**
 * Rendency of bone to return to pose position.
 */
double WiggleBoneNode::get_stiffness() {
    return _stiffness;
}

void WiggleBoneNode::set_stiffness(double value) {
    _stiffness = value;
}

/**
 * Reduction of motion.
 */
double WiggleBoneNode::get_damping() {
    return _damping;
}

void WiggleBoneNode::set_damping(double value) {
    _damping = value;
}

/**
 * Gravity pulling at mass center.
 */
LVecBase3 WiggleBoneNode::get_gravity() {
    return _gravity;
}

void WiggleBoneNode::set_gravity(const LVecBase3& value) {
    _gravity = value;
}

/**
 * Bone length.
 */
double WiggleBoneNode::get_length() {
    return _length;
}

void WiggleBoneNode::set_length(double value) {
    _length = MAX(0.01, value);
    reset();
}

/**
 * Maximum distance the bone can move around its pose position.
 */
double WiggleBoneNode::get_max_distance() {
    return _max_distance;
}

void WiggleBoneNode::set_max_distance(double value) {
    _max_distance = value;
}

/**
 * Maximum rotation relative to pose position.
 */
double WiggleBoneNode::get_max_degrees() {
    return _max_degrees;
}

void WiggleBoneNode::set_max_degrees(double value) {
    _max_degrees = value;
}

/**
 * Process bone transform input to calculate acceleration,
 * update it using calculated point mass and apply it to bone.
 */
void WiggleBoneNode::_process(NodePath root, LMatrix4 bone_pose, double delta) {
    NodePath np = NodePath::any_path(this);

    // may be 0.0 on first frame
    if (delta == 0.0)
        delta = 1.0 / 60.0;

    // panda -> phys
    LMatrix4 global_bone_pose = np.get_parent().get_mat(root) * bone_pose;
    LMatrix4 invert_global_bone_pose;
    invert_global_bone_pose.invert_from(global_bone_pose);
    _global_to_pose = invert_global_bone_pose.get_upper_3();

    LVecBase3 new_acceleration = _update_acceleration(global_bone_pose, delta);
    _acceleration = mix3(_acceleration, new_acceleration, ACCELERATION_WEIGHT);

    // adjust for varying framerates
    // this is only an approximation
    double delta_factor = log(delta * 60.0) / log(2.0) + 1.0;
    _acceleration /= MIN(MAX(delta_factor, 1.0), 3.0);  // TODO: adjust for rates higher than 60 fps

    // phys -> panda
    np.set_mat(_pose() * bone_pose);
}


void WiggleBoneNode::_physics_process(double delta) {
    _solve(_global_to_pose, _acceleration, delta);
}

void WiggleBoneNode::reset() {
    _point_mass_reset();
    _should_reset = true;
}

/**
 * Update acceleration, save current mass center and velocity.
 */
LVecBase3 WiggleBoneNode::_update_acceleration(
        const LMatrix4& global_bone_pose, double delta) {
    LVecBase3 mass_center = VECTOR3_ZERO;

    switch (_wb_mode) {
    case WIGGLEBONE_MODE_ROTATION:
        mass_center = VECTOR3_UP * _length;
        break;
    default:
        break;
    }

    mass_center = global_bone_pose.xform_point(mass_center);
    LVecBase3 delta_mass_center = _prev_mass_center - mass_center;
    _prev_mass_center = mass_center;

    if (_should_reset) {
        delta_mass_center = VECTOR3_ZERO;
    }

    LVecBase3 global_velocity = delta_mass_center / delta;
    LVecBase3 acceleration = global_velocity - _prev_velocity;
    _prev_velocity = global_velocity;

    if (_should_reset) {
        _acceleration = VECTOR3_ZERO;
        _should_reset = false;
    }

    return acceleration;
}

void WiggleBoneNode::_solve(
        const LMatrix3& global_to_local, const LVecBase3& acceleration, double delta) {
    LVecBase3 global_force = _gravity + const_force_global;
    LVecBase3 local_force = global_to_local.xform(global_force) + const_force_local;

    double mass_distance = _length;
    LVecBase3 local_acc = global_to_local.xform(acceleration);

    switch (_wb_mode) {
    case WIGGLEBONE_MODE_ROTATION:
        local_force = _project_to_vector_plane(VECTOR3_ZERO, mass_distance, local_force);
        local_acc = _project_to_vector_plane(VECTOR3_ZERO, mass_distance, local_acc);
        break;
    default:
        break;
    }

    _point_mass_accelerate(local_acc, delta);
    _point_mass_apply_force(local_force);
    _point_mass_solve(_stiffness, _damping, delta);
}

LMatrix4 WiggleBoneNode::_pose() {
    LMatrix4 pose;
    LMatrix3 mat;
    LVecBase3 mass_constrained, mass_local;
    double mass_distance, angular_offset, angular_limit, k;
    LQuaternion relative_rotation;

    switch (_wb_mode) {
    case WIGGLEBONE_MODE_ROTATION:
        mass_distance = _length;
        angular_offset = (
            mat.rotate_mat(_max_degrees).xform_point(VECTOR2_RIGHT) -
            VECTOR2_RIGHT).length();
        angular_limit = angular_offset * mass_distance;
        k = angular_limit * SOFT_LIMIT_FACTOR;
        mass_constrained = _clamp_length_soft(
            _point_mass.get_row(POINT_MASS_P), 0.0, angular_limit, k);

        mass_local = (VECTOR3_UP * _length) + mass_constrained;

        relative_rotation = quat_shortest_arc(
            VECTOR3_UP, mass_local.normalized());

        relative_rotation.extract_to_matrix(pose);
        break;

    case WIGGLEBONE_MODE_DISLOCATION:
        k = _max_distance * SOFT_LIMIT_FACTOR;
        mass_constrained = _clamp_length_soft(
            _point_mass.get_row(POINT_MASS_P), 0.0, _max_distance, k);

        pose.translate_mat(mass_constrained);
        break;
    default:
        break;
    }

    return pose;
}

void WiggleBoneNode::update(NodePath root, LMatrix4 bone_pose, double delta) {
    _physics_process(delta);
    _process(root, bone_pose, delta);
}

LVecBase3 _project_to_vector_plane(
        const LVecBase3& vector, double length, const LVecBase3& point) {
    return LPlane(vector.normalized(), vector.normalized() * length).project(point);
}

LVecBase3 _clamp_length_soft(
        const LVecBase3& v, double min_length, double max_length, double k) {
    return v.normalized() * _smin(MAX(min_length, v.length()), max_length, k);
}

/**
 * https://iquilezles.org/articles/smin/
 */
double _smin(double a, double b, double k) {
    double h = MAX(0.0, k - abs(a - b));
    return MIN(a, b) - h * h / (4.0 * k);
}

/**
 * Constructs a Quaternion representing the shortest arc between arc_from and arc_to.
 * These can be imagined as two points intersecting a sphere's surface,
 * with a radius of 1.0.
 */
LQuaternion quat_shortest_arc(const LVecBase3& arc_from, const LVecBase3& arc_to) {
    LVecBase3 v1 = arc_from.normalized();
    LVecBase3 v2 = arc_to.normalized();

    // compute the axis of rotation (cross product of v1 and v2)
    LVecBase3 axis = v1.cross(v2);

    // compute the dot product to get the cosine of the angle between v1 and v2
    double cos_angle = v1.dot(v2);

    double angle;

    if (ISCLOSE(cos_angle, 1.0)) {
        // if the vectors are already aligned, return the identity quaternion
        return LQuaternion(1, 0, 0, 0);

    } else if (ISCLOSE(cos_angle, -1.0)) {
        // if the vectors are opposite, rotate 180 degrees around any orthogonal vector
        angle = M_PI;

        // find an orthogonal vector
        if (v1.get_x() != 0 || v1.get_y() != 0) {
            axis = LVecBase3(-v1.get_y(), v1.get_x(), 0);
        } else {
            axis = LVecBase3(0, -v1.get_z(), v1.get_y());
        }

    } else {
        // compute the angle of rotation
        angle = acos(cos_angle);
    }

    // normalize the axis
    axis = axis.normalized();

    // construct the quaternion
    double half_angle = angle / 2.0;

    return LQuaternion(
        cos(half_angle),
        axis.get_x() * sin(half_angle),
        axis.get_y() * sin(half_angle),
        axis.get_z() * sin(half_angle));
}

void WiggleBoneNode::_point_mass_solve(double stiffness, double damping, double delta) {
    LVecBase3 p = _point_mass.get_row(POINT_MASS_P);
    LVecBase3 v = _point_mass.get_row(POINT_MASS_V);
    LVecBase3 a = _point_mass.get_row(POINT_MASS_A);

    // inertia
    v = v * (1.0 - damping) + a * delta;
		p += v;
		a = VECTOR3_ZERO;

    // constraint
		v -= p * stiffness;

    _point_mass.set_row(POINT_MASS_P, p);
    _point_mass.set_row(POINT_MASS_V, v);
    _point_mass.set_row(POINT_MASS_A, a);
}

void WiggleBoneNode::_point_mass_accelerate(const LVecBase3& acc, double delta) {
    LVecBase3 v = _point_mass.get_row(POINT_MASS_V);
		v += acc * delta;
    _point_mass.set_row(POINT_MASS_V, v);
}

void WiggleBoneNode::_point_mass_apply_force(const LVecBase3& force) {
    LVecBase3 a = _point_mass.get_row(POINT_MASS_A);
		a += force;
    _point_mass.set_row(POINT_MASS_A, a);
}

void WiggleBoneNode::_point_mass_reset() {
    _point_mass.set_row(POINT_MASS_P, VECTOR3_ZERO);
    _point_mass.set_row(POINT_MASS_V, VECTOR3_ZERO);
    _point_mass.set_row(POINT_MASS_A, VECTOR3_ZERO);
}
