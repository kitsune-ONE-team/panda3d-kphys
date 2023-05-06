#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nodePath.h"

#include "kphys/core/panda/armature.h"
#include "kphys/core/panda/bone.h"

#include "kphys/core/panda/bvhq.h"

#define WORD_MAX_LEN 256
#define MAX_CHANNELS 10


TypeHandle BVHQ::_type_handle;

BVHQ::BVHQ(const char* name, const char* data): Animation(name) {
    _offset = 0;
    _data = (char*) malloc(sizeof(char) * strlen(data));
    strcpy(_data, data);

    char* word = (char*) malloc(sizeof(char) * WORD_MAX_LEN);
    char end = ' ';

    unsigned long num_frames = 0;

    while (end != '\0') {
        end = _readword(word, WORD_MAX_LEN);
        if (strcmp(word, "HIERARCHY") == 0)
            break;
    }
    if (end == '\0')  // no hierarchy section
        goto except;

    while (end != '\0') {  // HIERARCHY
        end = _readword(word, WORD_MAX_LEN);

        if (strcmp(word, "ROOT") == 0 || strcmp(word, "JOINT") == 0) {
            end = _readword(word, WORD_MAX_LEN);  //  read bone name
            if (end == '\0')  // no bone name
                goto except;

            BVHQJoint* joint = new BVHQJoint();
            joint->name = (char*) malloc(sizeof(char) * strlen(word));
            joint->channels.clear();
            strcpy(joint->name, word);
            _hierarchy.push_back(joint);
            // printf("JOINT %s\n", joint->name);

        } else if (strcmp(word, "CHANNELS") == 0) {
            end = _readword(word, WORD_MAX_LEN);  // read channels count
            if (end == '\0')  // no channels count
                goto except;

            unsigned short bone_num_channels = atoi(word);
            // printf("JOINT %s CHANNELS %d\n", _hierarchy.back()->name, bone_num_channels);
            for (unsigned short i = 0; i < bone_num_channels; i++) {
                end = _readword(word, WORD_MAX_LEN);  // read channel

                char* channel = (char*) malloc(sizeof(char) * strlen(word));
                strcpy(channel, word);
                // printf("JOINT %s CHANNEL %s\n", _hierarchy.back()->name, channel);
                _hierarchy.back()->channels.push_back(channel);

                if (end == '\0' || end == '\n')  // no more channels
                    break;
            }

        } else if (strcmp(word, "MOTION") == 0) {
            break;
        }
    }
    if (end == '\0')  // no motion section
        goto except;

    end = _readword(word, WORD_MAX_LEN);
    while (end != '\0') {  // MOTION
        if (strcmp(word, "Frames:") == 0) {
            end = _readword(word, WORD_MAX_LEN);  // read frames count
            num_frames = atol(word);
            if (end == '\0')  // end of motion section
                goto finally;

        } else if (strcmp(word, "Frame") == 0) {
            end = _readword(word, WORD_MAX_LEN);  // read next keyword
            if (strcmp(word, "Time:") == 0) {
                end = _readword(word, WORD_MAX_LEN);  // read frame time
                _frame_time = atof(word);
                if (end == '\0')  // end of motion section
                    goto finally;
            }
            if (end == '\0')  // end of motion section
                goto finally;

        } else if (num_frames) {
            for (unsigned long iframe = 0; iframe < num_frames; iframe++) {
                Frame* frame = new Frame();

                for (BVHQJoint* joint: _hierarchy) {
                    LVecBase3 pos, hpr;
                    LQuaternion quat;
                    bool has_pos, has_hpr, has_quat;

                    for (char* channel: joint->channels) {
                        if (strcmp(channel, "Xposition") == 0) {
                            pos.set_x(atof(word));
                            has_pos = true;
                        }
                        if (strcmp(channel, "Yposition") == 0) {
                            pos.set_y(atof(word));
                            has_pos = true;
                        }
                        if (strcmp(channel, "Zposition") == 0) {
                            pos.set_z(atof(word));
                            has_pos = true;
                        }

                        if (strcmp(channel, "Xrotation") == 0) {
                            hpr.set_x(atof(word));
                            has_hpr = true;
                        }
                        if (strcmp(channel, "Yrotation") == 0) {
                            hpr.set_y(atof(word));
                            has_hpr = true;
                        }
                        if (strcmp(channel, "Zrotation") == 0) {
                            hpr.set_z(atof(word));
                            has_hpr = true;
                        }

                        if (strcmp(channel, "Irotation") == 0) {
                            quat.set_i(atof(word));
                            has_quat = true;
                        }
                        if (strcmp(channel, "Jrotation") == 0) {
                            quat.set_j(atof(word));
                            has_quat = true;
                        }
                        if (strcmp(channel, "Krotation") == 0) {
                            quat.set_k(atof(word));
                            has_quat = true;
                        }
                        if (strcmp(channel, "Rrotation") == 0) {
                            quat.set_r(atof(word));
                            has_quat = true;
                        }

                        if (end == '\0')  // end of frame or motion section
                            break;  // leave channels

                        end = _readword(word, WORD_MAX_LEN);  // read channel value

                        if (end == '\n')  // end of frame or motion section
                            break;  // leave channels
                    }  // bone channels

                    ConstPointerTo<TransformState> transform = NULL;

                    if (has_pos && has_hpr && !has_quat)
                        transform = TransformState::make_pos_hpr(pos, hpr);
                    else if (has_pos && !has_hpr && has_quat)
                        // there is no make_pos_quat() method
                        transform = TransformState::make_pos_quat_scale(
                            pos, quat, LVecBase3(1, 1, 1));
                    else if (has_pos && !has_hpr && !has_quat)
                        transform = TransformState::make_pos(pos);
                    else if (!has_pos && has_hpr && !has_quat)
                        transform = TransformState::make_hpr(hpr);
                    else if (!has_pos && !has_hpr && has_quat)
                        transform = TransformState::make_quat(quat);

                    if (transform != NULL) {
                        frame->add_transform(joint->name, transform);
                    }

                    if (end == '\n' || end == '\0')  // end of frame or motion section
                        break;  // leave bones
                }  // bone

                _motion.push_back(frame);

                if (end == '\0')  // end of motion section
                    break;  // leave frames
            }  // frame

            break;
        }  // has frames count

        end = _readword(word, WORD_MAX_LEN);
    }

    goto finally;

    except:
        // TODO: error message

    finally:
        free(word);
}

BVHQ::~BVHQ() {
    free(_data);
    for (BVHQJoint* joint: _hierarchy) {
        free(joint->name);
        for (char* channel: joint->channels) {
            free(channel);
        }
        joint->channels.clear();
        free(joint);
    }
    _hierarchy.clear();
}

char BVHQ::_readword(char* word, unsigned long size) {
    unsigned long i = 0;
    char c;
    do {
        c = _data[_offset++];
        word[i++] = c;
    } while (c != ' ' && c != '\t' && c != '\n' && c != '\0' && i <= size - 2);
    word[i - 1] = '\0';
    return c;
}
