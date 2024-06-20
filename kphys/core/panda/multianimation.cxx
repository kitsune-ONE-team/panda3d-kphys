#include "kphys/core/panda/multianimation.h"


TypeHandle MultiAnimation::_type_handle;

MultiAnimation::MultiAnimation(const char* name): Namable(name) {}

MultiAnimation::~MultiAnimation() {
    _animations.clear();
}

/**
   Put a reusable animation in the storage.
*/
void MultiAnimation::put_animation(const char* channel_name, PointerTo<Animation> animation) {
    std::string s = std::string(channel_name);
    _animations[s] = animation;
}

/**
   Get a reusable animation from the storage.
*/
PointerTo<Animation> MultiAnimation::get_animation(const char* channel_name) {
    std::string s = std::string(channel_name);
    if (_animations.find(s) == _animations.end())
        return nullptr;
    return _animations[s];
}
