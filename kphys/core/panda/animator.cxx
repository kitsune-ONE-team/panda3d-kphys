#include "kphys/core/panda/animator.h"
#include "kphys/core/panda/armature.h"

#include "nodePath.h"


TypeHandle AnimatorNode::_type_handle;

AnimatorNode::AnimatorNode(const char* name): PandaNode(name) {}

AnimatorNode::~AnimatorNode() {
    _animations.clear();
}

/**
   Create a new animation channel.
*/
void AnimatorNode::add_channel(char* name) {
    _channels[name] = new Channel(name);
}

/**
   Get an animation channel.
*/
PointerTo<Channel> AnimatorNode::get_channel(char* name) {
    if (_channels.find(name) == _channels.end())
        return NULL;
    return _channels[name];
}

/**
   Put a reusable animation in the storage.
*/
void AnimatorNode::put_animation(char* name, Animation* animation) {
    _animations[name] = animation;
}

/**
   Get a reusable animation from the storage.
*/
PointerTo<Animation> AnimatorNode::get_animation(char* name) {
    if (_animations.find(name) == _animations.end())
        return NULL;
    return _animations[name];
}

void AnimatorNode::apply() {
    PointerTo<Frame> frame = NULL;

    pmap<char*, PointerTo<Channel>>::iterator it = _channels.begin();
    while (it != _channels.end()) {
        char* name = it->first;
        if (strcmp(name, "root") == 0) {
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
