#include "kphys/core/panda/frame.h"


TypeHandle Frame::_type_handle;

Frame::Frame() {}

Frame::~Frame() {
    _bone_names.clear();
    _transforms.clear();
    _transform_flags.clear();
}

void Frame::add_transform(
        const char* name, ConstPointerTo<TransformState> transform,
        bool has_pos, bool has_hpr, bool has_quat) {
    std::string s = std::string(name);
    _bone_names.push_back(s);
    _transforms[s] = transform;
    unsigned short flags = 0;
    if (has_pos)
        flags |= TRANSFORM_POS;
    if (has_hpr)
        flags |= TRANSFORM_HPR;
    if (has_quat)
        flags |= TRANSFORM_QUAT;
    _transform_flags[s] = flags;
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
