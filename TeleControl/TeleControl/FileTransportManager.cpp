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

void CFileTransportManager::InsertGetFileTask(const CString csFileFullName,
                                              PFILETRANSPORTTASK pstTaskInfo)
{
    m_CriticalSection.Lock();
    m_mapGetFileTaskInfo.SetAt(csFileFullName, pstTaskInfo);
    m_CriticalSection.Unlock();
} //! CFileTransportManager::InsertTask END

void CFileTransportManager::UpdateFileNewName(const CString csFileFullName,
                                              const CString csFileNewName)
{
    FILETRANSPORTTASK *pstTargetTask = NULL;
    pstTargetTask = GetTask(csFileFullName);
    
    pstTargetTask->csFileNewName_ = csFileNewName;
    pstTargetTask->bHasNewFileName_ = TRUE;
}


void CFileTransportManager::UpdateKey(const CString csOrginalKey, 
                                      const CString csNewKey)
{
    FILETRANSPORTTASK *pstTargetTask = NULL;
    pstTargetTask = GetTask(csOrginalKey);

    if (NULL == pstTargetTask)
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("Don't fond target data in map."),
                                  __FILET__,
                                  __LINE__);
#endif // DEBUG
    }

    InsertGetFileTask(csNewKey, pstTargetTask);
} //! CFileTransportManager::UpdateKey END

FILETRANSPORTTASK *CFileTransportManager::GetTask(const CString csFileName)
{
    FILETRANSPORTTASK *pstTargetTask = NULL;

    m_CriticalSection.Lock();
    BOOL bRet = m_mapGetFileTaskInfo.Lookup(csFileName, pstTargetTask);
    m_CriticalSection.Unlock();
    
    if (!bRet)
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T(""),
                                  __FILET__,
                                  __LINE__);
#endif // DEBUG
    }

    return pstTargetTask;
} //! CFileTransportManager::GetTask END

void CFileTransportManager::GetAllValue(std::vector<PFILETRANSPORTTASK>
                                        &ref_vctAllValue)
{
    m_CriticalSection.Lock();
    if (!m_mapGetFileTaskInfo.IsEmpty())
    {
        POSITION posI = m_mapGetFileTaskInfo.GetStartPosition();
        CString csTmpKey;
        FILETRANSPORTTASK *pstTaskInfo = NULL; 

        while (posI)
        {
            m_mapGetFileTaskInfo.GetNextAssoc(posI, csTmpKey, pstTaskInfo);

            if (NULL != pstTaskInfo)
            {
                ref_vctAllValue.push_back(pstTaskInfo);
            }
        }
    }
    m_CriticalSection.Unlock();
} //! CFileTransportManager::GetAllValue END


void CFileTransportManager::Distroy()
{
    if (!m_mapGetFileTaskInfo.IsEmpty())
    {
        POSITION posI = m_mapGetFileTaskInfo.GetStartPosition();
        CString csTmpKey;
        FILETRANSPORTTASK *pstTaskInfo = NULL; 

        while (posI)
        {
            m_mapGetFileTaskInfo.GetNextAssoc(posI, csTmpKey, pstTaskInfo);

            if (NULL != pstTaskInfo)
            {
                delete pstTaskInfo;
                pstTaskInfo = NULL;
            }
        }

        m_mapGetFileTaskInfo.RemoveAll();
    }
} //! CFileTransportManager::Distroy END
