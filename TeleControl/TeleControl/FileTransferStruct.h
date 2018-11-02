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
    CString     csFileFullName_;
    ULONGLONG   ullFilePointPos_;
    CBuffer     FileDataBuffer_;
} FILEDATAINQUEUE, *PFILEDATAINQUEUE;

typedef enum tagFileTransportListColumn
{
    FTLC_FILEFULLNAME,
    FTLC_TASKTYPE,
    FTLC_TOTALSIZE,
    FTLC_TRANSMITTEDSIZE,
    FTLC_TASKSTATUS,
} FILETRANSPORTLISTCOLUMN, *PFILETRANSPORTLISTCOLUMN;

typedef enum tagFileTaskStatus
{
    FTS_START,
    FTS_TRANSPORTING,
    FTS_PAUSE,
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
    CString             csFilePath_;
    BOOL                bHasNewFileName_;
    CString             csFileOrignalName_;
    CString             csFileNewName_;
    FILETASKTYPE        eTaskType_;
    ULONGLONG           ullFileTotalSize_ = 0;
    ULONGLONG           ullTransmissionSize_ = 0;
    FILETASKSTATUS      eTaskStatus_;
} FILETRANSPORTTASK, *PFILETRANSPORTTASK;

#endif // !FILETRANSFERSTRUCT_H_
