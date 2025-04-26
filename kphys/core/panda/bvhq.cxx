#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nodePath.h"
#include "virtualFileSystem.h"

#include "kphys/core/panda/armature.h"
#include "kphys/core/panda/bone.h"

#include "kphys/core/panda/bvhq.h"

#define WORD_MAX_LEN 256
#define MAX_CHANNELS 10

#define NO_HIERARCHY 1
#define NO_MOTION 2
#define NO_JOINT_NAME 3
#define NO_CHANNELS 4
#define NO_FRAMES 5
#define NO_FRAME_TIME 6


TypeHandle BVHQ::_type_handle;
TypeHandle BVHQJoint::_type_handle;


BVHQJoint::BVHQJoint(const std::string name): Namable(name) {
}

BVHQJoint::~BVHQJoint() {
    _channels.clear();
}

unsigned long BVHQJoint::get_num_channels() {
    return _channels.size();
}

std::string BVHQJoint::get_channel(unsigned long i) {
    return _channels[i];
}

void BVHQJoint::add_channel(std::string name) {
    return _channels.push_back(name);
}


BVHQ::BVHQ(const std::string name, Filename filename, bool local_space, bool debug):
        Animation(name, local_space) {
    if (debug)
        printf("FILE OPEN %s\n", name.c_str());

    VirtualFileSystem* vfs = VirtualFileSystem::get_global_ptr();
    _istream = vfs->open_read_file(filename, true);

    _frame_time = 0;

    char* word = (char*) malloc(sizeof(char) * (WORD_MAX_LEN + 1));
    char end = ' ';
    unsigned int exception = 0;

    unsigned long num_frames = 0;

    while (end != '\0') {
        end = _readword(word, WORD_MAX_LEN);
        if (strcmp(word, "HIERARCHY") == 0)
            break;
    }
    if (end == '\0') {
        exception = NO_HIERARCHY;
        goto except;
    }

    while (end != '\0') {  // HIERARCHY
        end = _readword(word, WORD_MAX_LEN);

        if (strcmp(word, "ROOT") == 0 || strcmp(word, "JOINT") == 0) {
            if (end == '\0' || end == '\n') {
                exception = NO_JOINT_NAME;
                goto except;
            }
            end = _readword(word, WORD_MAX_LEN);  //  read bone name

            char* name = (char*) malloc(sizeof(char) * (strlen(word) + 1));
            strcpy(name, word);

            PointerTo<BVHQJoint> joint = new BVHQJoint(std::string(name));
            _hierarchy.push_back(joint);

            if (debug)
                printf("JOINT %s\n", joint->get_name().c_str());

        } else if (strcmp(word, "CHANNELS") == 0) {
            if (end == '\0' || end == '\n') {
                exception = NO_CHANNELS;
                goto except;
            }
            end = _readword(word, WORD_MAX_LEN);  // read channels count

            unsigned short bone_num_channels = atoi(word);
            if (debug)
                printf("JOINT %s CHANNELS %d\n", _hierarchy.back()->get_name().c_str(), bone_num_channels);

            if (end == '\0' || end == '\n') {
                exception = NO_CHANNELS;
                goto except;
            }

            for (unsigned short i = 0; i < bone_num_channels; i++) {
                end = _readword(word, WORD_MAX_LEN);  // read channel

                char* channel = (char*) malloc(sizeof(char) * (strlen(word) + 1));
                strcpy(channel, word);

                _hierarchy.back()->add_channel(std::string(channel));
                if (debug)
                    printf("JOINT %s CHANNEL %s\n", _hierarchy.back()->get_name().c_str(), channel);

                if (end == '\n')  // no more channels
                    break;

                if (end == '\0')  // no more channels
                    goto finally;
            }

        } else if (strcmp(word, "MOTION") == 0) {
            break;  // leave hierarchy
        }
    }  // HIERARCHY
    if (end == '\0') {
        exception = NO_MOTION;
        goto except;
    }

    while (end != '\0') {  // MOTION
        end = _readword(word, WORD_MAX_LEN);

        if (strcmp(word, "Frames:") == 0) {
            if (end == '\0' || end == '\n') {
                exception = NO_FRAMES;
                goto except;
            }
            end = _readword(word, WORD_MAX_LEN);  // read frames count
            num_frames = atol(word);
            if (debug)
                printf("FRAMES %ld\n", num_frames);
            if (end == '\0')
                goto finally;

        } else if (strcmp(word, "Frame") == 0) {
            if (end == '\0' || end == '\n') {
                exception = NO_FRAME_TIME;
                goto except;
            }

            end = _readword(word, WORD_MAX_LEN);  // read next keyword
            if (strcmp(word, "Time:") == 0) {
                if (end == '\0' || end == '\n') {
                    exception = NO_FRAME_TIME;
                    goto except;
                }
                end = _readword(word, WORD_MAX_LEN);  // read frame time
                _frame_time = atof(word);
                if (debug)
                    printf("FRAME TIME %.6f\n", _frame_time);
                if (end == '\0')  // end of motion section
                    goto finally;
            }
            if (end == '\0')  // end of motion section
                goto finally;

        } else if (num_frames) {
            for (unsigned long iframe = 0; iframe < num_frames; iframe++) {
                if (debug)
                    printf("FRAME %ld\n", iframe);

                PointerTo<Frame> frame = new Frame();
                unsigned int bi = 0;

                for (PointerTo<BVHQJoint> joint: _hierarchy) {
                    LVecBase3 pos, hpr;
                    LQuaternion quat;
                    unsigned short flags = 0;

                    for (unsigned long ci = 0; ci < joint->get_num_channels(); ci++) {
                        std::string channel = joint->get_channel(ci);

                        if (iframe > 0 || bi > 0 || ci > 0)
                            end = _readword(word, WORD_MAX_LEN);  // read channel value

                        // if (debug)
                        //     printf("CHANNEL %s %s %s\n", joint->name, channel, word);

                        if (strcmp(channel.c_str(), "Xposition") == 0) {
                            pos.set_x(atof(word));
                            flags |= TRANSFORM_POS;
                        }
                        if (strcmp(channel.c_str(), "Yposition") == 0) {
                            pos.set_y(atof(word));
                            flags |= TRANSFORM_POS;
                        }
                        if (strcmp(channel.c_str(), "Zposition") == 0) {
                            pos.set_z(atof(word));
                            flags |= TRANSFORM_POS;
                        }

                        if (strcmp(channel.c_str(), "Xrotation") == 0) {
                            hpr.set_x(atof(word));
                            flags |= TRANSFORM_HPR;
                        }
                        if (strcmp(channel.c_str(), "Yrotation") == 0) {
                            hpr.set_y(atof(word));
                            flags |= TRANSFORM_HPR;
                        }
                        if (strcmp(channel.c_str(), "Zrotation") == 0) {
                            hpr.set_z(atof(word));
                            flags |= TRANSFORM_HPR;
                        }

                        if (strcmp(channel.c_str(), "Irotation") == 0) {
                            quat.set_i(atof(word));
                            flags |= TRANSFORM_QUAT;
                        }
                        if (strcmp(channel.c_str(), "Jrotation") == 0) {
                            quat.set_j(atof(word));
                            flags |= TRANSFORM_QUAT;
                        }
                        if (strcmp(channel.c_str(), "Krotation") == 0) {
                            quat.set_k(atof(word));
                            flags |= TRANSFORM_QUAT;
                        }
                        if (strcmp(channel.c_str(), "Rrotation") == 0) {
                            quat.set_r(atof(word));
                            flags |= TRANSFORM_QUAT;
                        }

                        if (end == '\0' || end == '\n')
                            break;  // leave channels
                    }  // bone channels

                    ConstPointerTo<TransformState> transform = NULL;

                    if (flags & TRANSFORM_POS && flags & TRANSFORM_HPR && !(flags & TRANSFORM_QUAT))
                        transform = TransformState::make_pos_hpr(pos, hpr);
                    else if (flags & TRANSFORM_POS && !(flags & TRANSFORM_HPR) && flags & TRANSFORM_QUAT)
                        // there is no make_pos_quat() method
                        transform = TransformState::make_pos_quat_scale(
                            pos, quat, LVecBase3(1, 1, 1));
                    else if (flags & TRANSFORM_POS && !(flags & TRANSFORM_HPR) && !(flags & TRANSFORM_QUAT))
                        transform = TransformState::make_pos(pos);
                    else if (!(flags & TRANSFORM_POS) && flags & TRANSFORM_HPR && !(flags & TRANSFORM_QUAT))
                        transform = TransformState::make_hpr(hpr);
                    else if (!(flags & TRANSFORM_POS) && !(flags & TRANSFORM_HPR) && flags & TRANSFORM_QUAT)
                        transform = TransformState::make_quat(quat);

                    if (transform != NULL)
                        frame->set_transform(joint->get_name(), transform, flags);

                    bi++;
                    if (end == '\0' || end == '\n')
                        break;  // leave bones
                }  // bone

                _motion.push_back(frame);

                if (end == '\0')
                    break;  // leave frames
            }  // frame
            break;  // leave motion
        }  // has frames count
    }  // MOTION

    goto finally;

    except:
        if (debug)
            switch(exception) {
            case NO_HIERARCHY:
                printf("HIERARCHY not found!\n");
                break;
            case NO_MOTION:
                printf("MOTION not found!\n");
                break;
            case NO_JOINT_NAME:
                printf("JOINT has no name!\n");
                break;
            case NO_CHANNELS:
                printf("CHANNELS not found!\n");
                break;
            case NO_FRAMES:
                printf("FRAMES not found!\n");
                break;
            case NO_FRAME_TIME:
                printf("FRAME_TIME not found!\n");
                break;
            default:
                break;
            }

    finally:
        free(word);
        if (debug)
            printf("DONE\n");
        vfs->close_read_file(_istream);
        if (debug)
            printf("FILE CLOSED\n");

}

BVHQ::~BVHQ() {
    _hierarchy.clear();
}

char BVHQ::_readword(char* word, unsigned long size) {
    unsigned long i = 0;
    char c = '*';

    while (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\0' &&
           i < size && !_istream->eof()) {
        _istream->get(c);
        word[i++] = c;
    }

    if (i == 0) {
        word[i] = '\0';
        return '\0';
    } else {
        word[i - 1] = '\0';
        return c;
    }
}
