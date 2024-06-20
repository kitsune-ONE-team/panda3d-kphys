#ifndef PANDA_MULTI_ANIMATOR_H
#define PANDA_MULTI_ANIMATOR_H

#include "kphys/core/panda/animator.h"


class EXPORT_CLASS MultiAnimatorNode: public AnimatorNode {
PUBLISHED:
    MultiAnimatorNode(const char* name);
    ~MultiAnimatorNode();
    void put_multi_animation(const char* name, PointerTo<MultiAnimation> multi_animation);
    PointerTo<MultiAnimation> get_multi_animation(const char* name);
    void switch_multi_animation(const char* name);

private:
    PointerTo<MultiAnimation> _multi_animation;
    pmap<std::string, PointerTo<MultiAnimation>> _multi_animations;

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        AnimatorNode::init_type();
        register_type(_type_handle, "MultiAnimatorNode", PandaNode::get_class_type());
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
