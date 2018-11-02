#include "stdafx.h"
#include "CommunicationIOCP.h"
#include "MacroShare.h"
#include "StructShare.h"
#include "PacketHandle.h"
#include "TeleClientDlg.h"

CCommunicationIOCP::CCommunicationIOCP()
    : m_hIOCP(NULL)
    , m_phthdArray(NULL)
{
}


CCommunicationIOCP::~CCommunicationIOCP()
{
}

BOOL CCommunicationIOCP::Create(PIOCPTHREADADDTIONDATA pAddtionData /*= NULL*/,
                                DWORD dwThreadNum /*= 0*/)
{
    if (m_hIOCP == NULL)
    {
        // 创建IOCP
        m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                         NULL,
                                         0,
                                         dwThreadNum);

        m_dwMaxThreadNum = dwThreadNum;
        if (m_dwMaxThreadNum == 0)
        {
            // 获取系统CPU核心数
            SYSTEM_INFO stSi;
            ::GetSystemInfo(&stSi);
            
            // 计算线程数
            m_dwMaxThreadNum = stSi.dwNumberOfProcessors;
        }

        if (m_hIOCP != NULL)
        {
            // 如果创建IOCP成功，则开始创建线程
            m_stIOCPThreadParam.pIOCP_ = this;
            m_stIOCPThreadParam.pThreadAddtionData_ = pAddtionData;
            // *注意* 因为是立即执行，此时创建的线程会进入到IOCP的已释放线程列表
            m_phthdArray = new HANDLE[m_dwMaxThreadNum];
            for (size_t cntI = 0; cntI < m_dwMaxThreadNum; ++cntI)
            {
                m_phthdArray[cntI] = 
                    (HANDLE)::_beginthreadex(NULL,
                                             0,
                                             (_beginthreadex_proc_type)
                                             ThreadWork,
                                             &m_stIOCPThreadParam,
                                             0,
                                             NULL);
            }

            return TRUE;
        }
    }

    return FALSE;
}

BOOL CCommunicationIOCP::Associate(HANDLE hFileHandle,
                                   ULONG_PTR pulCompletionKey /*= 0*/)
{
    // *注意* 当该函数用于设备绑定时，第4个参数会被忽略
    HANDLE hRet = CreateIoCompletionPort(hFileHandle,
                                         m_hIOCP,
                                         pulCompletionKey,
                                         m_dwMaxThreadNum);
    return hRet == m_hIOCP;
}

BOOL CCommunicationIOCP::Destroy()
{

    return 0;
}

DWORD CCommunicationIOCP::ThreadWork(LPVOID lpParam)
{
    // 参数解析
    PIOCPTHREADPARAM pstIOCPThreadParam = (PIOCPTHREADPARAM)lpParam;
    CCommunicationIOCP *pIOCP = pstIOCPThreadParam->pIOCP_;
    CTeleClientDlg *pTeleClientDlg = 
        pstIOCPThreadParam->pThreadAddtionData_->pTeleClientDlg;

    // 线程循环获取完成包
    while (TRUE)
    {
        // 获取完成请求
        DWORD dwTransferNumBytes = 0;
        ULONG_PTR ulCompletionKey = 0;
        OVERLAPPED *pstOverlapped = NULL;

        // *注意* 此时该线程从已释放线程队列中进入到等待线程队列(后入先出)
        BOOL bRet = GetQueuedCompletionStatus(pIOCP->m_hIOCP,
                                              &dwTransferNumBytes,
                                              &ulCompletionKey,
                                              &pstOverlapped,
                                              IOCP_WAIT_TIME);
        // *注意* 此时该线程已从等待线程队列中进入到已释放线程队列
        DWORD dwError = GetLastError();
        if (!bRet)
        {
            CString csFailedInfo;
            if (pstOverlapped != NULL)
            {
                csFailedInfo.Format(_T("I/O请求处理失败: %u\r\n"),
                                    dwError);
            }
            else
            {
                if (dwError == WAIT_TIMEOUT)
                {
                    csFailedInfo = _T("等待超时\r\n");
                }
                else
                {
                    // 调用失败则直接退出
                    csFailedInfo.Format(_T("Get I/O请求函数调用失败: %u\r\n"),
                                        dwError);
                    OutputDebugString(csFailedInfo.GetString());
                    break;
                }
            }
            OutputDebugString(csFailedInfo.GetString());
            continue;
        }

        // 获取重叠结构数据
        POVERLAPPEDWITHDATA pstData = CONTAINING_RECORD(pstOverlapped,
                                                        OVERLAPPEDWITHDATA,
                                                        stOverlapped_);
        PCLIENTINFO pstClientInfo = (PCLIENTINFO)ulCompletionKey;
        switch (pstData->eIOCPType_)
        {
            case IOCP_RECV:
            {
                // 成功收到了数据包
                // 将数据包写到Client的接收缓冲区
                pstClientInfo->RecvBuffer_.Write((PBYTE)pstData->szPacket_,
                                               dwTransferNumBytes);

                // 循环接收 
                while (TRUE)
                {
                    // 判断是否已收到完整包头, 不完整则退出
                    if (pstClientInfo->RecvBuffer_.GetBufferLen() < 
                        PACKET_HEADER_SIZE)
                    {
                        break;
                    }
                    
                    // 解析包头
                    PPACKETFORMAT pstPacket = 
                        (PPACKETFORMAT)pstClientInfo->RecvBuffer_.GetBuffer();

                    // 判断包内容是否完整, 不完整则退出
                    DWORD dwCurrentPacketSize =
                        (pstClientInfo->RecvBuffer_.GetBufferLen() -
                         PACKET_HEADER_SIZE);
                    if (dwCurrentPacketSize < pstPacket->dwSize_)
                    {
                        break;
                    }
                    
                    // 开始处理数据包
                    pstClientInfo->CriticalSection_.Lock();
                    PACKETFORMAT stTmpHeader;
                    pstClientInfo->RecvBuffer_.Read((PBYTE)&stTmpHeader,
                                                    PACKET_HEADER_SIZE);

                    if (stTmpHeader.dwSize_ > 0 &&
                        stTmpHeader.dwSize_ < PACKET_CONTENT_MAXSIZE)
                    {
                        // 交换到临时处理Buffer中去，RecvBuffer清空
                        pstClientInfo->RecvBuffer_.Read(
                            (PBYTE)pstClientInfo->szRecvTmpBuffer_,
                            stTmpHeader.dwSize_);
                    }

                    OnHandlePacket(stTmpHeader.ePacketType_,
                                   pstClientInfo->sctClientSocket_,
                                   pstClientInfo->szRecvTmpBuffer_,
                                   stTmpHeader,
                                   pstClientInfo,
                                   *pIOCP);
                    pstClientInfo->CriticalSection_.Unlock();

                } //! while 循环接收 END

                // 再次投递一个Recv请求
                pIOCP->PostRecvRequst(pstClientInfo->sctClientSocket_);
                if (!bRet)
                {
                    OutputDebugString(_T("Recv请求投递失败\r\n"));
                }

                break;
            } //! case IOCP_RECV END
            case IOCP_SEND:
            {
                // 成功发送了数据包
                // 发送了多少数据，则从发送缓冲区中清除多少数据
                //pstClientInfo->CriticalSection_.Lock();
                //pstClientInfo->SendBuffer_.Delete(dwTransferNumBytes);
                //pstClientInfo->CriticalSection_.Unlock();

                // 发送缓冲区中仍有数据
                if (pstClientInfo->SendBuffer_.GetBufferLen() > 0)
                {
                    pIOCP->PostSendRequst(pstClientInfo->sctClientSocket_,
                                          pstClientInfo->SendBuffer_);
                }

                break;
            }
        } //! switch END

        // 释放重叠结构体
        if (pstData != NULL)
        {
            delete pstData;
            pstData = NULL;
        }
    } //! while 线程循环获取完成包 END

    return 0;
} //! CCommunicationIOCP::ThreadWork END

BOOL CCommunicationIOCP::PostSendRequst(const SOCKET sctTarget,
                                        CBuffer &SendBuffer)
{
    POVERLAPPEDWITHDATA pstOverlappedWithData = NULL;

    do
    {
        DWORD dwSendedBytes = 0;
        pstOverlappedWithData = new OVERLAPPEDWITHDATA();

        if (pstOverlappedWithData == NULL)
        {
            OutputDebugString(_T("PostRecvRequst申请内存失败\r\n"));
            break;
        }

        // buffer和长度赋值
        pstOverlappedWithData->eIOCPType_ = IOCP_SEND;
        pstOverlappedWithData->stBuffer_.buf =
            (char *)SendBuffer.GetBuffer();
        pstOverlappedWithData->stBuffer_.len =
            SendBuffer.GetBufferLen();

        int iRet = 
            WSASend(sctTarget,
                    &pstOverlappedWithData->stBuffer_,
                    1,
                    &dwSendedBytes,
                    0,
                    (WSAOVERLAPPED *)&pstOverlappedWithData->stOverlapped_,
                    NULL);
        if (iRet == SOCKET_ERROR &&
            WSAGetLastError() != ERROR_IO_PENDING)
        {
            OutputDebugString(_T("WSASend失败\r\n"));
            break;
        }

        // 正常结束
        return TRUE;
    } while (FALSE);
    
    // 异常回收资源
    if (pstOverlappedWithData == NULL)
    {
        delete pstOverlappedWithData;
        pstOverlappedWithData = NULL;
    } 

    // 出现异常
    return FALSE;
} //! CCommunicationIOCP::PostSendRequst END

BOOL CCommunicationIOCP::PostRecvRequst(const SOCKET sctTarget)
{
    POVERLAPPEDWITHDATA pstOverlappedWithData = NULL;

    do
    {
        DWORD dwRecvedBytes = 0;
        // *注意* 该空间在处理完收到的数据后尽快释放
        pstOverlappedWithData = new OVERLAPPEDWITHDATA();

        if (pstOverlappedWithData == NULL)
        {
            OutputDebugString(_T("PostRecvRequst申请内存失败\r\n"));
            break;
        }

        // buffer和长度赋值
        pstOverlappedWithData->eIOCPType_ = IOCP_RECV;
        pstOverlappedWithData->stBuffer_.buf =
            pstOverlappedWithData->szPacket_;
        pstOverlappedWithData->stBuffer_.len =
            PACKET_CONTENT_MAXSIZE;

        DWORD dwFlags = 0;
        int iRet = 
            WSARecv(sctTarget,
                    &pstOverlappedWithData->stBuffer_,
                    1,
                    &dwRecvedBytes,
                    &dwFlags,
                    (WSAOVERLAPPED *)&pstOverlappedWithData->stOverlapped_,
                    NULL);
        if (iRet == SOCKET_ERROR &&
            WSAGetLastError() != ERROR_IO_PENDING)
        {
            OutputDebugString(_T("WSARecv失败\r\n"));
            break;
        }

        // 结束
        return TRUE;
    } while (FALSE);
    
    // 异常回收资源
    if (pstOverlappedWithData == NULL)
    {
        delete pstOverlappedWithData;
        pstOverlappedWithData = NULL;
    }

    return FALSE;
} //! CCommunicationIOCP::PostRecvRequst END

BOOL SendDataUseIOCP(CLIENTINFO *&ref_pstClientInfo,
                     CCommunicationIOCP &ref_IOCP,
                     CString &ref_csData,
                     PACKETTYPE ePacketType)
{
    PPACKETFORMAT pstPacket = 
        (PPACKETFORMAT)ref_pstClientInfo->szSendTmpBuffer_;

    //***********************************************
    //* ALARM * It should complete thread synchronize
    //***********************************************
    ref_pstClientInfo->CriticalSection_.Lock();
    pstPacket->ePacketType_ = ePacketType;
    pstPacket->dwSize_ =
        (ref_csData.GetLength() + 1) * sizeof(TCHAR);

    // Copy
    memmove(pstPacket->szContent_,
            ref_csData.GetBuffer(),
            pstPacket->dwSize_);

    // Write data of need to send.
    ref_pstClientInfo->SendBuffer_.Write(
        (PBYTE)ref_pstClientInfo->szSendTmpBuffer_,
        PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // Make SendTmpBuffer be zero.
    memset(ref_pstClientInfo->szSendTmpBuffer_,
           0,
           PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // Post the send requst to iocp.
    BOOL bRet =
        ref_IOCP.PostSendRequst(ref_pstClientInfo->sctClientSocket_,
                                ref_pstClientInfo->SendBuffer_);

    ref_pstClientInfo->SendBuffer_.ClearBuffer();
    ref_pstClientInfo->CriticalSection_.Unlock();

    return bRet;
} //! SendDataUseIOCP END

BOOL SendDataUseIOCP(CLIENTINFO *&ref_pstClientInfo,
                     CCommunicationIOCP &ref_IOCP,
                     CString &ref_csData,
                     const DWORD &ref_dwSize,
                     CString &ref_csFileFullName,
                     const ULONGLONG ullFilePointPos)
{
    PPACKETFORMAT pstPacket = 
        (PPACKETFORMAT)ref_pstClientInfo->szSendTmpBuffer_;

    //***********************************************
    //* ALARM * It should complete thread synchronize
    //***********************************************
    ref_pstClientInfo->CriticalSection_.Lock();
    pstPacket->ePacketType_ = PT_FILE_DATA;
    pstPacket->dwSize_ = ref_dwSize;
    // Copy file name.
    memmove(pstPacket->szFileFullName_,
            ref_csFileFullName.GetBuffer(),
            MAX_PATH); 
    pstPacket->ullFilePointPos_ = ullFilePointPos;

    // Copy
    memmove(pstPacket->szContent_,
            ref_csData.GetBuffer(),
            pstPacket->dwSize_);

    // Write data of need to send.
    ref_pstClientInfo->SendBuffer_.Write(
        (PBYTE)ref_pstClientInfo->szSendTmpBuffer_,
        PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // Make SendTmpBuffer be zero.
    memset(ref_pstClientInfo->szSendTmpBuffer_,
           0,
           PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // Post the send requst to iocp.
    BOOL bRet =
        ref_IOCP.PostSendRequst(ref_pstClientInfo->sctClientSocket_,
                                ref_pstClientInfo->SendBuffer_);

    ref_pstClientInfo->SendBuffer_.ClearBuffer();
    ref_pstClientInfo->CriticalSection_.Unlock();

    return bRet;
} //! SendDataUseIOCP END