#include "kphys/core/panda/frame.h"


TypeHandle Frame::_type_handle;

Frame::Frame(const char* name): PandaNode(name) {
    _iframe = 0;
}

Frame::Frame(const char* name, unsigned long iframe): PandaNode(name) {
    _iframe = iframe;
}

Frame::~Frame() {
    _bone_names.clear();
    _transforms.clear();
}

unsigned int Frame::get_num_transforms() {
    return _transforms.size();
}

char* Frame::get_bone_name(unsigned int i) {
    return _bone_names.at(i);
}

ConstPointerTo<TransformState> Frame::get_transform(unsigned int i) {
    return _transforms.at(_bone_names.at(i));
}

ConstPointerTo<TransformState> Frame::get_transform(char* name) {
    return _transforms.at(name);
}

void Frame::add_transform(char* name, ConstPointerTo<TransformState> transform) {
    _bone_names.push_back(name);
    _transforms[name] = transform;
}
