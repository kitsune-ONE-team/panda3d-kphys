#ifndef PANDA_HITBOX_H
#define PANDA_HITBOX_H

#include "kphys/core/panda/hit.h"

#include "bulletGhostNode.h"
#include "geomNode.h"
#include "pandaNode.h"
#include "transformState.h"


class EXPORT_CLASS HitboxNode: public PandaNode {
PUBLISHED:
    explicit HitboxNode(const char *name, PT(BulletGhostNode) ghost);
    PT(Hit) ray_test(NodePath ray, const LVecBase3& ray_size);
    void show_debug_node();
    void hide_debug_node();
    PT(Geom) make_geometry();

private:
    /* NodePath hitbox; */
    LVecBase3 _size;
    double _radius;
    LVecBase3 _points[8];
    PT(PandaNode) _debug_node;
    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        PandaNode::init_type();
        register_type(
            _type_handle, "HitboxNode",
            PandaNode::get_class_type());
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
