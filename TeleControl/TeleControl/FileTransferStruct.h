#pragma once
#ifndef FILETRANSFERSTRUCT_H_
#define FILETRANSFERSTRUCT_H_

enum tagFileListColumnType
{
    FLCT_FILENAME,
    FLCT_WRITETIME,
    FLCT_FILETYPE,
    FLCT_FILESIZE,
};

enum tagFileListStyle
{
    FLS_NORMALICON,
    FLS_SMALLICON,
    FLS_LIST,
    FLS_REPORT,
};
typedef struct tagFileInfo
{
    int iIconIndex;
    // File and Time use '?' to separate.
    char *szFileNameAndTime;
} FILEINFO, *PFILEINFO;

#endif // !FILETRANSFERSTRUCT_H_
