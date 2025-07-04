#include <math.h>

#include "kphys/core/panda/channel.h"


TypeHandle Channel::_type_handle;

Channel::Channel(const std::string name): Namable(name) {
    for (unsigned short i = 0; i < NUM_SLOTS; i++) {
        _animations[i] = NULL;
        _frame_indices[i] = 0.0;
    }
    _factor = 0.0;
    _blending_func = BF_LINEAR;
}

Channel::~Channel() {
    for (unsigned short i = 0; i < NUM_SLOTS; i++) {
        _animations[i] = NULL;
        _frame_indices[i] = 0.0;
    }
    _include_bones.clear();
    _exclude_bones.clear();
}

void Channel::include_bone(std::string name) {
    if (_exclude_bones.find(name) == _exclude_bones.end()) {
        _include_bones[name] = true;
    } else {
        _exclude_bones.erase(name);
    }
}

void Channel::exclude_bone(std::string name) {
    if (_include_bones.find(name) == _include_bones.end()) {
        _exclude_bones[name] = true;
    } else {
        _include_bones.erase(name);
    }
}

unsigned int Channel::get_num_included_bones() {
    return _include_bones.size();
}

unsigned int Channel::get_num_excluded_bones() {
    return _exclude_bones.size();
}

bool Channel::is_bone_included(std::string name) {
    if (_include_bones.find(name) == _include_bones.end())
        return false;
    return true;
}

bool Channel::is_bone_excluded(std::string name) {
    if (_exclude_bones.find(name) == _exclude_bones.end())
        return false;
    return true;
}

bool Channel::is_bone_enabled(std::string name) {
    if (get_num_included_bones())  // whitelist
        return is_bone_included(name);
    else  // blacklist
        return !is_bone_excluded(name);
}

/**
   Returns factor of blending.
   0:   A = 100%, B =   0%
   100: A =   0%, B = 100%
*/
double Channel::get_factor() {
    double factor = _factor / _blending_time;
    switch (_blending_func) {
    case BF_EXPONENTIAL:
        return pow(factor, 2);
    default:  // BF_LINEAR
        return factor;
    }
}

void Channel::set_blending_time(double t) {
    _blending_time = t;
}

void Channel::set_blending_func(unsigned int type) {
    _blending_func = type;
}

double Channel::get_frame_index(unsigned short slot) {
    return _frame_indices[slot];
}

void Channel::set_frame_index(unsigned short slot, double index) {
    _frame_indices[slot] = index;
}

/**
   Returns animation frame from the specified slot.
*/
bool Channel::save_frame(Frame& frame, unsigned short slot, bool interpolate) {
    PointerTo<Animation> animation = get_animation(slot);
    if (animation == NULL)
        return false;

    double index = get_frame_index(slot);
    if (interpolate) {
        unsigned long i = (unsigned long) floor(index);
        unsigned long j = (unsigned long) ceil(index);
        PointerTo<Frame> frame_i = animation->get_frame(i);
        PointerTo<Frame> frame_j = animation->get_frame(j);
        double factor = fmod(index, 1);  // index % 1
        frame_i->mix_into(frame, frame_j, factor);
    } else {
        unsigned long i = (unsigned long) round(index);
        PointerTo<Frame> frame_i = animation->get_frame(i);
        frame_i->copy_into(frame);
    }
    return true;
}

/**
   Returns the animation in the specified slot.
*/
PointerTo<Animation> Channel::get_animation(unsigned short slot) {
    return _animations[slot];
}

void Channel::ls() {
    printf(
        "Channel (%s) { (%f):\n    A: %s\n    B: %s }\n",
        get_name().c_str(),
        get_factor(),
        (_animations[SLOT_A] != NULL) ? _animations[SLOT_A]->get_name().c_str() : "-",
        (_animations[SLOT_B] != NULL) ? _animations[SLOT_B]->get_name().c_str() : "-");
}

/**
   [A]  [B]
    A -- ? <- X

   [A]  [B]
    A -- X
*/
void Channel::push_animation(PointerTo<Animation> animation) {
    _animations[SLOT_B] = animation;
    _frame_indices[SLOT_B] = 0.0;

    if ((_animations[SLOT_A] != NULL && !_animations[SLOT_A]->can_blend_out()) ||
            (_animations[SLOT_B] != NULL && !_animations[SLOT_B]->can_blend_in()))
        _factor = _blending_time;  // set to max

#ifdef KPHYS_DEBUG
    ls();
#endif
}

/**
   [A]  [B]
    A <- B <- #

   [A]  [B]
    B <- #
 */
void Channel::switch_animation() {
    _animations[SLOT_A] = _animations[SLOT_B];
    _frame_indices[SLOT_A] = _frame_indices[SLOT_B];
#ifdef KPHYS_DEBUG
    ls();
#endif
    push_animation(NULL);
    _factor = 0.0;  // new A = 100%
}

void Channel::update(double dt) {
    // increment frames
    for (unsigned short s = 0; s < NUM_SLOTS; s++) {
        if (_animations[s] == NULL || _animations[s]->is_manual())
            continue;

        if (s == SLOT_A && _factor >= _blending_time)
            continue;

        double index_delta = dt / _animations[s]->get_frame_time();
        double index = get_frame_index(s) + index_delta;

        unsigned long num_frames = _animations[s]->get_num_frames();
        if (_animations[s]->is_loop()) {  // loop
            // num_frames = 100 (frame 0...99)
            // frame = 100
            // frame >= 100 -> frame = 100 - 100 = 0
            while (index >= num_frames)
                index -= num_frames;
        } else {  // clamp
            index = fmin(index, num_frames - 1);
        }

        set_frame_index(s, index);
    }

    // update blending factor
    _factor = fmin(_factor + dt, _blending_time);

#ifdef KPHYS_DEBUG
    if (get_factor() > 0.0 && get_factor() < 1.0)
        ls();
#endif
}
