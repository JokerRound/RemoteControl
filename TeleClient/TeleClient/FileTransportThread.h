#pragma once
#ifndef FILETRANSPORTTHREAD_H_
#define FILETRANSPORTTHREAD_H_
#include "Thread.h"




class CFileTransportThread :
    public CThread
{
public:
    CFileTransportThread();
    virtual ~CFileTransportThread();

    virtual bool OnThreadEventRun(LPVOID lpParam);
};


#endif // !1FILETRANSPORTTHREAD_H_
