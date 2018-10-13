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
