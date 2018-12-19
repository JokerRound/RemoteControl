#pragma once
#ifndef COMMUNICATIONIOCP_H_
#define COMMUNICATIONIOCP_H_
#include "stdafx.h"
#include "StructShare.h"
#include "CBuffer.h"

class CCommunicationIOCP
{
private:
    HANDLE m_hIOCP;
    HANDLE *m_phthdArray;
    // 线程池最大线程数量
    DWORD m_dwMaxThreadNum;
    // 当前线程池中的数量
    DWORD m_dwCurrentThreadNum;
    // 当前工作中的线程数量
    DWORD m_dwBusyThreadNum;
    // 临界区
    CCriticalSection m_CriticalSection;
    // 线程参数
    IOCPTHREADPARAM m_stIOCPThreadParam = { 0 };

public:
    CCommunicationIOCP();
    ~CCommunicationIOCP();

    // Create IOCP's work thread.
    BOOL Create(_In_ PIOCPTHREADADDTIONDATA pAddtionData = NULL,
                _In_ DWORD dwThreadNum = 0);
    // Bind.
    BOOL Associate(_In_ HANDLE hFileHandle,
                   _In_ ULONG_PTR pulCompletionKey = 0);
    // Free resource.
    BOOL Destroy();

    // 线程工作回调
    static DWORD ThreadWork(LPVOID lpParam);


    // 投递接收请求
    BOOL PostSendRequst(const SOCKET sctTarget, CBuffer &SendBuffer);
    // 投递发送请求
    BOOL PostRecvRequst(const SOCKET sctTarget);


};

BOOL SendDataUseIOCP(_In_ CLIENTINFO *&ref_pstClientInfo,
                     _In_ CCommunicationIOCP &ref_IOCP,
                     _In_ const CString &ref_csData,
                     _In_ PACKETTYPE ePacketType);



#endif // !COMMUNICATIONIOCP_H_
