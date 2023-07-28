#include "kphys/core/panda/animator.h"
#include "kphys/core/panda/armature.h"

#include "nodePath.h"


TypeHandle AnimatorNode::_type_handle;

AnimatorNode::AnimatorNode(const char* name): PandaNode(name) {}

AnimatorNode::~AnimatorNode() {
    _animations.clear();
    _channel_names.clear();
    _channels.clear();
}

unsigned int AnimatorNode::get_num_channels() {
    return _channels.size();
}

/**
   Create a new animation channel.
*/
void AnimatorNode::add_channel(const char* name) {
    std::string s = std::string(name);
    _channel_names.push_back(s);
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
    PointerTo<Frame> frame = new Frame();

    unsigned int csize = get_num_channels();
    for (unsigned int c = 0; c < csize; c++) {
        PointerTo<Channel> channel = get_channel(c);

        PointerTo<Frame> frame_a = channel->get_frame(SLOT_A);
        PointerTo<Frame> frame_b = channel->get_frame(SLOT_B);
        PointerTo<Frame> frame_c;
        if (frame_a != NULL && frame_b != NULL)
            frame_c = frame_a->mix(frame_b, channel->get_factor());
        else if (frame_a == NULL && frame_b != NULL)
            frame_c = frame_b;
        else if (frame_a != NULL && frame_b == NULL)
            frame_c = frame_a;
        else
            continue;

        unsigned int bsize = frame_c->get_num_transforms();
        for (unsigned int b = 0; b < bsize; b++) {
            const char* bone_name = frame_c->get_bone_name(b);
            if (!channel->is_bone_enabled(bone_name))
                continue;

            if (frame->get_transform(bone_name) != NULL)
                continue;

            ConstPointerTo<TransformState> transform = frame_c->get_transform(bone_name);
            unsigned short flags = frame_c->get_transform_flags(bone_name);
            frame->add_transform(bone_name, transform, flags);
        }
    }

    NodePath animator = NodePath::any_path(this);
    NodePathCollection armatures = animator.find_all_matches("**/+ArmatureNode");
    for (int i = 0; i < armatures.get_num_paths(); i++) {
        NodePath np = armatures.get_path(i);
        ((ArmatureNode*) np.node())->apply(frame);
        break;
    }
}
