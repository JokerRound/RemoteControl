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
    // 解析参数
    PCONNECTTHREADPARAM pConnectThreadParam = (PCONNECTTHREADPARAM)lpParam;
    // 获取IOCP类对象
    CCommunicationIOCP *pIOCP = pConnectThreadParam->pIOCP_;
    // 获取服务器地址信息
    sockaddr_in stServerAddrInfo = pConnectThreadParam->stServerAddrInfo_;
    // 获取中断事件句柄
    HANDLE *phBreakEvent = pConnectThreadParam->phBreakEvent_;
    // 获取客户端窗口的指针
    CTeleClientDlg *pTeleClientDlg = pConnectThreadParam->pTeleClientDlg_;

    int iRet = 0;
    SOCKET sctConnectSocket = INVALID_SOCKET;
    BOOL bHasError = FALSE;

    // 循环尝试连接
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

            // 对话框更新新的Sokcet
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
            // 绑定socket和IOCP
            BOOL bRet = pIOCP->Associate((HANDLE)sctConnectSocket,
                (ULONG_PTR)pstClientInfo);
            if (!bRet)
            {
                OutputDebugString(_T("绑定IOCP失败\r\n"));
                bHasError = TRUE;
                break;
            }

            // 投递一个接收请求
            pIOCP->PostRecvRequst(pstClientInfo->sctClientSocket_);

            // 设置计时器
            bRet = pTeleClientDlg->SetTimer(TIMER_HEATBEAT,
                                            HEATBEAT_ELAPSE,
                                            NULL);

            // 等待连接中断事件触发
            DWORD dwRet = WaitForSingleObject(*phBreakEvent, INFINITE);
            pTeleClientDlg->m_bBreakEventHasSigal = FALSE;
            if (dwRet == WAIT_FAILED)
            {
                // 事件因为某些原因被关闭，程序退出
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

        // 清理
        shutdown(sctConnectSocket, SD_SEND);
        closesocket(sctConnectSocket);

        // 关闭定时器
        pTeleClientDlg->KillTimer(TIMER_HEATBEAT);

        // 释放Socket上下文信息
        if (pTeleClientDlg->m_pstClientInfo != NULL)
        {
            delete pTeleClientDlg->m_pstClientInfo;
            pTeleClientDlg->m_pstClientInfo = NULL;
        }
    } //! while 循环尝试连接 END

    return true;
} //! CConnectThread::OnThreadEventRun END
