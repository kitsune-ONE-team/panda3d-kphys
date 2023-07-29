#include <math.h>

#include "kphys/core/panda/channel.h"


unsigned long umin(unsigned long a, unsigned long b) {
    if (a < b)
        return a;
    else
        return b;
}


TypeHandle Channel::_type_handle;

Channel::Channel(const char* name): Namable(name) {
    for (unsigned short i = 0; i < NUM_SLOTS; i++) {
        _animations[i] = NULL;
        _frame_indices[i] = 0;
    }
    double _factor = 0;
}

Channel::~Channel() {
    for (unsigned short i = 0; i < NUM_SLOTS; i++) {
        _animations[i] = NULL;
        _frame_indices[i] = 0;
    }
    _include_bones.clear();
    _exclude_bones.clear();
}

void Channel::include_bone(const char* name) {
    std::string s = std::string(name);
    if (_exclude_bones.find(s) == _exclude_bones.end()) {
        _include_bones[s] = true;
    } else {
        _exclude_bones.erase(s);
    }
}

void Channel::exclude_bone(const char* name) {
    std::string s = std::string(name);
    if (_include_bones.find(s) == _include_bones.end()) {
        _exclude_bones[s] = true;
    } else {
        _include_bones.erase(s);
    }
}

unsigned int Channel::get_num_included_bones() {
    return _include_bones.size();
}

unsigned int Channel::get_num_excluded_bones() {
    return _exclude_bones.size();
}

bool Channel::is_bone_included(const char* name) {
    std::string s = std::string(name);
    if (_include_bones.find(s) == _include_bones.end())
        return false;
    return true;
}

bool Channel::is_bone_excluded(const char* name) {
    std::string s = std::string(name);
    if (_exclude_bones.find(s) == _exclude_bones.end())
        return false;
    return true;
}

bool Channel::is_bone_enabled(const char* name) {
    if (get_num_included_bones())  // whitelist
        return is_bone_included(name);
    else  // blacklist
        return !is_bone_excluded(name);
}

/**
   Returns factor of blending.
   0   -> A = 100%, B =   0%
   100 -> A =   0%, B = 100%
*/
double Channel::get_factor() {
    return _factor / _blending_time;
}

void Channel::set_blending_time(double t) {
    _blending_time = t;
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
PointerTo<Frame> Channel::get_frame(unsigned short slot, bool interpolate) {
    PointerTo<Animation> animation = get_animation(slot);
    if (animation == NULL)
        return NULL;

    double index = get_frame_index(slot);
    if (interpolate) {
        unsigned long i = (unsigned long) floor(index);
        unsigned long j = (unsigned long) ceil(index);
        PointerTo<Frame> frame_i = animation->get_frame(i);
        PointerTo<Frame> frame_j = animation->get_frame(j);
        return frame_i->mix(frame_j, fmod(index, 1));  // index % 1
    } else {
        unsigned long i = (unsigned long) round(index);
        return animation->get_frame(i);
    }
}

/**
   Returns blended animation frame.
*/
PointerTo<Frame> Channel::get_frame(bool interpolate) {
    double factor = get_factor();

    if (factor <= 0.0) {
        return get_frame(SLOT_A, interpolate);

    } else if (factor >= 1.0) {
        return get_frame(SLOT_B, interpolate);

    } else {
        PointerTo<Frame> frame_a = get_frame(SLOT_A, interpolate);
        PointerTo<Frame> frame_b = get_frame(SLOT_B, interpolate);

        if (frame_a != NULL && frame_b != NULL) {
            return frame_a->mix(frame_b, factor);
        }
    }

    return NULL;
}

/**
   Returns the animation in the specified slot.
*/
PointerTo<Animation> Channel::get_animation(unsigned short slot) {
    return _animations[slot];
}

/**
   [A]  [B]
    A -- ? <- X

   [A]  [B]
    A -- X
*/
void Channel::push_animation(PointerTo<Animation> animation) {
    _animations[SLOT_B] = animation;
    _frame_indices[SLOT_B] = 0;
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
    push_animation(NULL);
    _factor = 0;  // new A = 100%
}

void Channel::update(double dt) {
    // increment frames
    for (unsigned short i = 0; i < NUM_SLOTS; i++) {
        if (_animations[i] == NULL || _animations[i]->is_manual())
            continue;

        if (i == SLOT_A && _factor >= _blending_time)
            continue;

        double index_delta = dt / _animations[i]->get_frame_time();
        double index = get_frame_index(i) + index_delta;
        unsigned long num_frames = _animations[i]->get_num_frames();

        if (_animations[i]->is_loop()) {
            // num_frames = 100 (frame 0...99)
            // frame = 100
            // frame >= 100 -> frame = 100 - 100 = 0
            while (index >= num_frames)
                index -= num_frames;
        } else {
            index = fmin(index, num_frames - 1);
        }
        set_frame_index(i, index);
    }

    // update blending factor
    if (_animations[SLOT_A] != NULL && _animations[SLOT_A]->can_blend_out() &&
            _animations[SLOT_B] != NULL && _animations[SLOT_B]->can_blend_in())
        _factor = fmin(_factor + dt, _blending_time);
    else  // set to max
        _factor = _blending_time;
}
