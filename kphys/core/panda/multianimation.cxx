#include "kphys/core/panda/multianimation.h"


TypeHandle MultiAnimation::_type_handle;

MultiAnimation::MultiAnimation(const std::string name): Namable(name) {}

MultiAnimation::~MultiAnimation() {
    _animations.clear();
}

/**
   Put a reusable animation in the storage.
*/
void MultiAnimation::put_animation(std::string channel_name, PointerTo<Animation> animation) {
    _animations[channel_name] = animation;
}

/**
   Get a reusable animation from the storage.
*/
PointerTo<Animation> MultiAnimation::get_animation(std::string channel_name) {
    if (_animations.find(channel_name) == _animations.end())
        return nullptr;
    return _animations[channel_name];
}
