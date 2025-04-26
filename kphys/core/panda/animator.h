#ifndef PANDA_ANIMATOR_H
#define PANDA_ANIMATOR_H

#include "pandaNode.h"
#include "pvector.h"
#include "nodePath.h"

#include "kphys/core/panda/animation.h"
#include "kphys/core/panda/channel.h"
#include "kphys/core/panda/frame.h"
#include "kphys/core/panda/types.h"


class EXPORT_CLASS AnimatorNode: public PandaNode {
PUBLISHED:
    AnimatorNode(const std::string name);
    ~AnimatorNode();
    unsigned int get_num_channels();
    void add_channel(std::string name);
    PointerTo<Channel> get_channel(unsigned int i);
    PointerTo<Channel> get_channel(std::string name);
    void put_animation(std::string name, PointerTo<Animation> animation);
    PointerTo<Animation> get_animation(std::string name);
    NodePath find_armature();
    void update(double dt);
    void apply(bool blend=true, bool interpolate=true, bool local_space=true);

private:
    PointerTo<Frame> _mframe;  // final mixed frame
    PointerTo<Frame> _iframes[NUM_SLOTS];  // interpolated frames
    PointerTo<Frame> _fframes[NUM_SLOTS];  // filtered frames
    KDICT<std::string, PointerTo<Animation>> _animations;
    KDICT<std::string, NodePath> _armatures;
    pvector<std::string> _channel_names;
    KDICT<std::string, PointerTo<Channel>> _channels;

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        PandaNode::init_type();
        register_type(_type_handle, "AnimatorNode", PandaNode::get_class_type());
    }
    virtual TypeHandle get_type() const {
        return get_class_type();
    }
    virtual TypeHandle force_init_type() {
        init_type();
        return get_class_type();
    }
};

#endif
