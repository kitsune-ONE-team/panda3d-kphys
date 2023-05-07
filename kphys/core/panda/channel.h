#ifndef PANDA_CHANNEL_H
#define PANDA_CHANNEL_H

#include "nodePath.h"
#include "pmap.h"
#include "typedReferenceCount.h"

#include "kphys/core/panda/animation.h"


class EXPORT_CLASS Channel: public TypedReferenceCount, public Namable {
PUBLISHED:
    Channel(const char* name);
    ~Channel();
    double get_factor();
    void set_blending_time(unsigned long t);
    double get_frame(unsigned short i);
    void set_frame(unsigned short i, double frame);
    Animation* get_animation(unsigned short i);
    void push_animation(Animation* animation);
    void switch_animation();
    void update(unsigned long dt);
    void include_bone(char* name);
    void exclude_bone(char* name);

private:
    Animation* _animations[2];
    unsigned long _frames[2];
    unsigned long _factor;
    unsigned long _blending_time;
    pmap<char*, bool> _include_bones;
    pmap<char*, bool> _exclude_bones;

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedReferenceCount::init_type();
        register_type(_type_handle, "Channel", TypedReferenceCount::get_class_type());
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