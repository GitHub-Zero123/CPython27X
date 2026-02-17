#ifndef Py_PYTHONX_H
#define Py_PYTHONX_H

#include "Python.h"

typedef const char* (*PyParser_CustomHandler_Type)(const char*);
typedef void (*PyParser_CustomHandler_Free_Type)(const char*);

PyAPI_FUNC(void) PyParser_SetCustomHandler(PyParser_CustomHandler_Type handler,
    PyParser_CustomHandler_Free_Type free_handler);

#endif /* !Py_PYTHONX_H */