#pragma once
#include "Thread.h"
class CRecvFileDataThread :
    public CThread
{
public:
    CRecvFileDataThread();
    virtual ~CRecvFileDataThread();

    virtual bool OnThreadEventRun(LPVOID lpParam);
};

