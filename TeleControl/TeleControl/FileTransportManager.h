//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      The define for class CFileTransportManager.
//
// Modify Log:
//      2018-07-24    Hoffman
//      Info: Add below member variables;
//              m_ctGetFileTaskInfo;
//            Add declare of below member methods.
//              InsertGetFileTask();
//              GetTask(): by destination file name;
//              
//      2018-07-24    Hoffman
//      Info: Add below member variables;
//              m_mapFileObject;
//              m_queFileData;
//            Add declare of below member methods.
//              GetAllValue();
//              InsertFileObject();
//              GetFileObject();
//
//      2018-11-13    Hoffman
//      Info: Add declare of below member methods.
//              GetTask(): by task id;
//******************************************************************************

#pragma once
#ifndef FILETRANSPORTMANAGER_H_
#define FILETRANSPORTMANAGER_H_
#include "FileTransferStruct.h"

class CFileTransportManager
{
private:
    // Key is target file name with path, value is task info.
    CList<PFILETRANSPORTTASK, PFILETRANSPORTTASK> m_ctGetFileTaskInfo;
    CMap<CString, LPCTSTR, CFile *, CFile *> m_mapFileObject;

     // The queue of data from target host.
    std::queue<PFILEDATAINQUEUE> m_queFileData;       


    CCriticalSection m_CriticalSection;
public:
    CFileTransportManager();
    virtual ~CFileTransportManager();


    void InsertFileObject(_In_ const CString csFileFullName,
                          _In_ CFile *pfFileObject);

    void InsertGetFileTask(_In_ const CString csFileFullName,
                           _In_ PFILETRANSPORTTASK pstTaskInfo);

    FILETRANSPORTTASK *GetTask(_In_ const CPath &ref_phFileNameWithPathDst);
    FILETRANSPORTTASK *GetTask(_In_ const ULONG &ref_ulTaskId);

    void GetAllValue(std::vector<PFILETRANSPORTTASK> &ref_vctAllValue);

    CFile *GetFileObject(CPath &ref_phFileNameWithPathDst);



    void InsertFileDataToQueue(_In_ PFILEDATAINQUEUE pstFileData);
    PFILEDATAINQUEUE GetFileDataFromQueue();
    BOOL DeleteFileDataFromQueue();
    BOOL CheckFileDataQueueEmpty();

    BOOL DeleteTaskAndFileObject(CPath & phFileNameWithPathDst);
    void Distroy();
};

#endif // !FILETRANSPORTMANAGER_H_
