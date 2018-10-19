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

    // 发送命令给目标服务器
    // 投递Send请求，开启对端的CMD
    PPACKETFORMAT pstPacket =
        (PPACKETFORMAT)m_pstClientInfo->szSendTmpBuffer_;

    // *注意* 写入数据时要加锁
    m_pstClientInfo->CriticalSection_.Lock();
    pstPacket->ePacketType_ = PT_CMD_ORDER;

    pstPacket->dwSize_ = (csOreder->GetLength() + 1) * sizeof(TCHAR);
    memmove(pstPacket->szContent_,
            csOreder->GetBuffer(),
            pstPacket->dwSize_);


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
    
    return 0;
}
