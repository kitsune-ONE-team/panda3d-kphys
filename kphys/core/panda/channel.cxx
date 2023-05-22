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
        _frames[i] = 0;
    }
    double _factor = 0;
}

Channel::~Channel() {
    for (unsigned short i = 0; i < NUM_SLOTS; i++) {
        _animations[i] = NULL;
        _frames[i] = 0;
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

/**
   Returns factor of blending.
   0 -> A = 100%, B =   0%
   100 -> A =   0%, B = 100%
*/
double Channel::get_factor() {
  return (double) _factor / (double) _blending_time;
}

void Channel::set_blending_time(unsigned long t) {
    _blending_time = t;
}

double Channel::get_frame_index(unsigned short slot) {
    return _frames[slot];
}

void Channel::set_frame_index(unsigned short slot, double frame) {
    _frames[slot] = frame;
}

/**
   Returns the closest frame of the animation in the specified slot.
*/
PointerTo<Frame> Channel::get_frame(unsigned short slot) {
    PointerTo<Animation> animation = get_animation(slot);
    if (animation == NULL)
        return NULL;

    unsigned long i = (unsigned long) round(get_frame_index(slot));
    return animation->get_frame(i);
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
    _frames[SLOT_B] = 0;
}

/**
   [A]  [B]
    A <- B <- #

   [A]  [B]
    B <- #
 */
void Channel::switch_animation() {
    _animations[SLOT_A] = _animations[SLOT_B];
    _frames[SLOT_A] = _frames[SLOT_B];
    push_animation(NULL);
    _factor = 0;  // new A = 100%
}

void Channel::update(unsigned long dt) {
    // increment frames
    for (unsigned short i = 0; i < NUM_SLOTS; i++) {
        if (_animations[i] == NULL || _animations[i]->is_manual())
            continue;

        double frame_delta = dt / _animations[i]->get_frame_time_hns();
        double frame = _frames[i] + frame_delta;
        double num_frames = (double) _animations[i]->get_num_frames();

        if (_animations[i]->is_loop()) {
            // num_frames = 100 (frame 0...99)
            // frame = 100
            // frame >= 100 -> frame = 100 - 100 = 0
            while (frame >= num_frames)
                frame -= num_frames;
        } else {
            frame = fmin(frame, num_frames - 1);
        }
        set_frame_index(i, frame);
    }

    // update blending factor
    if (_animations[SLOT_A] != NULL && _animations[SLOT_A]->can_blend_out() &&
            _animations[SLOT_B] != NULL && _animations[SLOT_B]->can_blend_in())
        _factor = umin(_factor + dt, _blending_time);
    else  // set to max
        _factor = _blending_time;
}
