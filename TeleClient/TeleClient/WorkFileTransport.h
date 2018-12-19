//******************************************************************************
// License:     MIT
// Author:      Hoffman
// GitHub:      https://github.com/JokerRound
// Create Time: 2018-12-08
// Description: 
//      The header file of class CWorkFileTrasnport.
//
// Modify Log:
//      2018-12-08    Hoffman
//      Info: a. Add below member method.
//              a.1. WorkBody();
//******************************************************************************

#pragma once
#include "Work.h"

// The parament of thread to transport file.
typedef struct tagFileTransportThreadParam
{
    CString             *pcsFileListToGet_;
    CTeleClientDlg      *pTeleClientDlg_;
    CCommunicationIOCP  *pIOCP_;
    PCLIENTINFO         pstClientInfo_;
} FILETRANSPORTTHREADPARAM, *PFILETRANSPORTTHREADPARAM;

class CWorkFileTransport :
    public CWork
{
private:
    FILETRANSPORTTHREADPARAM m_stParam = { NULL };
public:
    CWorkFileTransport(_In_ PVOID pvContext);
    virtual ~CWorkFileTransport();

    
    //**************************************************************************
    // FUNCTION:    The primary body of work, called by WorkCallBack funciton.
    // RETURN:      Successfully (true) or failly (false).
    // PARAMETER:   
    //      lParam: The parament passed by WorkCallBack function.
    //**************************************************************************
    virtual BOOL WorkBody() noexcept(FALSE);
};

