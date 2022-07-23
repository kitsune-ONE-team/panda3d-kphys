#include "Python.h"
#include "ik/ik.h"
#include "ik/python/ik_module_info.h"

/* ------------------------------------------------------------------------- */
static PyModuleDef ik_module_info = {
    PyModuleDef_HEAD_INIT,
    "info",                  /* Module name */
    NULL,                    /* docstring, may be NULL */
    -1,                      /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    NULL,                    /* module methods */
    NULL,                    /* m_reload */
    NULL,                    /* m_traverse */
    NULL,                    /* m_clear */
    NULL                     /* m_free */
};

/* ------------------------------------------------------------------------- */
static int
add_module_constants(PyObject* m)
{
    if (PyModule_AddStringConstant(m, "version", IKAPI.info.version()) == -1) return -1;
    if (PyModule_AddIntConstant(m, "build_number", IKAPI.info.build_number()) == -1) return -1;
    if (PyModule_AddStringConstant(m, "host", IKAPI.info.host()) == -1) return -1;
    if (PyModule_AddStringConstant(m, "date", IKAPI.info.date()) == -1) return -1;
    if (PyModule_AddStringConstant(m, "commit", IKAPI.info.commit()) == -1) return -1;
    if (PyModule_AddStringConstant(m, "compiler", IKAPI.info.compiler()) == -1) return -1;
    if (PyModule_AddStringConstant(m, "cmake", IKAPI.info.cmake()) == -1) return -1;
    if (PyModule_AddStringConstant(m, "all", IKAPI.info.all()) == -1) return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */
PyObject*
ik_module_info_create(void)
{
    PyObject* m;

    m = PyModule_Create(&ik_module_info );
    if (m == NULL)
        goto module_alloc_failed;

    if (add_module_constants(m) != 0)
        goto init_module_failed;

    return m;

    init_module_failed  : Py_DECREF(m);
    module_alloc_failed : return NULL;
}
