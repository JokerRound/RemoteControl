#include "stdafx.h"
#include "ClientManager.h"
#include "FileTransferDlg.h"

CClientManager::CClientManager()
{
}


CClientManager::~CClientManager()
{
    m_CriticalSection.Lock();
    Destory();
    m_CriticalSection.Unlock();
}

void CClientManager::InsertClient(const SOCKET &sctTargetSocket,
                                  tagClientInfo *pstClientInfo)
{
    m_CriticalSection.Lock();
    m_mapClient.SetAt(sctTargetSocket, pstClientInfo);
    m_CriticalSection.Unlock();
}

BOOL CClientManager::InsertSocket(const CString &ref_csIPAndPort,
                                  const SOCKET &ref_sctTargetSocket)
{
    // Update mapping which IP and port with socket.
    m_CriticalSection.Lock();
    m_mapIPPortWithSocket.SetAt(ref_csIPAndPort, ref_sctTargetSocket);
    m_CriticalSection.Unlock();

    return TRUE;
}

SOCKET CClientManager::GetSocket(const CString &ref_csIPAndPort)
{
    SOCKET sctTarget = INVALID_SOCKET;
    m_CriticalSection.Lock();
    BOOL bRet = m_mapIPPortWithSocket.Lookup(ref_csIPAndPort, sctTarget);
    if (!bRet)
    {
        sctTarget = INVALID_SOCKET;
    }
    m_CriticalSection.Unlock();

    return sctTarget;
}

tagClientInfo *CClientManager::GetClient(const SOCKET & sctTargetSocket)
{
    m_CriticalSection.Lock();
    tagClientInfo *pClient = NULL;

    BOOL bRet = m_mapClient.Lookup(sctTargetSocket, pClient);
    if (!bRet)
    {
        pClient = NULL;
    }
    m_CriticalSection.Unlock();

    return pClient;
}

BOOL CClientManager::RemoveClient(const SOCKET &sctTargetSocket)
{
    m_CriticalSection.Lock();
    tagClientInfo *pClient = NULL;
    do
    {
        BOOL bRet = m_mapClient.Lookup(sctTargetSocket, pClient);
        if (!bRet)
        {
            break;
        }

        if (pClient != NULL)
        {
            delete pClient;
            pClient = NULL;
        }

    } while (FALSE);
    m_CriticalSection.Unlock();

    return m_mapClient.RemoveKey(sctTargetSocket);
}

// 
void CClientManager::Destory()
{
    // The socket will close when clean m_mapClient, so remove direct.
    if (!m_mapIPPortWithSocket.IsEmpty())
    {
        m_mapIPPortWithSocket.RemoveAll();
    }

    // Cleanning m_mapClient, the main stage is free PCLIENTINFO struct.
    if (!m_mapClient.IsEmpty())
    {
        SOCKET sctTmp = SOCKET_ERROR;
        PCLIENTINFO pstClientInfoTmp = NULL;
        // Get first postion.
        POSITION posI = m_mapClient.GetStartPosition();

        while (posI)
        {
            m_mapClient.GetNextAssoc(posI, sctTmp, pstClientInfoTmp);

            // Main stage.
            if (NULL != pstClientInfoTmp)
            {
                if (NULL != pstClientInfoTmp->pFileTransferDlg_)
                {
                    delete pstClientInfoTmp->pFileTransferDlg_;
                    pstClientInfoTmp->pFileTransferDlg_ = NULL;
                }

                delete pstClientInfoTmp;
                pstClientInfoTmp = NULL;
            }

            // Shutdown the socket.
            if (SOCKET_ERROR != sctTmp)
            {
                shutdown(sctTmp, SD_SEND);
                closesocket(sctTmp);
                sctTmp = SOCKET_ERROR;
            }
        }
        
        m_mapClient.RemoveAll();
    }
} //! CClientManager::Destory END
