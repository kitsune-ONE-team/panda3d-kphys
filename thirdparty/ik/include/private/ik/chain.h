/*!
 * @file chain.h
 * @brief Manages synthesising the user specified tree of nodes (ik_node_t)
 * into a structure more optimal for solvers.
 *
 *
 */
#ifndef IK_CHAIN_H
#define IK_CHAIN_H

#include "ik/config.h"
#include "ik/vector.h"

C_BEGIN

struct ik_node_t;

struct chain_t
{
    /*
     * List of ik_node_t* references that belong to this chain.
     * NOTE: The nodes are in "reverse", i.e. the first node in this list is
     * the effector node.
     */
    struct vector_t nodes;
    /* list of chain_t objects */
    struct vector_t children;
};

IK_PRIVATE_API struct chain_t*
chain_create(void);

IK_PRIVATE_API void
chain_destroy(struct chain_t* chain);

/*!
 * @brief Initializes an allocated chain object.
 */
IK_PRIVATE_API void
chain_construct(struct chain_t* chain);

/*!
 * @brief Destroys and frees all members, but does not deallocate the chain
 * object itself.
 */
IK_PRIVATE_API void
chain_destruct(struct chain_t* chain);

/*!
 * @brief Deletes all children and nodes.
 */
IK_PRIVATE_API void
chain_clear_free(struct chain_t* chain);

IK_PRIVATE_API struct chain_t*
chain_create_child(struct chain_t* chain);

IK_PRIVATE_API ikret_t
chain_add_node(struct chain_t* chain, const struct ik_node_t* node);

/*!
 * @brief Breaks down the relevant nodes of the scene graph into a tree of
 * chains. FABRIK can then more efficiently solve each chain individually.
 *
 * A "sub-base joint" is a node in the scene graph where at least two end
 * effector nodes eventually join together. FABRIK only works on single
 * chains of joints at a time. The end position of every sub-base joint is
 * the average of the resulting multiple positions after running FABRIK on
 * each chain. Said average position becomes the new target position for
 * the next chain connected to it.
 *
 * This algorithm finds all sub-base joints and generates chains between
 * base, sub-base joints, and end effectors. These chains are inserted into
 * the chain tree.
 */
IK_PRIVATE_API ikret_t
chain_tree_rebuild(struct vector_t* chain_list,
                   const struct ik_node_t* base_node,
                   const struct vector_t* effector_nodes_list);

/*!
 * Computes the distances between the nodes and stores them in
 * node->segment_length. The positions used for this computation are those of
 * the active pose (node->position). For this reason, make sure that you've
 * correctly initialized the active pose before calling this function.
 *
 * The segment lengths are typically computed once during initialisation and
 * then never again. Of course, there are exceptions, such as when your tree
 * has translational motions. In this case, you will have to recalculate the
 * segment lengths every time node positions change.
 */
IK_PRIVATE_API void
update_distances(const struct vector_t* chains);

/*!
 * @brief Counts all of the chains in the tree.
 */
IK_PRIVATE_API int
count_chains(const struct vector_t* chains);

/*!
 * @brief Helper macro for retrieving the node by index.
 * @note Does no error checking at all (e.g. if the index is out of bounds).
 */
#define chain_get_node(chain_var, idx) \
    (*(struct ik_node_t**)vector_get_element(&(chain_var)->nodes, idx))

/*!
 * @brief Helper macro for retrieving the number of nodes in a chain.
 */
#define chain_length(chain_var) \
    vector_count(&(chain_var)->nodes)

/*!
 * @brief Helper macro for retrieving the base node in the chain.
 * @note Does no error checking at all.
 */
#define chain_get_base_node(chain_var) \
    (*(struct ik_node_t**)vector_get_element(&(chain_var)->nodes, \
                                chain_length(chain_var) - 1))

/*!
 * @brief Helper macro for retrieving the last node in the chain.
 * @note Does no error checking at all.
 */
#define chain_get_tip_node(chain_var) \
    chain_get_node(chain_var, 0)

#define CHAIN_FOR_EACH_CHILD(chain_var, var_name) \
    VECTOR_FOR_EACH(&(chain_var)->children, struct chain_t, var_name) {

#define CHAIN_FOR_EACH_NODE(chain_var, var_name) \
    VECTOR_FOR_EACH(&(chain_var)->nodes, struct ik_node_t*, chain_##var_name) \
    struct ik_node_t* var_name = *(chain_##var_name); {

#define CHAIN_END_EACH VECTOR_END_EACH }

#ifdef IK_DOT_OUTPUT
/*!
 * @brief Dumps the chain tree to DOT format.
 * @param[in] base The base node of the user created tree. This is a parameter
 * because the base chain does not necessarily hold the base node of the tree
 * because the base node doesn't have to be part of the IK problem.
 * @note Doesn't necessarily have to be the base node, it will dump the tree
 * beginning at this node.
 * @param[in] chains A vector of base chains to dump.
 * @param[in] file_name The name of the file to dump to.
 */
IK_PRIVATE_API void
dump_to_dot(const struct ik_node_t* node, const struct vector_t* chains, const char* file_name);
#endif /* IK_DOT_OUTPUT */

C_END

#endif /* IK_CHAIN_H */
