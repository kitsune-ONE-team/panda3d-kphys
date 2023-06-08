#include "kphys/core/panda/frame.h"


TypeHandle Frame::_type_handle;


LVecBase3 mix3(const LVecBase3& a, const LVecBase3& b, double factor) {
    return LVecBase3(
        a.get_x() - factor * (a.get_x() - b.get_x()),
        a.get_y() - factor * (a.get_y() - b.get_y()),
        a.get_z() - factor * (a.get_z() - b.get_z()));
}

LQuaternion mixq(const LQuaternion& a, const LQuaternion& b, double factor) {
    LQuaternion b2;
    if (a.dot(b) < 0)  // negate quat if dot product is negative
        b2 = LQuaternion(-b.get_r(), -b.get_i(), -b.get_j(), -b.get_k());
    else
        b2 = b;

    return LQuaternion(
        a.get_r() - factor * (a.get_r() - b2.get_r()),
        a.get_i() - factor * (a.get_i() - b2.get_i()),
        a.get_j() - factor * (a.get_j() - b2.get_j()),
        a.get_k() - factor * (a.get_k() - b2.get_k()));
}


Frame::Frame() {}

Frame::~Frame() {
    _bone_names.clear();
    _transforms.clear();
    _transform_flags.clear();
}

void Frame::add_transform(
        const char* name, ConstPointerTo<TransformState> transform,
        unsigned short flags) {
    std::string s = std::string(name);
    _bone_names.push_back(s);
    _transforms[s] = transform;
    _transform_flags[s] = flags;
}

void Frame::add_transform(
        const char* name, ConstPointerTo<TransformState> transform,
        bool has_pos, bool has_hpr, bool has_quat) {
    unsigned short flags = 0;
    if (has_pos)
        flags |= TRANSFORM_POS;
    if (has_hpr)
        flags |= TRANSFORM_HPR;
    if (has_quat)
        flags |= TRANSFORM_QUAT;
    add_transform(name, transform, flags);
}

unsigned int Frame::get_num_transforms() {
    return _transforms.size();
}

const char* Frame::get_bone_name(unsigned int i) {
    return _bone_names[i].c_str();
}

ConstPointerTo<TransformState> Frame::get_transform(unsigned int i) {
    return _transforms[_bone_names[i]];
}

ConstPointerTo<TransformState> Frame::get_transform(const char* name) {
    std::string s = std::string(name);
    if (_transforms.find(s) == _transforms.end())
        return NULL;
    return _transforms[s];
}

unsigned short Frame::get_transform_flags(const char* name) {
    std::string s = std::string(name);
    if (_transform_flags.find(s) == _transform_flags.end())
        return 0;
    return _transform_flags[s];
}

PointerTo<Frame> Frame::mix(PointerTo<Frame> frame_b, double factor) {
    if (factor == 0)
        return this;
    else if (factor == 1)
        return frame_b;

    PointerTo<Frame> frame = new Frame();

    unsigned int size = get_num_transforms();
    for (unsigned int i = 0; i < size; i++) {
        const char* bone_name = get_bone_name(i);

        ConstPointerTo<TransformState> transform_a = get_transform(bone_name);
        ConstPointerTo<TransformState> transform_b = frame_b->get_transform(bone_name);
        unsigned short flags_a = get_transform_flags(bone_name);
        unsigned short flags_b = frame_b->get_transform_flags(bone_name);
        unsigned short flags = flags_a & flags_b;

        if (transform_a != NULL, transform_b == NULL) {
            frame->add_transform(bone_name, transform_a, flags_a);

        } else if (transform_a == NULL, transform_b != NULL) {
            frame->add_transform(bone_name, transform_b, flags_b);

        } else if (transform_a != NULL, transform_b != NULL) {
            LVecBase3 pos, hpr;
            LQuaternion quat;
            bool has_pos = false, has_hpr = false, has_quat = false;

            if (flags & TRANSFORM_POS) {
                pos = mix3(transform_a->get_pos(), transform_b->get_pos(), factor);
                has_pos = true;
            }
            if (flags & TRANSFORM_HPR) {
                hpr = mix3(transform_a->get_hpr(), transform_b->get_hpr(), factor);
                has_hpr = true;
            }
            if (flags & TRANSFORM_QUAT) {
                quat = mixq(transform_a->get_quat(), transform_b->get_quat(), factor);
                has_quat = true;
            }

            ConstPointerTo<TransformState> transform = NULL;

            if (has_pos && has_hpr && !has_quat)
                transform = TransformState::make_pos_hpr(pos, hpr);
            else if (has_pos && !has_hpr && has_quat)
                // there is no make_pos_quat() method
                transform = TransformState::make_pos_quat_scale(
                    pos, quat, LVecBase3(1, 1, 1));
            else if (has_pos && !has_hpr && !has_quat)
                transform = TransformState::make_pos(pos);
            else if (!has_pos && has_hpr && !has_quat)
                transform = TransformState::make_hpr(hpr);
            else if (!has_pos && !has_hpr && has_quat)
                transform = TransformState::make_quat(quat);

            if (transform != NULL) {
                  frame->add_transform(
                      bone_name, transform, has_pos, has_hpr, has_quat);
            }
        }
    }

    return frame;
}
