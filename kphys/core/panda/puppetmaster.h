#ifndef PANDA_PUPPET_MASTER_H
#define PANDA_PUPPET_MASTER_H

#include "pandaNode.h"
#include "kphys/core/panda/puppet.h"


class EXPORT_CLASS PuppetMasterNode: public PandaNode {
 PUBLISHED:
    PuppetMasterNode(const char* name);
    ~PuppetMasterNode();
    void build();
    void start();

 private:
    pvector<PointerTo<Puppet>> _puppets;

    static TypeHandle _type_handle;

 public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        PandaNode::init_type();
        register_type(_type_handle, "PuppetMasterNode", PandaNode::get_class_type());
    }
    virtual TypeHandle get_type() const {
        return get_class_type();
    }
    virtual TypeHandle force_init_type() {
        init_type();
        return get_class_type();
    }
};

#endif
