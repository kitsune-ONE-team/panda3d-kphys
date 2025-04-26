#ifndef PANDA_CHANNEL_H
#define PANDA_CHANNEL_H

#include "nodePath.h"
#include "typedReferenceCount.h"

#include "kphys/core/panda/animation.h"
#include "kphys/core/panda/frame.h"
#include "kphys/core/panda/types.h"

#ifndef NDEBUG
#define DEBUG_KPHYS
#endif

#define NUM_SLOTS 2


BEGIN_PUBLISH
enum SLOT {
    SLOT_A = 0,
    SLOT_B = 1,
};
enum BLENDING_FUNC {
    BF_LINEAR = 0,
    BF_EXPONENTIAL = 1,
};
END_PUBLISH

class EXPORT_CLASS Channel: public TypedReferenceCount, public Namable {
PUBLISHED:
    Channel(const std::string name);
    ~Channel();
    double get_factor();
    void set_blending_time(double t);
    void set_blending_func(unsigned int type);
    double get_frame_index(unsigned short slot);
    void set_frame_index(unsigned short slot, double frame);
    PointerTo<Frame> get_frame(unsigned short slot, bool interpolate=true);
    PointerTo<Animation> get_animation(unsigned short slot);
    void ls();
    void push_animation(PointerTo<Animation> animation);
    void switch_animation();
    void update(double dt);
    void include_bone(std::string name);
    void exclude_bone(std::string name);
    unsigned int get_num_included_bones();
    unsigned int get_num_excluded_bones();
    bool is_bone_included(std::string name);
    bool is_bone_excluded(std::string name);
    bool is_bone_enabled(std::string name);

private:
    PointerTo<Animation> _animations[NUM_SLOTS];
    double _frame_indices[NUM_SLOTS];
    double _factor;
    double _blending_time;
    unsigned int _blending_func;
    KDICT<std::string, bool> _include_bones;
    KDICT<std::string, bool> _exclude_bones;

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
