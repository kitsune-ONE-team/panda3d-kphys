#include "threadPriority.h"
#include "kphys/core/panda/puppetmaster.h"


TypeHandle PuppetMasterNode::_type_handle;

PuppetMasterNode::PuppetMasterNode(const std::string name): PandaNode(name) {}

PuppetMasterNode::~PuppetMasterNode() {}

void PuppetMasterNode::build() {
    NodePath master = NodePath::any_path(this);

    NodePathCollection nps = master.find_all_matches("**/+AnimatorNode");
    int num_nps = nps.get_num_paths();
    for (int i = 0; i < num_nps; i++) {
        NodePath np = nps.get_path(i);
        PointerTo<Puppet> puppet = new Puppet(np.get_name(), np.get_name(), np);
        _puppets.push_back(puppet);
    }

    nps = master.find_all_matches("**/+MultiAnimatorNode");
    num_nps = nps.get_num_paths();
    for (int i = 0; i < num_nps; i++) {
        NodePath np = nps.get_path(i);
        PointerTo<Puppet> puppet = new Puppet(np.get_name(), np.get_name(), np);
        _puppets.push_back(puppet);
    }
}

void PuppetMasterNode::start() {
    for (PointerTo<Puppet> puppet : _puppets) {
        puppet->start(ThreadPriority::TP_low, true);
    }
}
