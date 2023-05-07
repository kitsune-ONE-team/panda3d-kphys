#include <math.h>

#include "kphys/core/panda/channel.h"

#define NUM_SLOTS 2
#define SLOT_A 0
#define SLOT_B 1


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

void Channel::include_bone(char* name) {
    if (_exclude_bones.find(name) == _exclude_bones.end()) {
        _include_bones[name] = true;
    } else {
        _exclude_bones.erase(name);
    }
}

void Channel::exclude_bone(char* name) {
    if (_include_bones.find(name) == _include_bones.end()) {
        _exclude_bones[name] = true;
    } else {
        _include_bones.erase(name);
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

double Channel::get_frame(unsigned short i) {
    return _frames[i];
}

void Channel::set_frame(unsigned short i, double frame) {
    _frames[i] = frame;
}

Animation* Channel::get_animation(unsigned short i) {
    return _animations[i];
}

/**
   [A]  [B]
    A -- ? <- X

   [A]  [B]
    A -- X
*/
void Channel::push_animation(Animation* animation) {
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
        if (_animations[i] && _animations[i]->is_manual())
            continue;

        double frame_delta = dt / _animations[i]->get_frame_time();
        double frame = _frames[i] + frame_delta;

        if (_animations[i] && _animations[i]->get_num_frames()) {
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
            set_frame(i, frame);
        }
    }

    // update factor
    if (_animations[SLOT_A] && _animations[SLOT_A]->can_blend_out() &&
            _animations[SLOT_B] && _animations[SLOT_B]->can_blend_in())
        _factor = umin(_factor + dt, _blending_time);
    else  // set to max
        _factor = _blending_time;
}
