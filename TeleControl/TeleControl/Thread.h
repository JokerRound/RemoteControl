#pragma once
#ifndef THREAD_H_
#define THREAD_H_
#include "StructShare.h"

struct tagThreadProcParameter
{
    LPVOID pClass_;
    LPVOID pParameter_;
    BOOL bStartSuccess_;
};

class CThread
{
public:
    // 线程句柄
    HANDLE m_hThread = INVALID_HANDLE_VALUE;
    // 线程ID
    unsigned int m_dwThreadId = 0;
protected:
    
    // 线程回调函数参数
    tagThreadProcParameter m_stThreadProcParameter = { 0 };
    
    // 成功初始化事件
    CEvent *m_pevtThreadStartSuccess = NULL;

public:
    CThread();
    virtual ~CThread();
    
    virtual BOOL StartThread(LPVOID pParameter = NULL,
                             BOOL bInheritHandle = TRUE,
                             BOOL bIsSuspended = FALSE);


    //结束线程
    virtual BOOL ConcludeThread(DWORD dwWaitTime = INFINITE);

    //线程执行主体
    virtual bool OnThreadEventStart()
    {
        return true;
    };

    virtual bool OnThreadEventRun(LPVOID lpParam)
    {
        return true;
    };

    virtual bool OnThreadEventEnd()
    {
        return true;
    }

    static DWORD ThreadFunc(LPVOID lpParam);
};

#endif // !THREAD_H_
