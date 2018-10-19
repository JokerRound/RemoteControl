#include "stdafx.h"
#include "Thread.h"



CThread::CThread()
{
    m_pevtThreadStartSuccess = new CEvent(TRUE, FALSE);
}

CThread::~CThread()
{
    ConcludeThread();
    if (m_pevtThreadStartSuccess != NULL)
    {
        delete m_pevtThreadStartSuccess;
        m_pevtThreadStartSuccess = NULL;
    }
}

BOOL CThread::StartThread(LPVOID pParameter, 
                          BOOL bInheritHandle, 
                          BOOL bIsSuspended)
{
    m_stThreadProcParameter.pClass_ = this;
    m_stThreadProcParameter.pParameter_ = pParameter;
    m_stThreadProcParameter.bStartSuccess_ = TRUE;

    SECURITY_ATTRIBUTES stSa = { 0 };
    stSa.nLength = sizeof(stSa);
    stSa.bInheritHandle = bInheritHandle;

    m_hThread = (HANDLE)_beginthreadex(&stSa,
                                       0,
                                       (_beginthreadex_proc_type)ThreadFunc,
                                       &m_stThreadProcParameter,
                                       bIsSuspended,
                                       &m_dwThreadId);

    if (m_hThread == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // 等待线程初始化完成的事件
    m_pevtThreadStartSuccess->Lock();
    if (!m_stThreadProcParameter.bStartSuccess_)
    {
        ConcludeThread();
        return FALSE;
    }

    return TRUE;
}

BOOL CThread::ConcludeThread(DWORD dwWaitTime)
{
    //释放资源
    if (m_hThread != INVALID_HANDLE_VALUE)
    {
        WaitForSingleObject(m_hThread, dwWaitTime);

        CloseHandle(m_hThread);
        m_hThread = INVALID_HANDLE_VALUE;
    }

    return 0;
}

DWORD CThread::ThreadFunc(LPVOID lpParameter)
{
    CThread *pThis =
        (CThread *)((tagThreadProcParameter *)lpParameter)->pClass_;
    LPVOID pParameter =
        (((tagThreadProcParameter *)lpParameter)->pParameter_);
    BOOL &bStartSuccess = 
        (((tagThreadProcParameter *)lpParameter)->bStartSuccess_);

    ASSERT(pThis != NULL);
    if (pThis == NULL)
    {
        return 0;
    }

    bStartSuccess = pThis->OnThreadEventStart();
    // 开启线程初始化完成事件
    pThis->m_pevtThreadStartSuccess->SetEvent();

    if (!bStartSuccess)
    {
        return 0;
    }

    try
    {
        pThis->OnThreadEventRun(pParameter);
    }
    catch (...)
    {
        ASSERT(FALSE);
    }

    try
    {
        pThis->OnThreadEventEnd();
    }
    catch (...)
    {
        ASSERT(FALSE);
    }
       
    return 0;
}