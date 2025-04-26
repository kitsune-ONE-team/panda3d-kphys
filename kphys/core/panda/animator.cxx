#include "kphys/core/panda/animator.h"
#include "kphys/core/panda/armature.h"

#include "nodePath.h"


TypeHandle AnimatorNode::_type_handle;

AnimatorNode::AnimatorNode(const std::string name): PandaNode(name) {
    _mframe = new Frame();
    for (unsigned int s = 0; s < NUM_SLOTS; s++) {
        _iframes[s] = new Frame();
        _fframes[s] = new Frame();
    }
}

AnimatorNode::~AnimatorNode() {
    _mframe->reset();
    for (unsigned int s = 0; s < NUM_SLOTS; s++) {
        _iframes[s]->reset();
        _fframes[s]->reset();
    }
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

    for (unsigned int s = 0; s < NUM_SLOTS; s++) {
        _fframes[s]->reset();

        if (s == SLOT_A && !blend)
            continue;

        // merge all channels into the singe frame
        unsigned int csize = get_num_channels();
        for (unsigned int c = 0; c < csize; c++) {
            PointerTo<Channel> channel = get_channel(c);

            _iframes[s]->reset();
            if (!channel->save_frame(*_iframes[s].p(), s, interpolate))
                continue;

            double cfactor = channel->get_factor();
            if (s == SLOT_A)
                cfactor = 1.0 - cfactor;

            if (!blend)
                cfactor = 1.0;

            // copy transforms
            unsigned int bsize = _iframes[s]->get_num_transforms();
            for (unsigned int b = 0; b < bsize; b++) {
                std::string bone_name = _iframes[s]->get_bone_name(b);
                if (!channel->is_bone_enabled(bone_name))
                    continue;

                _iframes[s]->copy_transform_into(*_fframes[s].p(), bone_name, cfactor);
            }
        }
    }

    if (blend) {
        _mframe->reset();
        _fframes[SLOT_A]->mix_into(*_mframe.p(), _fframes[SLOT_B]);
        ((ArmatureNode*) armature.node())->apply(_mframe, local_space);
    } else {
        ((ArmatureNode*) armature.node())->apply(_fframes[SLOT_B], local_space);
    }
}
