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

BVHQ::BVHQ(const char* name, Filename filename, bool local_space, bool debug):
        Animation(name, local_space) {
    VirtualFileSystem* vfs = VirtualFileSystem::get_global_ptr();
    _istream = vfs->open_read_file(filename, true);

    _frame_time = 0;

    char word[WORD_MAX_LEN];
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

            BVHQJoint* joint = new BVHQJoint();
            joint->name = (char*) malloc(sizeof(char) * strlen(word));
            strcpy(joint->name, word);
            _hierarchy.push_back(joint);
            if (debug)
                printf("JOINT %s\n", joint->name);

        } else if (strcmp(word, "CHANNELS") == 0) {
            if (end == '\0' || end == '\n') {
                exception = NO_CHANNELS;
                goto except;
            }
            end = _readword(word, WORD_MAX_LEN);  // read channels count

            unsigned short bone_num_channels = atoi(word);
            if (debug)
                printf("JOINT %s CHANNELS %d\n", _hierarchy.back()->name, bone_num_channels);

            if (end == '\0' || end == '\n') {
                exception = NO_CHANNELS;
                goto except;
            }

            for (unsigned short i = 0; i < bone_num_channels; i++) {
                end = _readword(word, WORD_MAX_LEN);  // read channel

                char* channel = (char*) malloc(sizeof(char) * strlen(word));
                strcpy(channel, word);
                if (debug)
                    printf("JOINT %s CHANNEL %s\n", _hierarchy.back()->name, channel);
                _hierarchy.back()->channels.push_back(channel);

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

                Frame* frame = new Frame();
                unsigned int bi = 0;

                for (BVHQJoint* joint: _hierarchy) {
                    LVecBase3 pos, hpr;
                    LQuaternion quat;
                    bool has_pos = false, has_hpr = false, has_quat = false;
                    unsigned ci = 0;

                    for (char* channel: joint->channels) {
                        if (iframe > 0 || bi > 0 || ci > 0)
                            end = _readword(word, WORD_MAX_LEN);  // read channel value

                        // if (debug)
                        //     printf("CHANNEL %s %s %s\n", joint->name, channel, word);

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

                        ci++;
                        if (end == '\0' || end == '\n')
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
                        frame->add_transform(
                            joint->name, transform, has_pos, has_hpr, has_quat);
                    }

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
        if (debug)
            printf("DONE\n");
        vfs->close_read_file(_istream);
        if (debug)
            printf("FILE CLOSED\n");

}

BVHQ::~BVHQ() {
    for (BVHQJoint* joint: _hierarchy) {
        // free(joint->name);
        // for (char* channel: joint->channels) {
        //     free(channel);
        // }
        joint->channels.clear();
        // free(joint);
    }
    _hierarchy.clear();
}

char BVHQ::_readword(char* word, unsigned long size) {
    unsigned long i = 0;
    char c = '*';

    while (c != ' ' && c != '\t' && c != '\n' && c != '\0' &&
           i < (size - 1) && !_istream->eof()) {
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
