#include "Python.h"
#include "ik/ik.h"
#include "ik/python/ik_module_info.h"
#include "ik/python/ik_module_log.h"
#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Solver.h"
#include "ik/python/ik_type_Vec3.h"

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

/* ------------------------------------------------------------------------- */
static void
module_free(void* x)
{
    (void)x;
    IKAPI.deinit();
}

/* ------------------------------------------------------------------------- */
static PyModuleDef ik_module = {
    PyModuleDef_HEAD_INIT,
    EXPAND_AND_QUOTE(IKAPI), /* Module name */
    NULL,                    /* docstring, may be NULL */
    -1,                      /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    NULL,                    /* module methods */
    NULL,                    /* m_reload */
    NULL,                    /* m_traverse */
    NULL,                    /* m_clear */
    module_free              /* m_free */
};

/* ------------------------------------------------------------------------- */
static int
init_builtin_types(void)
{
    if (init_ik_ConstraintType() != 0) return -1;
    if (init_ik_EffectorType() != 0)   return -1;
    if (init_ik_NodeType() != 0)       return -1;
    if (init_ik_QuatType() != 0)       return -1;
    if (init_ik_SolverType() != 0)     return -1;
    if (init_ik_Vec3Type() != 0)       return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
    Py_INCREF(&ik_ConstraintType); if (PyModule_AddObject(m, "Constraint", (PyObject*)&ik_ConstraintType) < 0) return -1;
    Py_INCREF(&ik_EffectorType);   if (PyModule_AddObject(m, "Effector",   (PyObject*)&ik_EffectorType) < 0)   return -1;
    Py_INCREF(&ik_NodeType);       if (PyModule_AddObject(m, "Node",       (PyObject*)&ik_NodeType) < 0)       return -1;
    Py_INCREF(&ik_QuatType);       if (PyModule_AddObject(m, "Quat",       (PyObject*)&ik_QuatType) < 0)       return -1;
    Py_INCREF(&ik_SolverType);     if (PyModule_AddObject(m, "Solver",     (PyObject*)&ik_SolverType) < 0)     return -1;
    Py_INCREF(&ik_Vec3Type);       if (PyModule_AddObject(m, "Vec3",       (PyObject*)&ik_Vec3Type) < 0)       return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_submodules_to_module(PyObject* m)
{
    PyObject* submodule;

    submodule = ik_module_info_create();
    if (submodule == NULL)
        return -1;
    if (PyModule_AddObject(m, "info", submodule) < 0)
    {
        Py_DECREF(submodule);
        return -1;
    }

    submodule = ik_module_log_create();
    if (submodule == NULL)
        return -1;
    if (PyModule_AddObject(m, "log", submodule) < 0)
    {
        Py_DECREF(submodule);
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
#define PASTER(x, y) x ## y
#define EVALUATOR(x, y) PASTER(x, y)
PyMODINIT_FUNC EVALUATOR(PyInit_, IKAPI)(void)
{
    PyObject* m;

    if (IKAPI.init() != IK_OK)
        goto ik_init_failed;

    m = PyModule_Create(&ik_module);
    if (m == NULL)
        goto module_alloc_failed;

    if (init_builtin_types() != 0)            goto init_module_failed;
    if (add_builtin_types_to_module(m) != 0)  goto init_module_failed;
    if (add_submodules_to_module(m) != 0)     goto init_module_failed;

    return m;

    init_module_failed            : Py_DECREF(m);
    module_alloc_failed           : IKAPI.deinit();
    ik_init_failed                : return NULL;
}
