//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-27
// Description: 
//      The data structs are used by file transport related.
//
// Modify Log:
//      2018-11-27    Hoffman
//      Info: a. Add below data structs.
//              a.1 enum FILETRANSFERDLGMENUTYPE;
//
//******************************************************************************

#pragma once
#ifndef FILETRANSFERSTRUCT_H_
#define FILETRANSFERSTRUCT_H_

typedef enum tagFileListColumnType
{
    FLCT_FILENAME,
    FLCT_WRITETIME,
    FLCT_FILETYPE,
    FLCT_FILESIZE,
} FILELISTCOLUMNTYPE, *PFILELISTCOLUMNTYPE;

typedef enum tagFileListStyle
{
    FLS_NORMALICON,
    FLS_SMALLICON,
    FLS_LIST,
    FLS_REPORT,
} FILELISTSTYLE, *PFILELISTSTYLE;

typedef struct tagFileDataInQueue
{
    CPath       phFileNameWithPath_;
    ULONGLONG   ullFilePointPos_;
    ULONG       ulTaskId_;
    CBuffer     FileDataBuffer_;
} FILEDATAINQUEUE, *PFILEDATAINQUEUE;

typedef enum tagFileTransportListColumn
{
    FTLC_DSTFILE,
    FTLC_SRCFILE,
    FTLC_TASKTYPE,
    FTLC_TOTALSIZE,
    FTLC_TRANSMITTEDSIZE,
    FTLC_TASKSTATUS,
} FILETRANSPORTLISTCOLUMN, *PFILETRANSPORTLISTCOLUMN;

typedef enum tagFileTransmittionParticipantType
{
    FTPT_SERVER,
    FTPT_CLIENT,

    NUM_PARTICIPANT
}FILETRANSMITTIONPARTICIPANTTYPE, *PFILETRANSMITTIONPARTICIPANTTYPE;

typedef enum tagFileTaskStatus
{
    FTS_PAUSE,
    FTS_TRANSPORTING,
    FTS_PENDING,
    FTS_ERROR,
    FTS_FINISH,

    NUM_FILETASKSTATUS
} FILETASKSTATUS, *PFILETASKSTATUS;

typedef enum tagFileTaskType
{
    FTT_GETFILE,
    FTT_PUTFILE,

    NUM_FILETASKTYPE
} FILETASKTYPE, *PFILETASKTYEP;

typedef struct tagFileTransportTask
{
    CPath               pathFileNameWithPathSrc_;
    CPath               pathFileNameWithPathDst_;
    FILETASKTYPE        eTaskType_;
    ULONGLONG           ullFileTotalSize_ = 0;
    ULONGLONG           ullTransmissionSize_ = 0;
    FILETASKSTATUS      eTaskStatus_;
    ULONG               ulId_;
    int                 iIdxInTaskList_;
    CCriticalSection    syncCriticalSection_;
} FILETRANSPORTTASK, *PFILETRANSPORTTASK;

typedef enum tagFileDlgUpdateType
{
    FDUT_TASKINFO,
    FDUT_ERROR,
} FILEDLGUPDATETYPE, *PFileDlgUpdateType;


typedef enum tagFileTransferDlgMenuType
{
    FTDMT_TRANSPORT_TASK_LIST_RBTNDOWN,
    FTDMT_FILE_LIST_RBTNDOWN,

    TOTAL_FTDMT_NUM,
} FILETRANSFERDLGMENUTYPE, *PFILETRANSFERDLGMENUTYPE;

typedef enum tagTransportTaskListRMenuItem
{
    TTLRMI_PAUSEALLTASK,
    TTLRMI_STARTALLTASK,
    TTLRMI_PAUSESELECTEDTASK,
    TTLRMI_STARTSELECTEDTASK,

} TRANSPORTTASKLISTRMENUITEM, *PTRANSPORTTASKLISTRMENUITEM;

typedef enum tagTransportTaskListMenuType
{
    TTLMT_HASSELECTEDITEM,
    TTLMT_NOSELECTEDITEM,
    
    TOTAL_TTLMT_NUM,
} TRANSPORTTASKLISTMENUTYPE, *PTRANSPORTTASKLISTMENUTYPE;

#endif // !FILETRANSFERSTRUCT_H_
