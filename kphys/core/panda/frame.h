#ifndef PANDA_FRAME_H
#define PANDA_FRAME_H

#include "nodePath.h"
#include "typedReferenceCount.h"
#include "pvector.h"
#include "transformState.h"

#include "kphys/core/panda/types.h"

#define FACTOR_AUTO -1.0

BEGIN_PUBLISH
enum TransformComponent {
    TRANSFORM_POS = 1 << 0,
    TRANSFORM_HPR = 1 << 1,
    TRANSFORM_QUAT = 1 << 2,
};
END_PUBLISH

LVecBase3 mix3(const LVecBase3& a, const LVecBase3& b, double factor);
LQuaternion quat_nlerp(const LQuaternion& a, const LQuaternion& b, double factor);
LQuaternion quat_slerp(const LQuaternion& a, const LQuaternion& b, double factor);


class EXPORT_CLASS Frame: public TypedReferenceCount {
PUBLISHED:
    Frame();
    ~Frame();
    void reset();
    unsigned int get_num_transforms();
    std::string get_bone_name(unsigned int i);
    ConstPointerTo<TransformState> get_transform(unsigned int i);
    ConstPointerTo<TransformState> get_transform(std::string name);
    unsigned short get_transform_flags(std::string name);
    double get_transform_factor(std::string name);
    void set_transform(
        std::string name, ConstPointerTo<TransformState> transform,
        unsigned short flags, double factor=1.0);
    void set_transform(
        std::string name, ConstPointerTo<TransformState> transform,
        bool has_pos, bool has_hpr, bool has_quat, double factor=1.0);
    void copy_transform_into(Frame& frame_dest, std::string name, double factor=FACTOR_AUTO);
    void copy_into(Frame& frame_dest);
    void mix_into(Frame& frame_dest, PointerTo<Frame> frame_b, double factor=FACTOR_AUTO);
    void ls();

private:
    unsigned long _iframe;
    pvector<std::string> _bone_names;
    KDICT<std::string, ConstPointerTo<TransformState>> _transforms;
    KDICT<std::string, unsigned short> _transform_flags;
    KDICT<std::string, double> _transform_factors;

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedReferenceCount::init_type();
        register_type(_type_handle, "Frame", TypedReferenceCount::get_class_type());
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
