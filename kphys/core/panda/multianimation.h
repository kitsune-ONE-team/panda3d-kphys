#ifndef PANDA_MULTI_ANIMATION_H
#define PANDA_MULTI_ANIMATION_H

#include "namable.h"
#include "pvector.h"
#include "typedReferenceCount.h"

#include "kphys/core/panda/animation.h"
#include "kphys/core/panda/types.h"


class EXPORT_CLASS MultiAnimation: public TypedReferenceCount, public Namable {
PUBLISHED:
    MultiAnimation(const std::string name);
    ~MultiAnimation();
    void put_animation(std::string channel_name, PointerTo<Animation> animation);
    PointerTo<Animation> get_animation(std::string channel_name);

private:
    KDICT<std::string, PointerTo<Animation>> _animations;

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedReferenceCount::init_type();
        register_type(_type_handle, "MultiAnimation", TypedReferenceCount::get_class_type());
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
