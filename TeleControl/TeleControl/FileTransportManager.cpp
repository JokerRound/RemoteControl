//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      Achieve of class CFileTransportManager's member method.
//
// Modify Log:
//      2018-11-10    Hoffman
//      Info: a. Move achieve of below methods from FileTransferDlg.cpp.
//              a.1. InsertFileDataToQueue();
//              a.2. GetFileDataFromQueue();
//              a.3. CheckFileDataQueueEmpty();
//
//      2018-11-13    Hoffman
//      Info: Modify achieve of below member methods.
//            Add ahcieve of below member method.
//              GetTask(): overload by id.
//******************************************************************************

#include "stdafx.h"
#include "FileTransportManager.h"


CFileTransportManager::CFileTransportManager()
{
}


CFileTransportManager::~CFileTransportManager()
{
    m_CriticalSection.Lock();
    Distroy();
    m_CriticalSection.Unlock();
}

void CFileTransportManager::InsertFileObject(const CString csFileFullName,
                                             CFile *pfFileObject)
{
    m_CriticalSection.Lock();
    m_mapFileObject.SetAt(csFileFullName, pfFileObject);
    m_CriticalSection.Unlock();
} //! CFileTransportManager::InsertFileObject END

void CFileTransportManager::InsertGetFileTask(const CString csFileFullName,
                                              PFILETRANSPORTTASK pstTaskInfo)
{
    m_CriticalSection.Lock();
    m_ctGetFileTaskInfo.AddTail(pstTaskInfo);
    m_CriticalSection.Unlock();
} //! CFileTransportManager::InsertTask END


FILETRANSPORTTASK *CFileTransportManager::GetTask(
    const CPath &ref_phFileNameWithPathDst)
{
    FILETRANSPORTTASK *pstTargetTask = NULL;

    m_CriticalSection.Lock();
    POSITION posI = m_ctGetFileTaskInfo.GetHeadPosition();

    while (posI)
    {
        pstTargetTask = m_ctGetFileTaskInfo.GetNext(posI);
        if (NULL != pstTargetTask &&
            pstTargetTask->pathFileNameWithPathDst_.m_strPath ==
            ref_phFileNameWithPathDst.m_strPath)
        {
            break;
        }
    }

    m_CriticalSection.Unlock();
    return pstTargetTask;
} //! CFileTransportManager::GetTask END

// Traversing the chain table for founding the target task by id.
FILETRANSPORTTASK *CFileTransportManager::GetTask(const ULONG &ref_ulTaskId)
{
    FILETRANSPORTTASK *pstTargetTask = NULL;

    m_CriticalSection.Lock();
    POSITION posI = m_ctGetFileTaskInfo.GetHeadPosition();

    while (posI)
    {
        pstTargetTask = m_ctGetFileTaskInfo.GetNext(posI);
        if (NULL != pstTargetTask &&
            pstTargetTask->ulId_ == ref_ulTaskId)
        {
            break;
        }
    }

    m_CriticalSection.Unlock();
    return pstTargetTask;
} //! CFileTransportManager::GetTask END

void CFileTransportManager::GetAllValue(std::vector<PFILETRANSPORTTASK>
                                        &ref_vctAllValue)
{
    m_CriticalSection.Lock();
    if (!m_ctGetFileTaskInfo.IsEmpty())
    {
        POSITION posI = m_ctGetFileTaskInfo.GetHeadPosition();
        CString csTmpKey;
        FILETRANSPORTTASK *pstTaskInfo = NULL; 

        while (posI)
        {
            pstTaskInfo = m_ctGetFileTaskInfo.GetNext(posI);

            if (NULL != pstTaskInfo)
            {
                ref_vctAllValue.push_back(pstTaskInfo);
            }
        }
    }
    m_CriticalSection.Unlock();
} //! CFileTransportManager::GetAllValue END

CFile *CFileTransportManager::GetFileObject(CPath &ref_phFileNameWithPathDst)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
#endif // DEBUG

    CFile *pfTargetFile = NULL; 

    m_CriticalSection.Lock();
    BOOL bRet = m_mapFileObject.Lookup(ref_phFileNameWithPathDst, pfTargetFile);
    if (!bRet)
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("Don't fond the file object.\r\n"),
                                  __FILET__,
                                  __LINE__);
#endif // DEBUG
    }   
    m_CriticalSection.Unlock();

    return pfTargetFile;
} //! CFileTransportManager::GetFileObject END



BOOL CFileTransportManager::DeleteTaskAndFileObject(
    CPath &ref_phFileNameWithPathDst)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLien = 0;
#endif // DEBUG
    BOOL bNoError = FALSE;
    CFile *pfTargetFile = NULL;
    PFILETRANSPORTTASK pstTask = NULL;

    m_CriticalSection.Lock();
    do
    {
        if (!m_mapFileObject.IsEmpty())
        {
            bNoError = m_mapFileObject.Lookup(ref_phFileNameWithPathDst,
                                              pfTargetFile);
            if (!bNoError)
            {
#ifdef DEBUG
                OutputDebugStringWithInfo(_T("Don't found target file object."),
                                          __FILET__,
                                          __LINE__);
#endif // DEBUG
                break;
            }

            if (NULL != pfTargetFile)
            {
                pfTargetFile->Close();

                delete pfTargetFile;
                pfTargetFile = NULL;
            }

            bNoError = m_mapFileObject.RemoveKey(ref_phFileNameWithPathDst);
            if (!bNoError)
            {
#ifdef DEBUG
                dwLien = __LINE__;
#endif // DEBUG
                break;
            }
        }

        if (!m_ctGetFileTaskInfo.IsEmpty())
        {
            POSITION posI = m_ctGetFileTaskInfo.GetHeadPosition();
            while (posI)
            {
                POSITION posCurrently = posI;

                pstTask = m_ctGetFileTaskInfo.GetNext(posI);
                if (NULL != pstTask && 
                    (pstTask->pathFileNameWithPathDst_.m_strPath == 
                    ref_phFileNameWithPathDst.m_strPath))
                {
                    delete pstTask;
                    pstTask = NULL;
                    m_ctGetFileTaskInfo.RemoveAt(posCurrently);
                }
            }
        }

        bNoError = TRUE;
    } while (FALSE);
    m_CriticalSection.Unlock();

    return bNoError;
} //! CFileTransportManager::DeleteTaskAndFileObject END

void CFileTransportManager::Distroy()
{
    if (!m_ctGetFileTaskInfo.IsEmpty())
    {
        POSITION posI = m_ctGetFileTaskInfo.GetHeadPosition();
        CString csTmpKey;
        FILETRANSPORTTASK *pstTaskInfo = NULL; 

        while (posI)
        {
            pstTaskInfo = m_ctGetFileTaskInfo.GetNext(posI);

            if (NULL != pstTaskInfo)
            {
                delete pstTaskInfo;
                pstTaskInfo = NULL;
            }
        }

        m_ctGetFileTaskInfo.RemoveAll();
    }

    if (!m_mapFileObject.IsEmpty())
    {
        POSITION posI = m_mapFileObject.GetStartPosition();
        CString csTmpKey;
        CFile *pfFileObject = NULL;

        while (posI)
        {
            m_mapFileObject.GetNextAssoc(posI, csTmpKey, pfFileObject);

            if (NULL != pfFileObject)
            {
                delete pfFileObject;
                pfFileObject = NULL;
            }
        }

        m_mapFileObject.RemoveAll();
    }

} //! CFileTransportManager::Distroy END

void CFileTransportManager::InsertFileDataToQueue(PFILEDATAINQUEUE pstFileData)
{
    m_CriticalSection.Lock();
    m_queFileData.push(pstFileData);
    m_CriticalSection.Unlock();
}

PFILEDATAINQUEUE CFileTransportManager::GetFileDataFromQueue()
{
    PFILEDATAINQUEUE pstFileData = NULL;
    m_CriticalSection.Lock();
    if (!m_queFileData.empty())
    {
        pstFileData = m_queFileData.front();
        m_queFileData.pop();
    }
    else
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("Want to get file data "
                                     "but queue is empty.\r\n"), 
                                  __FILET__, 
                                  __LINE__);
#endif // DEBUG
    }
    m_CriticalSection.Unlock();

    return pstFileData;
} //! CFileTransferDlg::GetFileDataFromQueue END


BOOL CFileTransportManager::CheckFileDataQueueEmpty()
{
    m_CriticalSection.Lock();
    BOOL bRet = m_queFileData.empty();
    m_CriticalSection.Unlock();
    if (bRet)
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("No data in queue.\r\n"),
                                  __FILET__,
                                  __LINE__);
#endif // DEBUG
    }

    return bRet;
} //! CFileTransferDlg::CheckFileDataQueueEmpty END

