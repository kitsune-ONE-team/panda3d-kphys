#include "kphys/core/panda/hit.h"


TypeHandle Hit::_type_handle;

Hit::Hit(NodePath hitbox, const LMatrix3& aabb,
         const LPoint3& hit_pos, const LVector3& hit_normal) {
    _hitbox = hitbox;
    _aabb = aabb;
    _hit_pos = hit_pos;
    _hit_normal = hit_normal;
}

/*
  Get AABB (Axis Aligned Bounding Box).
*/
LMatrix3 Hit::get_aabb() const {
    return _aabb;
}

NodePath Hit::get_node() const {
    return _hitbox;
}

LPoint3 Hit::get_hit_pos() const {
    return _hit_pos;
}

LVector3 Hit::get_hit_normal() const {
    return _hit_normal;
}

bool Hit::is_closer(PT(Hit) other) {
    // target Y min < other Y min
    return (
        get_aabb().get_cell(ROW_Y, COL_MIN) <
        other->get_aabb().get_cell(ROW_Y, COL_MIN));
}
