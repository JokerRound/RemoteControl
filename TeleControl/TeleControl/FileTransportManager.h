#pragma once
#ifndef FILETRANSPORTMANAGER_H_
#define FILETRANSPORTMANAGER_H_
#include "FileTransferStruct.h"

class CFileTransportManager
{
private:
    CMap<CString, LPCTSTR, 
        PFILETRANSPORTTASK, PFILETRANSPORTTASK> m_mapGetFileTaskInfo;

    CCriticalSection m_CriticalSection;
public:
    CFileTransportManager();
    virtual ~CFileTransportManager();

    void InsertGetFileTask(_In_ const CString csFileName,
                           _In_ PFILETRANSPORTTASK pstTaskInfo);

    PFILETRANSPORTTASK GetTask(_In_ const CString csFileName);

    void CheckTask();

    void Distroy();
};

#endif // !FILETRANSPORTMANAGER_H_
