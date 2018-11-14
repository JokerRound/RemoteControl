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
    ULONGLONG   ullTaskId_;
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
    CPath               phFileNameWithPathSrc_;
    CPath               phFileNameWithPathDst_;
    FILETASKTYPE        eTaskType_;
    ULONGLONG           ullFileTotalSize_ = 0;
    ULONGLONG           ullTransmissionSize_ = 0;
    FILETASKSTATUS      eTaskStatus_;
    ULONGLONG           ullId_;
    int                 iIdxInTaskList_;
} FILETRANSPORTTASK, *PFILETRANSPORTTASK;

typedef enum tagFileDlgUpdateType
{
    FDUT_TASKINFO,
    FDUT_ERROR,
} FILEDLGUPDATETYPE, *PFileDlgUpdateType;

#endif // !FILETRANSFERSTRUCT_H_
