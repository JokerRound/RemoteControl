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

    void InsertGetFileTask(_In_ const CString csFileFullName,
                           _In_ PFILETRANSPORTTASK pstTaskInfo);

    void UpdateFileNewName(_In_ const CString csFileFullName,
                           _In_ const CString csFileNewName);

    void UpdateKey(_In_ const CString csOrginalKey,
                   _In_ const CString csNewKey);

    PFILETRANSPORTTASK GetTask(_In_ const CString csFileName);

    void GetAllValue(std::vector<PFILETRANSPORTTASK> &ref_vctAllValue);

    void CheckTask();

    void Distroy();
};

#endif // !FILETRANSPORTMANAGER_H_
