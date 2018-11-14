//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      The define for class CAcceptThread.
//
// Modify Log:
//      2018-07-24    Hoffman
//      Info: Add below member method.
//              OnThreadEventRun();
//******************************************************************************

#pragma once
#ifndef ACCEPTTHREAD_H_
#define ACCEPTTHREAD_H_
#include "Thread.h"

class CAcceptThread :
    public CThread
{
public:
    CAcceptThread();
    virtual ~CAcceptThread();

    virtual bool OnThreadEventRun(LPVOID lpParam);
};


#endif // !ACCEPTTHREAD_H_
