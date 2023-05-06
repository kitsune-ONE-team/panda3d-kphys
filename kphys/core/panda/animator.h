#ifndef PANDA_ANIMATOR_H
#define PANDA_ANIMATOR_H

#include "pandaNode.h"
#include "pmap.h"
#include "nodePath.h"

#include "kphys/core/panda/animation.h"


class EXPORT_CLASS AnimatorNode: public PandaNode {
PUBLISHED:
    AnimatorNode(const char* name);
    ~AnimatorNode();
    void put_animation(char* name, Animation* animation);
    Animation* get_animation(char* name);

private:
    NodePath _armature;
    pmap<char*, Animation*> _animations;

    static TypeHandle _type_handle;

protected:
    pvector<Frame*> _motion;
    double _frame_time;

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