#ifndef PANDA_HIT_H
#define PANDA_HIT_H

#define ROW_X 0
#define ROW_Y 1
#define ROW_Z 2
#define COL_MIN 0
#define COL_MAX 2

#include "nodePath.h"
#include "typedReferenceCount.h"
#include "transformState.h"


class EXPORT_CLASS Hit: public TypedReferenceCount {
PUBLISHED:
    Hit(NodePath hitbox, const LMatrix3& aabb,
        const LPoint3& hit_pos, const LVector3& hit_normal);

    LMatrix3 get_aabb() const;
    NodePath get_node() const;
    LPoint3 get_hit_pos() const;
    LVector3 get_hit_normal() const;
    bool is_closer(PT(Hit) other);

private:
    NodePath _hitbox;
    LMatrix3 _aabb;
    LPoint3 _hit_pos;
    LVector3 _hit_normal;

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedReferenceCount::init_type();
        register_type(_type_handle, "Hit", TypedReferenceCount::get_class_type());
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
