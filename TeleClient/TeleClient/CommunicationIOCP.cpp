//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-11-08
// Description: 
//      achieve of method in CCommunicationIOCP and related function.
//
// Modify Log:
//      2018-10-24    Hoffman
//      Info: a. Add achieve of below member method.
//              a.1 SendDataUseIOCP();
//
//      2018-11-22    Hoffman
//      Info: a. Midify achieve of below methods.
//              a.1 Create(): 
//                  a.1.1. Close thread handle directly.
//                  a.1.2. Change execution structure.
//                  a.1.3. Add some error check.
//              a.2 ThreadWork():
//                  a.2.1. Add some error check.
//******************************************************************************

#include "stdafx.h"
#include "CommunicationIOCP.h"
#include "MacroShare.h"
#include "StructShare.h"
#include "PacketHandle.h"
#include "TeleClientDlg.h"

CCommunicationIOCP::CCommunicationIOCP()
{
}


CCommunicationIOCP::~CCommunicationIOCP()
{
}

BOOL CCommunicationIOCP::Create(PIOCPTHREADADDTIONDATA pAddtionData /*= NULL*/,
                                DWORD dwThreadNum /*= 0*/)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG

    do
    {
        if (NULL != m_hIOCP)
        {
            break;
        }

        // Create IOCP.
        m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                         NULL,
                                         0,
                                         dwThreadNum);
        if (NULL == m_hIOCP)
        {
#ifdef DEBUG
            bOutputErrMsg = TRUE;
            dwLine = __LINE__;
#endif // DEBUG
            // Jump out if has error.
            break;
        }

        // Create threads if create iocp successfully.
        m_stIOCPThreadParam.pIOCP_ = this;
        m_stIOCPThreadParam.pThreadAddtionData_ = pAddtionData;

        m_dwMaxThreadNum = dwThreadNum;
        if (m_dwMaxThreadNum == 0)
        {
            // Get cpu cores.
            SYSTEM_INFO stSi;
            ::GetSystemInfo(&stSi);

            // Set number of threads.
            m_dwMaxThreadNum = stSi.dwNumberOfProcessors;
        }

        // This threads will be added into 
        // "Had freed threads queue" of iocp.
        //m_pahThreadArray = new HANDLE[m_dwMaxThreadNum];

        if (m_dwMaxThreadNum > 0)
        {
            size_t cntI = 0;
            do
            {
                //m_pahThreadArray[cntI] = 
                HANDLE hTmpThreadHandle =
                    (HANDLE)::_beginthreadex(NULL,
                                             0,
                                             (_beginthreadex_proc_type)
                                             ThreadWork,
                                             &m_stIOCPThreadParam,
                                             0,
                                             NULL);
                if (hTmpThreadHandle)
                {
                    CloseHandle(hTmpThreadHandle);
                }
#ifdef DEBUG
                else
                {

                }
#endif // DEBUG
            } while (++cntI < m_dwBusyThreadNum);
        }

        return TRUE;
    } while (FALSE);
    
#ifdef DEBUG
    if (bOutputErrMsg && 0 != dwLine)
    {
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);
    }
#endif // DEBUG 

    return FALSE;
} //! CCommunicationIOCP::Create END

BOOL CCommunicationIOCP::Associate(HANDLE hFileHandle,
                                   ULONG_PTR pulCompletionKey /*= 0*/)
{
    // The fourth parament will be ignored.
    HANDLE hRet = CreateIoCompletionPort(hFileHandle,
                                         m_hIOCP,
                                         pulCompletionKey,
                                         m_dwMaxThreadNum);
    return hRet == m_hIOCP;
} //! CCommunicationIOCP::Associate END

BOOL CCommunicationIOCP::Destroy()
{
    if (NULL != m_hIOCP)
    {
        CloseHandle(m_hIOCP);
        m_hIOCP = NULL;
    }
    return 0;
}

DWORD CCommunicationIOCP::ThreadWork(LPVOID lpParam)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG
    // Parament analysis.
    PIOCPTHREADPARAM pstIOCPThreadParam = (PIOCPTHREADPARAM)lpParam;
    CCommunicationIOCP *pIOCP = pstIOCPThreadParam->pIOCP_;
    CTeleClientDlg *pTeleClientDlg = 
        pstIOCPThreadParam->pThreadAddtionData_->pTeleClientDlg;

    // Get compelet package loop.
    while (TRUE)
    {
        DWORD dwTransferNumBytes = 0;
        ULONG_PTR ulCompletionKey = 0;
        OVERLAPPED *pstOverlapped = NULL;

        //*********************************************************************
        //* Alarm * Thread go to "Wait Thread Queue" from "Freed Thread Queue".
        //*********************************************************************
        BOOL bRet = GetQueuedCompletionStatus(pIOCP->m_hIOCP,
                                              &dwTransferNumBytes,
                                              &ulCompletionKey,
                                              &pstOverlapped,
                                              IOCP_WAIT_TIME);

        DWORD dwErrorNumber = GetLastError();
        //*********************************************************************
        //* Alarm * Thread go to "Freed Thread Queue" from "Wait Thread Queue".
        //*********************************************************************
        if (!bRet)
        {
            if (pstOverlapped != NULL)
            {
#ifdef DEBUG
                OutputDebugStringWithInfo(_T("Deal with I/O request"
                                             " failed.\r\n"),
                                          __FILET__,
                                          __LINE__);
#endif // DEBUG
            }
            else
            {
                if (dwErrorNumber == WAIT_TIMEOUT)
                {
#ifdef DEBUG
                    OutputDebugStringWithInfo(_T("Wait overtime."),
                                              __FILET__,
                                              __LINE__);
#endif // DEBUG
                }
                else
                {
#ifdef DEBUG
                    dwError = GetLastError();
                    GetErrorMessage(dwError, csErrorMessage);
                    OutputDebugStringWithInfo(csErrorMessage,
                                              __FILET__,
                                              __LINE__);
#endif // DEBUG 
                    // Finish loop and threads exits.
                    break;
                }
            }

#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG 
            continue;
        }

        // Get overlap struct.
        POVERLAPPEDWITHDATA pstData = CONTAINING_RECORD(pstOverlapped,
                                                        OVERLAPPEDWITHDATA,
                                                        stOverlapped_);

        PCLIENTINFO pstClientInfo = (PCLIENTINFO)ulCompletionKey;
        switch (pstData->eIOCPType_)
        {
            case IOCP_RECV:
            {
                // Write package data to client receive buffer.
                pstClientInfo->RecvBuffer_.Write((PBYTE)pstData->szPacket_,
                                                 dwTransferNumBytes);

                // Receive loop.
                while (TRUE)
                {
                    // Quit this change if the package's header is incomplete.
                    if (pstClientInfo->RecvBuffer_.GetBufferLen() < 
                        PACKET_HEADER_SIZE)
                    {
                        break;
                    }
                    
                    PPACKETFORMAT pstPacket = 
                        (PPACKETFORMAT)pstClientInfo->RecvBuffer_.GetBuffer();

                    // Quit this chance if the package's content is incomplete.
                    DWORD dwCurrentPacketSize =
                        (pstClientInfo->RecvBuffer_.GetBufferLen() -
                         PACKET_HEADER_SIZE);
                    if (dwCurrentPacketSize < pstPacket->dwSize_)
                    {
                        break;
                    }
                    
                    // Begin to deal with the package.
                    pstClientInfo->CriticalSection_.Lock();
                    PACKETFORMAT stTmpHeader;
                    memset(&stTmpHeader, 0, sizeof(stTmpHeader));

                    pstClientInfo->RecvBuffer_.Read((PBYTE)&stTmpHeader,
                                                    PACKET_HEADER_SIZE);

                    if (stTmpHeader.dwSize_ > 0 &&
                        stTmpHeader.dwSize_ < PACKET_CONTENT_MAXSIZE)
                    {
                        // Swap data to temporary buffer.
                        // RecvBuffer had been cleaned.
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

                } //! while "Receive loop" END

                // Post new recv request.
                bRet = pIOCP->PostRecvRequst(pstClientInfo->sctClientSocket_);
                if (!bRet)
                {

                }

                break;
            } //! case IOCP_RECV END
            case IOCP_SEND:
            {

                if (pstClientInfo->SendBuffer_.GetBufferLen() > 0)
                {
                    pIOCP->PostSendRequst(pstClientInfo->sctClientSocket_,
                                          pstClientInfo->SendBuffer_);
                }

                break;
            }
        } //! switch END

        // Free overlap struct.
        if (pstData != NULL)
        {
            delete pstData;
            pstData = NULL;
        }
    } //! while "Get compelet package loop" END

    return 0;
} //! CCommunicationIOCP::ThreadWork END

BOOL CCommunicationIOCP::PostSendRequst(const SOCKET sctTarget,
                                        CBuffer &SendBuffer)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG
    POVERLAPPEDWITHDATA pstOverlappedWithData = NULL;

    do
    {
        DWORD dwSendedBytes = 0;
        //******************************************************************
        //*ALARM* This memory will free when IOCP deal with it finished.
        //******************************************************************
        pstOverlappedWithData = new OVERLAPPEDWITHDATA();
        if (NULL == pstOverlappedWithData)
        {
#ifdef DEBUG
            bOutputErrMsg = TRUE;
            dwLine = __LINE__;
#endif // DEBU
            break;
        }

        // Set buffer and length.
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
        if (SOCKET_ERROR == iRet  &&
            WSAGetLastError() != ERROR_IO_PENDING)
        {
#ifdef DEBUG
            bOutputErrMsg = TRUE;
            dwLine = __LINE__;
#endif // DEBU
            break;
        }

        return TRUE;
    } while (FALSE);
    
#ifdef DEBUG
    if (bOutputErrMsg && 0 != dwLine)
    {
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);
    }
#endif // DEBUG

    if (NULL != pstOverlappedWithData)
    {
        delete pstOverlappedWithData;
        pstOverlappedWithData = NULL;
    }

    return FALSE;
} //! CCommunicationIOCP::PostSendRequst END

BOOL CCommunicationIOCP::PostRecvRequst(const SOCKET sctTarget)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG
    POVERLAPPEDWITHDATA pstOverlappedWithData = NULL;

    do
    {
        DWORD dwRecvedBytes = 0;
        //******************************************************************
        //*ALARM* This memory will free when IOCP deal with it finished.
        //******************************************************************
        pstOverlappedWithData = new OVERLAPPEDWITHDATA();
        if (NULL == pstOverlappedWithData)
        {
#ifdef DEBUG
            bOutputErrMsg = TRUE;
            dwLine = __LINE__;
#endif // DEBUG
            break;
        }

        pstOverlappedWithData->eIOCPType_ = IOCP_RECV;
        pstOverlappedWithData->stBuffer_.buf = pstOverlappedWithData->szPacket_;
        pstOverlappedWithData->stBuffer_.len = PACKET_CONTENT_MAXSIZE;

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
#ifdef DEBUG
            bOutputErrMsg = TRUE;
            dwLine = __LINE__;
#endif // DEBUG
            break;
        }

        // Successfully.
        return TRUE;
    } while (FALSE);
    
#ifdef DEBUG
    if (bOutputErrMsg && 0 != dwLine)
    {
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);
    }
#endif // DEBUG

    if (NULL != pstOverlappedWithData)
    {
        delete pstOverlappedWithData;
        pstOverlappedWithData = NULL;
    }

    return FALSE;
} //! CCommunicationIOCP::PostRecvRequst END

BOOL SendDataUseIOCP(CLIENTINFO *&ref_pstClientInfo,
                     CCommunicationIOCP &ref_IOCP,
                     const CString &ref_csData,
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
            ref_csData.GetString(),
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
                     const CString &ref_csData,
                     const DWORD &ref_dwSize,
                     CString &ref_csFileFullName,
                     const ULONGLONG &ref_ullFilePointPos,
                     const ULONG &ref_ulTaskId)
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
    pstPacket->ullFilePointPos_ = ref_ullFilePointPos;
    pstPacket->ulTaskId_ = ref_ulTaskId;

    // Copy
    memmove(pstPacket->szContent_,
            ref_csData.GetString(),
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