#ifndef PANDA_TYPES_H
#define PANDA_TYPES_H

#include <unordered_map>
#include "nodePath.h"
#include "pmap.h"

#define MAX_BONES 256
#define FLOAT_SIZE 4
#define MAT4_SIZE (MAT4_WIDTH * MAT4_HEIGHT * FLOAT_SIZE)
#define MAT4_WIDTH 4
#define MAT4_HEIGHT 4
#define RGBA_CHANNEL_COUNT 4
#define RGBA_MAT4_SIZE ((MAT4_WIDTH * MAT4_HEIGHT) / RGBA_CHANNEL_COUNT)

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)
#define ISCLOSE(a, b) (fabs(a - b) <= CMP_EPSILON)

// #define KDICT pmap
#define KDICT std::unordered_map

typedef union LMatrix4Array {
    LMatrix4 matrices[MAX_BONES];
    unsigned char data[MAT4_SIZE * MAX_BONES];
} LMatrix4Array;

LMatrix4 get_matrix(LMatrix4Array* array, unsigned short i);
void set_matrix(LMatrix4Array* array, unsigned short i, LMatrix4 matrix);
bool is_armature(NodePath np);
bool is_bone(NodePath np);
bool is_wiggle_bone(NodePath np);
bool is_any_bone(NodePath np);
bool is_rigid_body(NodePath np);
bool is_effector(NodePath np);
bool is_animator(NodePath np);
NodePath get_armature(NodePath np);

#endif
