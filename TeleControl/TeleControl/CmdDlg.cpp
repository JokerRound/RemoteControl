//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      Achieve of class CCmdDlg's member method.
//
// Modify Log:
//      2018-07-24    Hoffman
//      Info: a. Add below functions.
//              a.1 OnHascmdreply();
//              a.2 OnHasordertosend();
//              
//      2018-11-22    Hoffman
//      Info: a. Modify below functions.
//              a.1 OnHasordertosend():
//                  1.1 Use SendDataUseIOCP functions to instead of 
//                     send code segment.
//              a.2 OnHascmdreply():
//                  1.2 Add check screen tail has "\r\n" or not.
//            b. Add below functions.
//              b.1 OnClose();
//              b.2 OnDestroy();
//******************************************************************************

// CmdDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TeleControl.h"
#include "CmdDlg.h"
#include "afxdialogex.h"


// CCmdDlg 对话框

IMPLEMENT_DYNAMIC(CCmdDlg, CDialogEx)

CCmdDlg::CCmdDlg(CString &ref_csIPAndPort,
                 PCLIENTINFO pClientInfo,
                 CCommunicationIOCP &ref_IOCP,
                 CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_CMD, pParent)
    , m_ref_csIPAndPort(ref_csIPAndPort)
    , m_ref_IOCP(ref_IOCP)
    , m_pstClientInfo(pClientInfo)
{

}

CCmdDlg::~CCmdDlg()
{
}

void CCmdDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDT_CMD, m_edtCmd);
}


BEGIN_MESSAGE_MAP(CCmdDlg, CDialogEx)
    ON_MESSAGE(WM_HASCMDREPLY, &CCmdDlg::OnHascmdreply)
    ON_MESSAGE(WM_HASORDERTOSEND, &CCmdDlg::OnHasordertosend)
    ON_WM_CLOSE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCmdDlg 消息处理程序


BOOL CCmdDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    // 投递Send请求，开启对端的CMD
    PPACKETFORMAT pstPacket =
        (PPACKETFORMAT)m_pstClientInfo->szSendTmpBuffer_;

    // *注意* 写入数据时要加锁
    m_pstClientInfo->CriticalSection_.Lock();
    pstPacket->ePacketType_ = PT_CMDCOMMAND_START;
    pstPacket->dwSize_ = 0;

    m_pstClientInfo->SendBuffer_.Write(
        (PBYTE)m_pstClientInfo->szSendTmpBuffer_,
        PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // 清理发送临时缓冲区
    memset(m_pstClientInfo->szSendTmpBuffer_,
           0,
           PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // 投递发送请求
    BOOL bRet =
        m_ref_IOCP.PostSendRequst(m_pstClientInfo->sctClientSocket_,
                                  m_pstClientInfo->SendBuffer_);

    m_pstClientInfo->SendBuffer_.ClearBuffer();
    m_pstClientInfo->CriticalSection_.Unlock();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}


afx_msg LRESULT CCmdDlg::OnHascmdreply(WPARAM wParam, LPARAM lParam)
{
    CString *pcsCmdReply = (CString *)wParam;

    if (m_csScreen.Right(2) != _T("\r\n"))
    {
        m_csScreen += _T("\r\n");
    }

    m_csScreen += *pcsCmdReply;

    m_edtCmd.SetWindowText(m_csScreen);

    // Focus auto move down.
    int nLenth = m_edtCmd.GetWindowTextLength();
    m_edtCmd.SetSel(nLenth, nLenth, FALSE);
    m_edtCmd.SetFocus();
    UpdateData();

    return TRUE;
}


afx_msg LRESULT CCmdDlg::OnHasordertosend(WPARAM wParam, LPARAM lParam)
{
    CString *csOreder = (CString *)wParam;

    PPACKETFORMAT pstPacket =
        (PPACKETFORMAT)m_pstClientInfo->szSendTmpBuffer_;

    SendDataUseIOCP(m_pstClientInfo,
                    m_ref_IOCP,
                    *csOreder,
                    PT_CMD_ORDER);

    //// *注意* 写入数据时要加锁
    //m_pstClientInfo->CriticalSection_.Lock();
    //pstPacket->ePacketType_ = PT_CMD_ORDER;

    //pstPacket->dwSize_ = (csOreder->GetLength() + 1) * sizeof(TCHAR);
    //memmove(pstPacket->szContent_,
    //        csOreder->GetBuffer(),
    //        pstPacket->dwSize_);


    //m_pstClientInfo->SendBuffer_.Write(
    //    (PBYTE)m_pstClientInfo->szSendTmpBuffer_,
    //    PACKET_HEADER_SIZE + pstPacket->dwSize_);

    //// 清理发送临时缓冲区
    //memset(m_pstClientInfo->szSendTmpBuffer_,
    //       0,
    //       PACKET_HEADER_SIZE + pstPacket->dwSize_);

    //// 投递发送请求
    //BOOL bRet =
    //    m_ref_IOCP.PostSendRequst(m_pstClientInfo->sctClientSocket_,
    //                              m_pstClientInfo->SendBuffer_);

    //m_pstClientInfo->SendBuffer_.ClearBuffer();
    //m_pstClientInfo->CriticalSection_.Unlock();
    
    return 0;
}


void CCmdDlg::OnClose()
{
    // Send close to target host.
    BOOL bRet = SendDataUseIOCP(m_pstClientInfo,
                                m_ref_IOCP,
                                NULL,
                                PT_CMDCOMMAND_END);
    if (!bRet)
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("Send PT_CMDCOMMAND_END faild.\r\n"),
                                  __FILET__,
                                  __LINE__);
#endif // DEBUG
    }

    CDialogEx::OnClose();
    DestroyWindow();
} //! CCmdDlg::OnClose END


void CCmdDlg::OnDestroy()
{
    CDialogEx::OnDestroy();

    GetParent()->SendMessage(WM_HASDLGCLOSE,
                             (WPARAM)m_pstClientInfo,
                             CDT_CMD);
}
