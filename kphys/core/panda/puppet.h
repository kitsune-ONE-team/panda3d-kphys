#ifndef PANDA_PUPPET_H
#define PANDA_PUPPET_H

#include "nodePath.h"
#include "thread.h"


class EXPORT_CLASS Puppet: public Thread {
PUBLISHED:
    Puppet(const std::string &name, const std::string &sync_name, NodePath _animator);
    ~Puppet();

private:
    NodePath _animator;
    void thread_main();

    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        Thread::init_type();
        register_type(_type_handle, "Puppet", Thread::get_class_type());
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
