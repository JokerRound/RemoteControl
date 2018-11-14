//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      The define for class CClientManager.
//
// Modify Log:
//      2018-07-24    Hoffman
//      Info: Add below member variables.
//              m_mapClient;
//              m_mapIPPortWithSocket;
//              m_CriticalSection;
//            Add below member methods.
//              InserClient();
//******************************************************************************

#pragma once
#ifndef CLIENTMANAGER_H_
#define CLIENTMANAGER_H_
#include "stdafx.h"
#include "StructShare.h"

class CClientManager
{
private:
    // Binding client infomation with SOCKET.
    CMap<SOCKET, SOCKET, tagClientInfo *, tagClientInfo *> m_mapClient;
    // Binding SOCKET with IP and port. 
    CMap<CString, LPCTSTR, SOCKET, SOCKET> m_mapIPPortWithSocket;

    CCriticalSection m_CriticalSection;
public:
    CClientManager();
    virtual ~CClientManager();

    //**********************************************************************
    // FUNTION:     Insert client infomation to m_mapClient
    // OUPUT:       None
    // RETURN:      Void
    // PARAMETER:
    //      sctTargetSocket:    A socket which take to bind, it's Key in map
    //      pClientInfo:        A ClientInfo point which bind with socket,
    //                          it's value in map
    // NOTE:        
    //      1. Point cann't be NULL.
    //**********************************************************************
    void InsertClient(_In_ const SOCKET &sctTargetSocket,
                      _In_ tagClientInfo *pstClientInfo);


    // Insert SOECKET to m_mapIPPortWithSocket
    BOOL InsertSocket(_In_ const CString &ref_csIPAndPort, 
                      _In_ const SOCKET &ref_sctTargetSocket);

    SOCKET GetSocket(_In_ const CString &csIPAndPort);

    tagClientInfo *GetClient(_In_ const SOCKET &sctTargetSocket);

    BOOL RemoveClient(_In_ const SOCKET &sctTargetSocket);

    // Clean and free resource
    void Destory();
};

#endif // !SOCKETMANAGER_H_
