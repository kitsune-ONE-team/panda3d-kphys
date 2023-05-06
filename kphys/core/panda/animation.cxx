#include "kphys/core/panda/animation.h"


TypeHandle Animation::_type_handle;

Animation::Animation(const char* name): Namable(name) {}

Animation::~Animation() {
    _motion.clear();
}

unsigned long Animation::_get_frame_index(long frame) {
    unsigned long num_frames = get_num_frames();
    while (frame < 0)
        frame = num_frames - frame;
    while (frame >= num_frames)
        frame -= num_frames;
    return (unsigned long) frame;
}

unsigned long Animation::get_num_frames() {
    return _motion.size();
}

Frame* Animation::get_frame(unsigned long i) {
    return _motion.at(_get_frame_index(i));
}

double Animation::get_frame_time() {
    return _frame_time;
}
