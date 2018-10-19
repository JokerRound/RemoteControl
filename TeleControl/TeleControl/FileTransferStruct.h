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

//typedef struct tagFileInfo
//{
//    int iIconIndex;
//    // File and Time use '?' to separate.
//    TCHAR *szFileNameAndTime;
//} FILEINFO, *PFILEINFO;

typedef enum tagFileTaskState
{
    FTS_START,
    FTS_PAUSE,
    FTS_END,
} FILETASKSTATE, *PFILETASKSTATE;

typedef enum tagFileTaskType
{
    FTT_GETFILE,
    FTT_PUTFILE,
} FILETASKTYPE, *PFILETASKTYEP;

typedef struct tagFileTransportTask
{
    CString             csFileFullPath_;
    FILETASKTYPE        eTaskType_;
    u_int64             uint64FileTotalSize_ = 0;
    u_int64             uint64TransmissionSize_ = 0;
    FILETASKSTATE       eTaskState_;
} FILETRANSPORTTASK, *PFILETRANSPORTTASK;

#endif // !FILETRANSFERSTRUCT_H_
