#include "kphys/core/panda/spring.h"

#define SPRING_CLASS SpringConstraint
#define BT_SPRING_CONSTRAINT btGeneric6DofSpringConstraint
#define BT_SPRING_CONSTRAINT_ARGS ,use_frame_a_ref
#include "kphys/core/panda/spring_common.cxx"
#undef SPRING_CLASS
#undef BT_SPRING_CONSTRAINT
#undef BT_SPRING_CONSTRAINT_ARGS

void SpringConstraint::set_linear_lower_limit(const LVecBase3 &linear_lower) {
    ((btGeneric6DofSpringConstraint*) _constraint)->setLinearLowerLimit(
        LVecBase3_to_btVector3(linear_lower));
}

void SpringConstraint::set_linear_upper_limit(const LVecBase3 &linear_upper) {
    ((btGeneric6DofSpringConstraint*) _constraint)->setLinearUpperLimit(
        LVecBase3_to_btVector3(linear_upper));
}
