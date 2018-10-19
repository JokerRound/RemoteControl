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

void CFileTransportManager::InsertGetFileTask(const CString csFileName,
                                              FILETRANSPORTTASK *pstTaskInfo)
{
    m_CriticalSection.Lock();
    m_mapGetFileTaskInfo.SetAt(csFileName, pstTaskInfo);
    m_CriticalSection.Unlock();
} //! CFileTransportManager::InsertTask END


FILETRANSPORTTASK *CFileTransportManager::GetTask(const CString csFileName)
{
    FILETRANSPORTTASK *pstTargetTask = NULL;

    m_CriticalSection.Lock();
    BOOL bRet = m_mapGetFileTaskInfo.Lookup(csFileName, pstTargetTask);
    m_CriticalSection.Unlock();
    
    if (!bRet)
    {
#ifdef DEBUG

#endif // DEBUG
    }

    return pstTargetTask;
} //! CFileTransportManager::GetTask END

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
