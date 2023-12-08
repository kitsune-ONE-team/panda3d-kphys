#include "kphys/core/panda/hitbox.h"
#include <stdio.h>

#include "bulletBoxShape.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"

TypeHandle HitboxNode::_type_handle;

HitboxNode::HitboxNode(const char *name, PT(BulletGhostNode) ghost):
        PandaNode(name) {
    BulletBoxShape* shape = (BulletBoxShape*) ghost->get_shape(0);
    LVecBase3 extents = shape->get_half_extents_without_margin();
    _size = extents * 2;
    _radius = extents.length();

    unsigned int i = 0;
    double z = -extents.get_z();
    for (unsigned int zi = 0; zi < 2; zi++) {
        double y = -extents.get_y();
        for (unsigned int yi = 0; yi < 2; yi++) {
            double x = -extents.get_x();
            for (unsigned int xi = 0; xi < 2; xi++) {
                _points[i] = LVecBase3(x, y, z);
                i++;
                x *= -1;
            }
            y *= -1;
        }
        z *= -1;
    }
}

PT(Hit) HitboxNode::ray_test(NodePath ray, const LVecBase3& ray_size) {
    // if (!hitbox)
    //     hitbox = NodePath::any_path(this);
    NodePath hitbox = NodePath::any_path(this);

    // check if ray does intersects sphere
    LVecBase3 center = ray.get_relative_point(hitbox, LVecBase3(0, 0, 0));
    LVecBase2 center2d(center.get_x(), center.get_z());
    if (center2d.length() > _radius)
        return nullptr;

    // calculate AABB (Axis Aligned Bounding Box)
    // and store it in matrix as:
    // (X_min, 0, X_max)
    // (Y_min, 0, Y_max)
    // (Z_min, 0, Z_max)
    LMatrix3 aabb(
        LVecBase3(1000, 0, -1000),
        LVecBase3(1000, 0, -1000),
        LVecBase3(1000, 0, -1000));
    double value;
    for (unsigned int i = 0; i < 8; i++) {
        // get relative to nodepath point in ray's local space
        LVecBase3 point = ray.get_relative_point(hitbox, _points[i]);

        value = aabb.get_cell(ROW_X, COL_MIN);
        aabb.set_cell(ROW_X, COL_MIN, fmin(value, point.get_x()));
        value = aabb.get_cell(ROW_X, COL_MAX);
        aabb.set_cell(ROW_X, COL_MAX, fmax(value, point.get_x()));

        value = aabb.get_cell(ROW_Y, COL_MIN);
        aabb.set_cell(ROW_Y, COL_MIN, fmin(value, point.get_y()));
        value = aabb.get_cell(ROW_Y, COL_MAX);
        aabb.set_cell(ROW_Y, COL_MAX, fmax(value, point.get_y()));

        value = aabb.get_cell(ROW_Z, COL_MIN);
        aabb.set_cell(ROW_Z, COL_MIN, fmin(value, point.get_z()));
        value = aabb.get_cell(ROW_Z, COL_MAX);
        aabb.set_cell(ROW_Z, COL_MAX, fmax(value, point.get_z()));
    }

    // ray width
    if (aabb.get_cell(ROW_X, COL_MIN) > ray_size.get_x() / 2.0 ||
        aabb.get_cell(ROW_X, COL_MAX) < -ray_size.get_x() / 2.0)
        return nullptr;

    // ray height
    if (aabb.get_cell(ROW_Z, COL_MIN) > ray_size.get_z() / 2.0 ||
        aabb.get_cell(ROW_Z, COL_MAX) < -ray_size.get_z() / 2.0)
        return nullptr;

    // ray length/depth
    if (aabb.get_cell(ROW_Y, COL_MIN) < 0 ||
             aabb.get_cell(ROW_Y, COL_MAX) > ray_size.get_y())
        return nullptr;

    LPoint3 hit_pos(0, 0, 0);
    LVector3 hit_normal(0, 0, 0);
    return new Hit(hitbox, aabb, hit_pos, hit_normal);
}

void HitboxNode::show_debug_node() {
    if (_debug_node != nullptr)
        return;
    PT(GeomNode) geom_node = new GeomNode("debug");
    geom_node->add_geom(make_geometry());
    _debug_node = geom_node;
    add_child(_debug_node);
}

void HitboxNode::hide_debug_node() {
    if (_debug_node == nullptr)
        return;
    remove_child(_debug_node);
    _debug_node = nullptr;
}

PT(Geom) HitboxNode::make_geometry() {
    PT(Geom) geom;
    PT(GeomPrimitive) prim;
    PT(GeomVertexData) vdata;

    vdata = new GeomVertexData("cube", GeomVertexFormat::get_v3n3c4(), Geom::UH_static);
    vdata->set_num_rows(24);

    GeomVertexWriter vertices(vdata, InternalName::get_vertex());
    GeomVertexWriter normals(vdata, InternalName::get_normal());
    GeomVertexWriter colors(vdata, InternalName::get_color());

    double z = -_size.get_z() / 2.0;
    int zc = -1;
    for (unsigned int zi = 0; zi < 2; zi++) {
        double y = -_size.get_y() / 2.0;
        int yc = -1;
        for (unsigned int yi = 0; yi < 2; yi++) {
            double x = -_size.get_x() / 2.0;
            int xc = -1;
            if (y > 0) {
                x *= -1;
                xc *= -1;
            }
            for (unsigned int xi = 0; xi < 2; xi++) {
                for (unsigned int v = 0; v < 3; v++) {
                    vertices.add_data3(x, y, z);
                    // -1...1 -> 0...1
                    colors.add_data4(
                        (xc + 1.0) / 2.0,
                        (yc + 1.0) / 2.0,
                        (zc + 1.0) / 2.0, 1.0);
                }
                normals.add_data3(0.0, y, 0.0);
                normals.add_data3(x, 0.0, 0.0);
                normals.add_data3(0.0, 0.0, z);
                x *= -1;
                xc *= -1;
            }
            y *= -1;
            yc *= -1;
        }
        z *= -1;
        zc *= -1;
    }

    // store the triangles, counter clockwise from front
    prim = new GeomTriangles(Geom::UH_static);
    prim->add_vertices(0, 3, 15);
    prim->add_vertices(0, 15, 12);
    prim->add_vertices(4, 6, 18);
    prim->add_vertices(4, 18, 16);
    prim->add_vertices(7, 9, 21);
    prim->add_vertices(7, 21, 19);
    prim->add_vertices(10, 1, 13);
    prim->add_vertices(10, 13, 22);
    prim->add_vertices(2, 11, 8);
    prim->add_vertices(2, 8, 5);
    prim->add_vertices(14, 17, 20);
    prim->add_vertices(14, 20, 23);
    prim->close_primitive();

    geom = new Geom(vdata);
    geom->add_primitive(prim);
    return geom;
}
