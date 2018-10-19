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
    PT_TESE,
    PT_HEATBEAT,
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

// IOCP包类型
typedef enum tagIOCPType
{
    IOCP_RECV,
    IOCP_SEND,
} IOCPTYPE, *PIOCPTYPE;

// 通信包格式
#pragma pack(push, 1)
typedef struct tagPacketFormat
{
    PACKETTYPE  ePacketType_;
    DWORD       dwSize_;
    char        szContent_[1];
} PACKETFORMAT, *PPACKETFORMAT;
#pragma pack(pop)

// 重叠结构体封装
typedef struct tagOverlappedWithData
{
    IOCPTYPE        eIOCPType_;
    OVERLAPPED      stOverlapped_;
    WSABUF          stBuffer_;
    char            szPacket_[PACKET_CONTENT_MAXSIZE];
} OVERLAPPEDWITHDATA, *POVERLAPPEDWITHDATA;


// 接待线程参数
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
    // 最近一次通信
    time_t              tLastTime_;
    SOCKET              sctClientSocket_;
    // 读写缓冲区
    CBuffer             RecvBuffer_;
    CBuffer             SendBuffer_;
    // 接收后的临时处理缓冲区
    char                szRecvTmpBuffer_[PACKET_CONTENT_MAXSIZE];
    // 发送前的临时构建缓冲区
    char                szSendTmpBuffer_[PACKET_CONTENT_MAXSIZE];
    // 临界区
    CCriticalSection    CriticalSection_;
    // 文件传输对话框
    CFileTransferDlg    *pFileTransferDlg_ = NULL;
    // CMD对话框
    CCmdDlg             *pCmdDlg_ = NULL;
    // 进程管理对话框
    CProcessManagerDlg  *pProcessManagerDlg_ = NULL;
    // 屏幕监控
    CScreenShowerDlg    *pScreenShowerDlg_ = NULL;
} CLIENTINFO, *PCLIENTINFO;

// IOCP线程所需要的附加数据
typedef struct tagIOCPThreadAddtionData
{
    CClientManager *pClientManager_;
} IOCPTHREADADDTIONDATA, *PIOCPTHREADADDTIONDATA;

// IOCP线程参数
typedef struct tagIOCPThreadParam
{
    CCommunicationIOCP      *pIOCP_;
    PIOCPTHREADADDTIONDATA  pThreadAddtionData_;
} IOCPTHREADPARAM, *PIOCPTHREADPARAM;
#endif // !STRUCTSHARE_H_
