#include "kphys/core/panda/multianimation.h"
#include "kphys/core/panda/multianimator.h"


TypeHandle MultiAnimatorNode::_type_handle;

MultiAnimatorNode::MultiAnimatorNode(const char* name): AnimatorNode(name) {}

MultiAnimatorNode::~MultiAnimatorNode() {
    _multi_animations.clear();
}

/**
   Put a reusable animation in the storage.
*/
void MultiAnimatorNode::put_multi_animation(const char* name, PointerTo<MultiAnimation> multi_animation) {
    std::string s = std::string(name);
    _multi_animations[s] = multi_animation;

    // put all the linked animations in the storage aswell
    for (unsigned int i = 0; i < get_num_channels(); i++) {
        PointerTo<Channel> channel = get_channel(i);
        PointerTo<Animation> animation = multi_animation->get_animation(
            channel->get_name().c_str());
        if (animation != nullptr) {
            put_animation(animation->get_name().c_str(), animation);
        }
    }
}

/**
   Get a reusable animation from the storage.
*/
PointerTo<MultiAnimation> MultiAnimatorNode::get_multi_animation(const char* name) {
    std::string s = std::string(name);
    if (_multi_animations.find(s) == _multi_animations.end())
        return NULL;
    return _multi_animations[s];
}

void MultiAnimatorNode::switch_multi_animation(const char* name) {
    if (_multi_animation != nullptr && name != NULL &&
            strcmp(_multi_animation->get_name().c_str(), name) == 0) {
        return;
    }

    PointerTo<MultiAnimation> multi_animation = get_multi_animation(name);
    if (multi_animation == nullptr) {
        return;
    }

    for (unsigned int i = 0; i < get_num_channels(); i++) {
        PointerTo<Channel> channel = get_channel(i);
        PointerTo<Animation> new_animation = multi_animation->get_animation(
            channel->get_name().c_str());
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
