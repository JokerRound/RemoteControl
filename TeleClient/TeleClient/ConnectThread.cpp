#include "stdafx.h"
#include "ConnectThread.h"
#include "StructShare.h"
#include "CommunicationIOCP.h"
#include "TeleClientDlg.h"


CConnectThread::CConnectThread()
{
}


CConnectThread::~CConnectThread()
{
}

bool CConnectThread::OnThreadEventRun(LPVOID lpParam)
{
    // Analysis parament.
    PCONNECTTHREADPARAM pConnectThreadParam = (PCONNECTTHREADPARAM)lpParam;
    // Get IOCP object.
    CCommunicationIOCP *pIOCP = pConnectThreadParam->pIOCP_;
    // Get server address info.
    sockaddr_in stServerAddrInfo = pConnectThreadParam->stServerAddrInfo_;
    // Get break event handle.
    HANDLE *phBreakEvent = pConnectThreadParam->phBreakEvent_;
    // Get window point of client.
    CTeleClientDlg *pTeleClientDlg = pConnectThreadParam->pTeleClientDlg_;

    int iRet = 0;
    SOCKET sctConnectSocket = INVALID_SOCKET;
    BOOL bHasError = FALSE;

    // Connect loop.
    while (TRUE)
    {
        do
        {
            sctConnectSocket = socket(AF_INET,
                                      SOCK_STREAM,
                                      IPPROTO_TCP);

            if (sctConnectSocket == INVALID_SOCKET)
            {
                OutputDebugString(_T("创建Socket失败\r\n"));
                bHasError = TRUE;
                break;
            }

            // Dialog update new socket.
            pTeleClientDlg->SetConnectSocket(sctConnectSocket);

            iRet = connect(sctConnectSocket,
                (sockaddr *)&stServerAddrInfo,
                           sizeof(stServerAddrInfo));

            if (iRet == SOCKET_ERROR)
            {
                WaitForSingleObject(*phBreakEvent, CONNECT_WAIT_TIME);
                bHasError = TRUE;
                break;
            }

            // 构建连接信息
            PCLIENTINFO pstClientInfo = new CLIENTINFO;
            pstClientInfo->sctClientSocket_ = sctConnectSocket;
            pstClientInfo->tLastTime_ = time(NULL);
            pstClientInfo->hCMD_ = INVALID_HANDLE_VALUE;
            pstClientInfo->hCmdReadPipe_ = INVALID_HANDLE_VALUE;
            pstClientInfo->hCmdWritePipe_ = INVALID_HANDLE_VALUE;
            pstClientInfo->pTeleClientDlg_ = pTeleClientDlg;

            // 窗口类保存Socket上下文信息
            pTeleClientDlg->m_pstClientInfo = pstClientInfo;
            // Bind socket with IOCP.
            BOOL bRet = 
                pIOCP->Associate((HANDLE)sctConnectSocket,
                                 (ULONG_PTR)pstClientInfo);
            if (!bRet)
            {
                OutputDebugString(_T("绑定IOCP失败\r\n"));
                bHasError = TRUE;
                break;
            }

            // 投递一个接收请求
            pIOCP->PostRecvRequst(pstClientInfo->sctClientSocket_);

            // Begin the heartbeat Timer.
            bRet = pTeleClientDlg->SetTimer(TIMER_HEATBEAT,
                                            HEATBEAT_ELAPSE,
                                            NULL);

            // 等待连接中断事件触发
            DWORD dwRet = WaitForSingleObject(*phBreakEvent, INFINITE);
            pTeleClientDlg->m_bBreakEventHasSigal = FALSE;
            if (dwRet == WAIT_FAILED)
            {
#ifdef DEBUG
                OutputDebugString(_T("中断事件等待异常\r\n"));
#endif // DEBUG
            }
            else if (dwRet == WAIT_OBJECT_0)
            {
#ifdef DEBUG
                OutputDebugString(_T("正常触发中断事件\r\n"));
#endif // DEBUG
            }
        } while (FALSE);

        // Clean.
        shutdown(sctConnectSocket, SD_SEND);
        closesocket(sctConnectSocket);

        // Close Timer.
        pTeleClientDlg->KillTimer(TIMER_HEATBEAT);

        // Free context info of client.
        if (pTeleClientDlg->m_pstClientInfo != NULL)
        {
            delete pTeleClientDlg->m_pstClientInfo;
            pTeleClientDlg->m_pstClientInfo = NULL;
        }
    } //! while "Loop connect" END

    return true;
} //! CConnectThread::OnThreadEventRun END
