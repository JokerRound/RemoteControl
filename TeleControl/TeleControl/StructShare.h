//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      The common type of be used by this project.
//
// Modify Log:
//      2018-11-28    Hoffman
//      Info: a. Modify below type.
//              a.1 enum tagPacketType;
//******************************************************************************

#pragma once
#ifndef STRUCTSHARE_H_
#define STRUCTSHARE_H_
#include "stdafx.h"
#include "MacroShare.h"
#include "CBuffer.h"

class CClientManager;
class CCommunicationIOCP;
class CHostListView;
class CFileTransferDlg;
class CCmdDlg;
class CProcessManagerDlg;
class CScreenShowerDlg;

namespace std
{
#ifdef _UNICODE
    typedef std::wstring tstring;
    typedef std::wsmatch tsmatch;
    typedef std::wcmatch tcmatch;
#else
    typedef std::string tstring;
    typedef std::smatch tsmatch;
    typedef std::cmatch tcmatch;
#endif
}


// Type of packet
typedef enum tagPacketType
{
    PT_TEST,
    PT_HEARTBEAT,
    PT_SCREENPICTURE,
    PT_PROCESS_INFO,
    PT_PROCESSCOMMAND_KILL,
    // Used when get target host file list.
    PT_FILE_LIST,
    // Used when get target host device.
    PT_FILE_DEVICE,
    // Used when need to get or put file.
    PT_FILE_DATA,
    PT_FILECOMMAND_PUTFILE,
    PT_FILECOMMAND_GETFILE,
    PT_FILECOMMAND_PAUSE,
    PT_FILECOMMAND_CONTINUE,
    PT_CMD_ORDER,
    PT_CMD_REPLY,
    PT_CMDCOMMAND_START,
    PT_CMDCOMMAND_END,
} PACKETTYPE;

// Customize Dialog Type
typedef enum tagDialogType
{
    CDT_FILETRANSFER,
    CDT_CMD,
    CDT_PROCESSMANAGER,
    CDT_SCREENSHOWER,
} DIALOGTYPE;

// 界面信息刷新类型
typedef enum tagInfoFlushType
{
    IFT_ACCEPTCLIENT,
    IFT_SERVERSTART,
    IFT_SERVERCLOSE,
} INFOFLUSHTYPE;

// 主机列表的列类型
typedef enum tagHostListColumnType
{
    HLCT_IPADDR,
    HLCT_PORT,
} HOSTLISTCOLUMNTYPE;

// type of log list.
typedef enum tagLogListColumnType
{
    LLCT_TIME,
    LLCT_TYPE,
    LLCT_INFO,
} LOGLISTCOLUMNTYPE;

// type of process list.
typedef enum tagProcessListColumnTYPE
{
    PLCT_IMAGENAME,
    PLCT_ID,
    PLCT_PARENTID,
    PLCT_IMAGEPATH,
} PROCESSLISTCOLUMNTYPE;

typedef enum tagIOCPType
{
    IOCP_RECV,
    IOCP_SEND,
} IOCPTYPE, *PIOCPTYPE;

#pragma pack(push, 1)
typedef struct tagPacketFormat
{
    PACKETTYPE      ePacketType_;
    DWORD           dwSize_;
    TCHAR           szFileFullName_[MAX_PATH + sizeof(TCHAR)];
    ULONGLONG       ullFilePointPos_;
    ULONG           ulTaskId_;
    char            szContent_[1];
} PACKETFORMAT, *PPACKETFORMAT;
#pragma pack(pop)


typedef struct tagOverlappedWithData
{
    IOCPTYPE        eIOCPType_;
    OVERLAPPED      stOverlapped_;
    WSABUF          stBuffer_;
    char            szPacket_[PACKET_CONTENT_MAXSIZE];
} OVERLAPPEDWITHDATA, *POVERLAPPEDWITHDATA;


typedef struct tagAcceptThreadParam
{
    SOCKET              sctAcceptSocket_;
    CClientManager      *pClientManager_;
    CCommunicationIOCP  *pIOCP_;
    CHostListView       *pHostListView_;
} ACCEPTTHREADPARAM, *PACCEPTTHREADPARAM;

// 客户端信息
typedef struct tagClientInfo
{
    sockaddr_in         stClientAddrInfo_;
    time_t              tLastTime_;
    SOCKET              sctClientSocket_;
    CBuffer             RecvBuffer_;
    CBuffer             SendBuffer_;
    char                szRecvTmpBuffer_[PACKET_CONTENT_MAXSIZE];
    char                szSendTmpBuffer_[PACKET_CONTENT_MAXSIZE];
    CCriticalSection    CriticalSection_;
    CFileTransferDlg    *pFileTransferDlg_ = NULL;
    CCmdDlg             *pCmdDlg_ = NULL;
    CProcessManagerDlg  *pProcessManagerDlg_ = NULL;
    CScreenShowerDlg    *pScreenShowerDlg_ = NULL;
} CLIENTINFO, *PCLIENTINFO;

typedef struct tagIOCPThreadAddtionData
{
    CClientManager *pClientManager_;
} IOCPTHREADADDTIONDATA, *PIOCPTHREADADDTIONDATA;

typedef struct tagIOCPThreadParam
{
    CCommunicationIOCP      *pIOCP_;
    PIOCPTHREADADDTIONDATA  pThreadAddtionData_;
} IOCPTHREADPARAM, *PIOCPTHREADPARAM;
#endif // !STRUCTSHARE_H_
