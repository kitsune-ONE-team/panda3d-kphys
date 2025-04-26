#include "kphys/core/panda/multianimation.h"
#include "kphys/core/panda/multianimator.h"


TypeHandle MultiAnimatorNode::_type_handle;

MultiAnimatorNode::MultiAnimatorNode(const std::string name): AnimatorNode(name) {}

MultiAnimatorNode::~MultiAnimatorNode() {
    _multi_animations.clear();
}

/**
   Put a reusable animation in the storage.
*/
void MultiAnimatorNode::put_multi_animation(std::string name, PointerTo<MultiAnimation> multi_animation) {
    _multi_animations[name] = multi_animation;

    // put all the linked animations in the storage aswell
    unsigned int num_channels = get_num_channels();
    for (unsigned int i = 0; i < num_channels; i++) {
        PointerTo<Channel> channel = get_channel(i);
        PointerTo<Animation> animation = multi_animation->get_animation(channel->get_name());
        if (animation != nullptr) {
            put_animation(animation->get_name(), animation);
        }
    }
}

/**
   Get a reusable animation from the storage.
*/
PointerTo<MultiAnimation> MultiAnimatorNode::get_multi_animation(std::string name) {
    if (_multi_animations.find(name) == _multi_animations.end())
        return NULL;
    return _multi_animations[name];
}

void MultiAnimatorNode::switch_multi_animation(std::string name) {
    if (_multi_animation != nullptr &&
            strcmp(_multi_animation->get_name().c_str(), name.c_str()) == 0) {
        return;
    }

    PointerTo<MultiAnimation> multi_animation = get_multi_animation(name);
    if (multi_animation == nullptr) {
        return;
    }

    unsigned int num_channels = get_num_channels();
    for (unsigned int i = 0; i < num_channels; i++) {
        PointerTo<Channel> channel = get_channel(i);
        PointerTo<Animation> new_animation = multi_animation->get_animation(channel->get_name());
        PointerTo<Animation> cur_animation = channel->get_animation(SLOT_B);

        if (new_animation != nullptr && cur_animation != nullptr &&
                strcmp(new_animation->get_name().c_str(),
                       cur_animation->get_name().c_str()) == 0) {
            continue;
        }

        channel->switch_animation();
        if (new_animation != nullptr) {
            channel->push_animation(new_animation);
        }
    }

    _multi_animation = multi_animation;
}
