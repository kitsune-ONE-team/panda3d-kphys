#include "kphys/core/panda/animator.h"
#include "kphys/core/panda/armature.h"

#include "nodePath.h"


TypeHandle AnimatorNode::_type_handle;

AnimatorNode::AnimatorNode(const char* name): PandaNode(name) {}

AnimatorNode::~AnimatorNode() {
    _animations.clear();
}

unsigned int AnimatorNode::get_num_channels() {
    return _channels.size();
}

/**
   Create a new animation channel.
*/
void AnimatorNode::add_channel(const char* name) {
    std::string s = std::string(name);
    _channels[s] = new Channel(name);
}

/**
   Get an animation channel.
*/
PointerTo<Channel> AnimatorNode::get_channel(unsigned int i) {
    return _channels[_channel_names[i]];
}

/**
   Get an animation channel.
*/
PointerTo<Channel> AnimatorNode::get_channel(const char* name) {
    std::string s = std::string(name);
    if (_channels.find(s) == _channels.end())
        return NULL;
    return _channels[s];
}

/**
   Put a reusable animation in the storage.
*/
void AnimatorNode::put_animation(const char* name, Animation* animation) {
    std::string s = std::string(name);
    _animations[s] = animation;
}

/**
   Get a reusable animation from the storage.
*/
PointerTo<Animation> AnimatorNode::get_animation(const char* name) {
    std::string s = std::string(name);
    if (_animations.find(s) == _animations.end())
        return NULL;
    return _animations[s];
}

void AnimatorNode::apply() {
    PointerTo<Frame> frame = NULL;

    pmap<std::string, PointerTo<Channel>>::iterator it = _channels.begin();
    while (it != _channels.end()) {
        std::string name = it->first;
        if (strcmp(name.c_str(), "root") == 0) {
            PointerTo<Channel> channel = it->second;
            frame = channel->get_frame(SLOT_B);
            break;
        }
        it++;
    }

    if (!frame)
        return;

    NodePath animator = NodePath::any_path(this);
    NodePathCollection armatures = animator.find_all_matches("**/+ArmatureNode");
    for (int i = 0; i < armatures.get_num_paths(); i++) {
        NodePath np = armatures.get_path(i);
        ((ArmatureNode*) np.node())->apply(frame);
        break;
    }
}
