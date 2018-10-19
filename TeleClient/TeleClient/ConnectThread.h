#pragma once
#ifndef CONNECTTHREAD_H_
#define CONNECTTHREAD_H_
#include "Thread.h"

class CConnectThread :
    public CThread
{
public:
    CConnectThread();
    virtual ~CConnectThread();

    virtual bool OnThreadEventRun(LPVOID lpParam);
};

#endif // !1CONNECTTHREAD_H_
