#ifndef PANDA_ANIMATION_H
#define PANDA_ANIMATION_H

#include "typedReferenceCount.h"
#include "namable.h"
#include "pvector.h"

#include "kphys/core/panda/frame.h"


class EXPORT_CLASS Animation: public TypedReferenceCount, public Namable {
PUBLISHED:
    Animation(const char* name);
    ~Animation();
    Frame* get_frame(unsigned long i);
    unsigned long get_num_frames();
    double get_frame_time();

private:
    unsigned long _get_frame_index(long frame);

    static TypeHandle _type_handle;

protected:
    pvector<Frame*> _motion;
    double _frame_time;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedReferenceCount::init_type();
        register_type(_type_handle, "Animation", TypedReferenceCount::get_class_type());
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
