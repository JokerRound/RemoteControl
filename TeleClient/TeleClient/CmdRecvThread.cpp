#include "stdafx.h"
#include "TeleClientDlg.h"
#include "CmdRecvThread.h"


CCmdRecvThread::CCmdRecvThread()
{
}


CCmdRecvThread::~CCmdRecvThread()
{
}

bool CCmdRecvThread::OnThreadEventRun(LPVOID lpParam)
{
    CTeleClientDlg *pTeleClientDlg = (CTeleClientDlg *)lpParam;
    BOOL bRet = FALSE;

    while (TRUE)
    {
        // 从管道中接收数据
        bRet = pTeleClientDlg->ReadDataFromCmd();
        if (!bRet)
        {
#ifdef DEBUG
            OutputDebugString(_T("从CMD中读取数据失败\r\n"));
#endif // DEBUG
            if (pTeleClientDlg->m_bCmdQuit)
            {
                break;
            }
        }
    }

    return true;
}
