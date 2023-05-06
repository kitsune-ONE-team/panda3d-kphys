#include "kphys/core/panda/animator.h"


TypeHandle AnimatorNode::_type_handle;

AnimatorNode::AnimatorNode(const char* name): PandaNode(name) {}

AnimatorNode::~AnimatorNode() {
    _animations.clear();
}

void AnimatorNode::put_animation(char* name, Animation* animation) {
    _animations[name] = animation;
}

Animation* AnimatorNode::get_animation(char* name) {
    return _animations[name];
}
