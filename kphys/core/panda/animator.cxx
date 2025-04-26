#include "kphys/core/panda/animator.h"
#include "kphys/core/panda/armature.h"

#include "nodePath.h"


TypeHandle AnimatorNode::_type_handle;

AnimatorNode::AnimatorNode(const std::string name): PandaNode(name) {}

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
void AnimatorNode::add_channel(std::string name) {
    _channel_names.push_back(name);
    _channels[name] = new Channel(name);
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
PointerTo<Channel> AnimatorNode::get_channel(std::string name) {
    if (_channels.find(name) == _channels.end())
        return NULL;
    return _channels[name];
}

/**
   Put a reusable animation in the storage.
*/
void AnimatorNode::put_animation(std::string name, PointerTo<Animation> animation) {
    _animations[name] = animation;
}

/**
   Get a reusable animation from the storage.
*/
PointerTo<Animation> AnimatorNode::get_animation(std::string name) {
    if (_animations.find(name) == _animations.end())
        return NULL;
    return _animations[name];
}

NodePath AnimatorNode::find_armature() {
    if (_armatures.find("armature") != _armatures.end())
        return _armatures["armature"];

    NodePath animator = NodePath::any_path(this);
    NodePathCollection armatures = animator.find_all_matches("**/+ArmatureNode");
    for (int i = 0; i < armatures.get_num_paths(); i++) {
        NodePath np = armatures.get_path(i);
        _armatures["armature"] = np;
        return np;
    }

    _armatures["armature"] = NodePath();
    return _armatures["armature"];
}

void AnimatorNode::update(double dt) {
    unsigned int csize = get_num_channels();
    for (unsigned int c = 0; c < csize; c++) {
        PointerTo<Channel> channel = get_channel(c);
        channel->update(dt);
    }
}

void AnimatorNode::apply(bool blend, bool interpolate, bool local_space) {
    NodePath armature = find_armature();
    if (armature.is_empty())
        return;

    PointerTo<Frame> frames[NUM_SLOTS];
    for (unsigned int s = 0; s < NUM_SLOTS; s++) {
        // if (!blend && s == SLOT_A)
        //     continue;

        frames[s] = new Frame();

        // merge all channels into the singe frame
        unsigned int csize = get_num_channels();
        for (unsigned int c = 0; c < csize; c++) {
            PointerTo<Channel> channel = get_channel(c);

            PointerTo<Frame> frame = channel->get_frame(s, interpolate);
            // frame->ls();
            if (frame == NULL)
                continue;

            double cfactor = channel->get_factor();
            if (!blend)
                cfactor = 1.0;

            if (s == SLOT_A)
                cfactor = 1.0 - cfactor;

            // copy transforms
            unsigned int bsize = frame->get_num_transforms();
            for (unsigned int b = 0; b < bsize; b++) {
                std::string bone_name = frame->get_bone_name(b);
                if (!channel->is_bone_enabled(bone_name))
                    continue;

                if (frames[s]->get_transform(bone_name) != NULL)
                    continue;

                ConstPointerTo<TransformState> transform = frame->get_transform(bone_name);
                if (transform == NULL)
                    continue;

                unsigned short flags = frame->get_transform_flags(bone_name);
                frames[s]->set_transform(bone_name, transform, flags, cfactor);
            }
        }
    }

    // frames[SLOT_A]->ls();
    // frames[SLOT_B]->ls();
    PointerTo<Frame> frame = frames[SLOT_A]->mix(frames[SLOT_B]);
    // frame->ls();
    ((ArmatureNode*) armature.node())->apply(frame, local_space);
}
