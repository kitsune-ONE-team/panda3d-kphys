#include "kphys/core/panda/animator.h"
#include "kphys/core/panda/puppet.h"


TypeHandle Puppet::_type_handle;

Puppet::Puppet(
        const std::string &name, const std::string &sync_name,
        NodePath animator):
        Thread(name, sync_name), _animator(animator) {}

Puppet::~Puppet() {}

void Puppet::thread_main() {
    ((AnimatorNode*) _animator.node())->apply();
    sleep(0.01);
}
