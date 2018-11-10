#include "stdafx.h"
#include "AcceptThread.h"
#include "StructShare.h"
#include "CommunicationIOCP.h"
#include "ClientManager.h"
#include "HostListView.h"


CAcceptThread::CAcceptThread()
{
}


CAcceptThread::~CAcceptThread()
{
}

bool CAcceptThread::OnThreadEventRun(LPVOID lpParam)
{
    // Analysis parament.
    PACCEPTTHREADPARAM pAcceptThreadParam = (PACCEPTTHREADPARAM)lpParam;
    // IOCP对象
    CCommunicationIOCP *pIOCP = pAcceptThreadParam->pIOCP_;
    // 客户端管理者独享
    CClientManager *pClientManager = pAcceptThreadParam->pClientManager_;
    SOCKET sctAcceptSocket = pAcceptThreadParam->sctAcceptSocket_;
    CHostListView *pHostListView = pAcceptThreadParam->pHostListView_;

    // 循环等待连接
    while (TRUE)
    {
        sockaddr_in stClientAddrInfo = { 0 };
        stClientAddrInfo.sin_family = AF_INET;
        int iSize = sizeof(stClientAddrInfo);
        SOCKET sctClientSocket = SOCKET_ERROR;
        PCLIENTINFO pstClientInfo = NULL;
        BOOL bHasError = FALSE;

        // 等待连接并记录信息及绑定IOCP
        do
        {
            sctClientSocket = accept(sctAcceptSocket,
                (sockaddr *)&stClientAddrInfo,
                                     &iSize);
            if (sctClientSocket == SOCKET_ERROR)
            {
                // 线程结束点
                OutputDebugString(_T("Accept线程退出\r\n"));
                return true;
            }

            // 记录客户端信息
            // *注意* 该位置为CLIENTINFO对象申请的内存会在ClientManager结束
            // 或连接断开时进行释放
            pstClientInfo = new CLIENTINFO;
            if (pstClientInfo == NULL)
            {
#ifdef DEBUG
                OutputDebugString(_T("客户端信息申请内存失败\r\n"));
#endif // DEBUG
                bHasError = TRUE;
                break;
            }

            pstClientInfo->tLastTime_ = time(NULL);
            pstClientInfo->stClientAddrInfo_ = stClientAddrInfo;
            pstClientInfo->sctClientSocket_ = sctClientSocket;
            pstClientInfo->pFileTransferDlg_ = NULL;
            pstClientInfo->pCmdDlg_ = NULL;

            CString csIPAndPort;
            // Change address and Port to wide character.
            USES_CONVERSION;
            csIPAndPort.Format(_T("%s:%d"),
                               A2W(inet_ntoa(stClientAddrInfo.sin_addr)),
                               ntohs(stClientAddrInfo.sin_port));

            // 将信息添加到客户端管理中
            pClientManager->InsertClient(sctClientSocket, pstClientInfo);
            pClientManager->InsertSocket(csIPAndPort, sctClientSocket);

            // 界面显示新客户端的连接信息
            pHostListView->SendMessage(WM_HASINFOTOFLUSH,
                                       (WPARAM)IFT_ACCEPTCLIENT,
                                       (LPARAM)pstClientInfo);

            // 将客户端socket和IOCP绑定
            BOOL bRet =
                pIOCP->Associate((HANDLE)sctClientSocket,
                                 (ULONG_PTR)pstClientInfo);
            if (!bRet)
            {
                OutputDebugString(_T("客户端Socket和IOCP绑定失败\r\n"));
                bHasError = TRUE;
                break;
            }

            // 投递接收消息的申请
            pIOCP->PostRecvRequst(sctClientSocket);
        } while (FALSE); 

        // Continue when no error.
        if (!bHasError)
        {
            continue;
        }

        // Shutdown the socket which has connected if has error. 
        if (sctClientSocket != SOCKET_ERROR)
        {
            shutdown(sctClientSocket, SD_SEND);
            closesocket(sctClientSocket);
            sctClientSocket = SOCKET_ERROR;
        }

        // Free the heap memory.
        if (pstClientInfo != NULL)
        {
            delete pstClientInfo;
            pstClientInfo = NULL;
        }
    } // while 循环等待连接 END

    return false;
} //! CAcceptThread::OnThreadEventRun END
