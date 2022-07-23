#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/node.h"
#include "ik/util.h"
#include <assert.h>

typedef struct effector_data_t
{
    ikreal_t rotation_weight;
    ikreal_t rotation_weight_decay;
} effector_data_t;

/* ------------------------------------------------------------------------- */
static effector_data_t
calculate_rotation_weight_decays_recursive(struct chain_t* chain)
{
    int node_idx, node_count;

    /*
     * Find the rotation weight of this chain's last node by averaging the
     * rotation weight of all child chain's first nodes.
     *
     * If there are no child chains, then the first node in the chain must
     * contain an effector. Initialise the weight parameters from said
     * effector.
     *
     * If there are child chains then average the effector data we've
     * accumulated.
     */
    int average_count = 0;
    effector_data_t effector_data = { 0.0, 0.0 };
    CHAIN_FOR_EACH_CHILD(chain, child)
        effector_data_t child_eff_data =
                calculate_rotation_weight_decays_recursive(child);
        effector_data.rotation_weight += child_eff_data.rotation_weight;
        effector_data.rotation_weight_decay += child_eff_data.rotation_weight_decay;
        ++average_count;
    CHAIN_END_EACH

    if (average_count == 0)
    {
        struct ik_node_t* effector_node = chain_get_tip_node(chain);
        struct ik_effector_t* effector = effector_node->effector;

        effector_data.rotation_weight = effector->rotation_weight;
        effector_data.rotation_weight_decay = effector->rotation_decay;
    }
    else
    {
        ikreal_t div = 1.0 / average_count;
        effector_data.rotation_weight *= div;
        effector_data.rotation_weight_decay *= div;
    }

    /*
     * With the rotation weight of the last node calculated, we can now iterate
     * the nodes in the chain and set each rotation weight, decaying a little
     * bit every time. Note that we exclude the last node, because it will
     * be handled by the parent chain. If there is no parent chain then the
     * non-recursive caller of this function will set the rotation weight of
     * the base node.
     */
    node_count = chain_length(chain) - 1;
    for (node_idx = 0; node_idx < node_count; ++node_idx)
    {
        struct ik_node_t* node = chain_get_node(chain, node_idx);
        node->rotation_weight = effector_data.rotation_weight;
        effector_data.rotation_weight *= effector_data.rotation_weight_decay;
    }

    /* Rotation weight is now equal to that of this chain's base node */
    return effector_data;
}

/* ------------------------------------------------------------------------- */
void
ik_calculate_rotation_weight_decays(const struct vector_t* chains)
{
    /*
     * The recursive version of this function does not set the rotation weight
     * of the first node in every tree that gets passed to it, but it does
     * return the rotation weight that *would have been set* (which gets used
     * recursively to calculate the average rotation weight in the case of
     * multiple child chains).
     *
     * For these reasons we iterate the chain islands and set the first node in
     * each island to the returned rotation weight.
     */
    VECTOR_FOR_EACH(chains, struct chain_t, base_chain)
        effector_data_t effector_data = calculate_rotation_weight_decays_recursive(base_chain);
        assert(chain_length(base_chain) >= 2);
        chain_get_base_node(base_chain)->rotation_weight = effector_data.rotation_weight;
    VECTOR_END_EACH
}
