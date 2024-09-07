#ifndef PANDA_WIGGLEBONE_H
#define PANDA_WIGGLEBONE_H

#define VECTOR2_RIGHT LVecBase2(1, 0)
#define VECTOR3_ZERO LVecBase3(0, 0, 0)
/* #define VECTOR3_UP LVecBase3(0, 0, 1) */
#define VECTOR3_UP LVecBase3(0, 1, 0)  // in godot
#define POINT_MASS_P 0
#define POINT_MASS_V 1
#define POINT_MASS_A 2
#define ACCELERATION_WEIGHT 0.5
#define SOFT_LIMIT_FACTOR 0.5
#define CMP_EPSILON 0.00001
// A constant global force.
#define const_force_global VECTOR3_ZERO
// A constant local force relative to the bone's rest pose.
#define const_force_local VECTOR3_ZERO

BEGIN_PUBLISH
enum WIGGLEBONE_MODE {
    WIGGLEBONE_MODE_ROTATION = 0,
    WIGGLEBONE_MODE_DISLOCATION = 1,
};
END_PUBLISH

#include "kphys/core/panda/bone.h"
#include "kphys/core/panda/types.h"


LVecBase3 _project_to_vector_plane(
    const LVecBase3& vector, double length, const LVecBase3& point);
LVecBase3 _clamp_length_soft(
    const LVecBase3& v, double min_length, double max_length, double k);
double _smin(double a, double b, double k);
LQuaternion quat_shortest_arc(const LVecBase3& arc_from, const LVecBase3& arc_to);


class EXPORT_CLASS WiggleBoneNode: public BoneNode {
PUBLISHED:
    explicit WiggleBoneNode(const char* name, unsigned int bone_id);
    void set_wigglebone_mode(int value);
    void set_stiffness(double value);
    void set_damping(double value);
    void set_gravity(const LVecBase3& value);
    void set_length(double value);
    void set_max_distance(double value);
    void set_max_degrees(double value);
    void reset();
    void update(NodePath root, LMatrix4 bone_pose, double delta);

private:
    int _wb_mode = WIGGLEBONE_MODE_ROTATION;
    double _stiffness = 0.1;
    double _damping = 0.1;
    LVecBase3 _gravity = VECTOR3_ZERO;
    double _length = 0.1;
    double _max_distance = 0.1;
    double _max_degrees = 60.0;

    LMatrix3 _point_mass;
    LMatrix4 _global_to_pose;
    bool _should_reset;
    LVecBase3 _acceleration;
    LVecBase3 _prev_mass_center;
    LVecBase3 _prev_velocity;

    void _process(NodePath root, LMatrix4 bone_pose, double delta);
    void _physics_process(double delta);
    LVecBase3 _update_acceleration(const LMatrix4& global_bone_pose, double delta);
    void _solve(
        const LMatrix4& global_to_local, const LVecBase3& acceleration,
        double delta);
    LMatrix4 _pose();
    void _point_mass_solve(double stiffness, double damping, double delta);
    void _point_mass_accelerate(const LVecBase3& acc, double delta);
    void _point_mass_apply_force(const LVecBase3& force);
    void _point_mass_reset();

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        BoneNode::init_type();
        register_type(_type_handle, "WiggleBoneNode", BoneNode::get_class_type());
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
