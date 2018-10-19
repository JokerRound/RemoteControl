#include "stdafx.h"
#include "StructShare.h"
#include "FileTransferDlg.h"
#include "ClientManager.h"
#include "FileTransferStruct.h"
#include "PacketHandle.h"
#include "CmdDlg.h"
#include "CommunicationIOCP.h"


BOOL OnHeartBeat(SOCKET sctTargetSocket,
                 PCLIENTINFO pstClientInfo,
                 CCommunicationIOCP &IOCP)
{
    PPACKETFORMAT pstPacket = (PPACKETFORMAT)pstClientInfo->szSendTmpBuffer_;

    // *注意* 写入数据时要加锁
    pstClientInfo->CriticalSection_.Lock();
    pstPacket->ePacketType_ = PT_HEATBEAT;
    pstPacket->dwSize_ = 0;

    pstClientInfo->SendBuffer_.Write((PBYTE)pstClientInfo->szSendTmpBuffer_,
                                     PACKET_HEADER_SIZE + pstPacket->dwSize_);
    // 清空临时发送区
    memset(pstClientInfo->szSendTmpBuffer_,
           0,
           PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // 投递发送请求
    BOOL bRet = IOCP.PostSendRequst(sctTargetSocket,
                                    pstClientInfo->SendBuffer_);

    pstClientInfo->SendBuffer_.ClearBuffer();
    pstClientInfo->CriticalSection_.Unlock();

    return bRet;
} 

BOOL OnFileList(SOCKET sctTargetSocket,
                char *szBuffer,
                size_t uiLen,
                PCLIENTINFO pstClientInfo,
                CCommunicationIOCP &IOCP)
{
    pstClientInfo->pFileTransferDlg_->m_csFileList.Empty();

    memmove(pstClientInfo->pFileTransferDlg_->
            m_csFileList.GetBufferSetLength(PACKET_CONTENT_MAXSIZE),
            szBuffer,
            uiLen);
    pstClientInfo->pFileTransferDlg_->m_csFileList.ReleaseBuffer();
    // 清空接收临时缓冲区
    memset(szBuffer, 0, uiLen);

    SetEvent(pstClientInfo->pFileTransferDlg_->m_hGetTargetFileListEvent);

    return TRUE;
} //! OnFileList END

BOOL OnFileDevice(SOCKET sctTargetSocket,
                  char *szBuffer,
                  size_t uiLen,
                  PCLIENTINFO pstClientInfo,
                  CCommunicationIOCP &IOCP)
{
    pstClientInfo->pFileTransferDlg_->m_csTargetHostDevice.Empty();
    // 将数据传给文件传输对话框
    pstClientInfo->pFileTransferDlg_->m_uiTargetHostDeviceLen = uiLen;
    memmove(pstClientInfo->pFileTransferDlg_->
            m_csTargetHostDevice.GetBufferSetLength(PACKET_CONTENT_MAXSIZE),
            szBuffer,
            uiLen);

    pstClientInfo->pFileTransferDlg_->
        m_csTargetHostDevice.ReleaseBuffer();

    // 清空接收临时缓冲区
    memset(szBuffer, 0, uiLen);

    // 触发成功获取盘符事件
    SetEvent(pstClientInfo->pFileTransferDlg_->m_hGetTargetDeviceEvent);
    
    return TRUE;
}

BOOL OnFileData(SOCKET sctTargetSocket,
                char *szBuffer,
                size_t uiLen,
                PCLIENTINFO pstClientInfo,
                CCommunicationIOCP &IOCP)
{
    CString csFileData;
    szBuffer[uiLen] = _T('\0');
    csFileData = szBuffer;
    memset(szBuffer, 0, uiLen);

    BOOL bRet = TRUE;
    //BOOL bRet = pstClientInfo->pFileTransferDlg_->SendMessage();

    return bRet;
} //! OnFileData END

// Deal with the reply from target host.
BOOL OnCMDReply(SOCKET sctTargetSocket,
                char *szBuffer,
                size_t uiLen,
                PCLIENTINFO pstClintInfo)
{
    CString csCmdReply;
    szBuffer[uiLen] = _T('\0');
    csCmdReply = szBuffer;
    memset(szBuffer, 0, uiLen);

    BOOL bRet = 
        pstClintInfo->pCmdDlg_->SendMessage(WM_HASCMDREPLY,
                                            (WPARAM)&csCmdReply,
                                            0);

    return bRet;
}

// Deal with the info from target host.
BOOL OnProcessInfo(SOCKET sctTargetSocket,
                   char *pszBuffer,
                   size_t uiLen,
                   PCLIENTINFO pstClintInfo)
{
    CString csProcessInfoList;
    char *pszTmpBuffer = new char[uiLen + 1];
    memset(pszTmpBuffer, 0, uiLen + 1);
    memmove(pszTmpBuffer, pszBuffer, uiLen + 1);

    csProcessInfoList = pszTmpBuffer;

    if (pszTmpBuffer != NULL)
    {
        delete[] pszTmpBuffer;
        pszTmpBuffer = NULL;
    }

    // 解析客户端发送过来的进程信息列表
    while (!csProcessInfoList.IsEmpty())
    {
        CString csProcessInfo;
        int iProcessInfoPos = csProcessInfoList.Find(_T("|"));
        csProcessInfo = csProcessInfoList.Left(iProcessInfoPos);
        csProcessInfoList =
            csProcessInfoList.Right(
                csProcessInfoList.GetLength() - iProcessInfoPos - 1);

        // EXE文件名
        int iExeFilePos = csProcessInfo.Find(_T("?"));
        CString csExeFile = csProcessInfo.Left(iExeFilePos);
        csProcessInfo =
            csProcessInfo.Right(
                csProcessInfo.GetLength() - iExeFilePos - 1);

        // 进程ID
        int iProcessIdPos = csProcessInfo.Find(_T("?"));
        CString csProcessId = csProcessInfo.Left(iProcessIdPos);
        csProcessInfo =
            csProcessInfo.Right(
                csProcessInfo.GetLength() - iExeFilePos - 1);

        // 父进程ID
        int iParentIdPos = csProcessInfo.Find(_T("?"));
        CString csParentid = csProcessInfo.Left(iParentIdPos);
        csProcessInfo =
            csProcessInfo.Right(
                csProcessInfo.GetLength() - iParentIdPos - 1);

        // 获取全路径
        iParentIdPos = csProcessInfo.Find(_T("?"));
        CString csFullPath = csProcessInfo.Left(iParentIdPos);
        csProcessInfo =
            csProcessInfo.Right(
                csProcessInfo.GetLength() - iParentIdPos - 1);
    } //! while 解析进程列表 END

    return TRUE;
}

BOOL OnScreenPicture(SOCKET sctTargetSocket,
                     char *szBuffer,
                     size_t uiLen,
                     PCLIENTINFO pstClintInfo)
{

    return TRUE;
}


BOOL OnHandlePacket(PACKETTYPE ePacketType,
                    SOCKET sctTargetSocket,
                    char *szBuffer,
                    size_t uiLen,
                    PCLIENTINFO pstClientInfo,
                    CCommunicationIOCP &IOCP)
{
    BOOL bRet = FALSE;
    
    // Packet Identification.
    switch (ePacketType)
    {
        case PT_HEATBEAT:
        {
            OutputDebugString(_T("收到心跳包\r\n"));
            // Deel with heartbeat.
            bRet = OnHeartBeat(sctTargetSocket,
                               pstClientInfo,
                               IOCP);
            if (!bRet)
            {
                OutputDebugString(_T("心跳包处理失败\r\n"));
            }

            break;
        }
        // 目标主机文件列表数据
        case PT_FILE_LIST:
        {
            OutputDebugString(_T("收到文件列表信息\r\n"));
            bRet = OnFileList(sctTargetSocket,
                              szBuffer,
                              uiLen,
                              pstClientInfo,
                              IOCP);
            if (!bRet)
            {
                OutputDebugString(_T("文件列表信息处理失败"));
            }
            break;
        }
        // 目标主机盘符信息
        case PT_FILE_DEVICE:
        {
#ifdef DEBUG
            OutputDebugString(_T("收到主机盘符信息\r\n"));
#endif // DEBUG
            bRet = OnFileDevice(sctTargetSocket,
                                szBuffer,
                                uiLen,
                                pstClientInfo,
                                IOCP);
            if (!bRet)
            {
                OutputDebugString(_T("主机盘符信息处理失败"));
            }
            break;
        }
        case PT_FILE_DATA:
        {

        }
        case PT_PROCESS_INFO:
        {
#ifdef DEBUG
            OutputDebugString(_T("收到目标发来的进程信息\r\n"));
#endif // DEBUG
            bRet = OnProcessInfo(sctTargetSocket,
                                 szBuffer,
                                 uiLen,
                                 pstClientInfo);
            if (!bRet)
            {
#ifdef DEBUG
            OutputDebugString(_T("收到目标发来的进程信息\r\n"));
#endif // DEBUG
            }
            break;
        }
        case PT_PROCESSCOMMAND_KILL:
        {
            OutputDebugString(_T("进程KILL成功\r\n"));
            break;
        }
        case PT_SCREENPICTURE:
        {
            bRet = OnScreenPicture(sctTargetSocket,
                                   szBuffer,
                                   uiLen,
                                   pstClientInfo);
            break;
        }
        case PT_CMD_REPLY:
        {
#ifdef DEBUG
            OutputDebugString(_T("收到CMD的回复信息\r\n"));
#endif // DEBUG
            bRet = OnCMDReply(sctTargetSocket,
                              szBuffer,
                              uiLen,
                              pstClientInfo);
            if (!bRet)
            {
#ifdef DEBUG
                OutputDebugString(_T("CMD的回复信息\r\n"));
#endif // DEBUG
            }
            break;
        }
        default:
        {
            OutputDebugString(_T("数据包类型不在处理范围内\r\n"));
            break;
        }
    } //! switch "Packet Identification" END

    return bRet;
}