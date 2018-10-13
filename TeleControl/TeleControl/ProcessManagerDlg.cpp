// ProcessManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TeleControl.h"
#include "ProcessManagerDlg.h"
#include "afxdialogex.h"


// CProcessManagerDlg 对话框

IMPLEMENT_DYNAMIC(CProcessManagerDlg, CDialogEx)

CProcessManagerDlg::CProcessManagerDlg(CString &ref_csIPAndPort,
                                       PCLIENTINFO pstClientInfo,
                                       CCommunicationIOCP &ref_IOCP,
                                       CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PROCESSMANAGER, pParent)
    , m_ref_IOCP(ref_IOCP)
    , m_ref_csIPAndPort(ref_csIPAndPort)
{
    m_pstClientInfo = pstClientInfo;
}

CProcessManagerDlg::~CProcessManagerDlg()
{
}

void CProcessManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProcessManagerDlg, CDialogEx)
END_MESSAGE_MAP()


// CProcessManagerDlg 消息处理程序


BOOL CProcessManagerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    do
    {
        PPACKETFORMAT pstPacket =
            (PPACKETFORMAT)m_pstClientInfo->szSendTmpBuffer_;

        // *注意* 写入数据时要加锁
        m_pstClientInfo->CriticalSection_.Lock();
        pstPacket->ePacketType_ = PT_PROCESS_INFO;
        pstPacket->dwSize_ = 0;

        m_pstClientInfo->SendBuffer_.Write(
            (PBYTE)m_pstClientInfo->szSendTmpBuffer_,
            PACKET_HEADER_SIZE + pstPacket->dwSize_);
        // 清空临时发送区
        memset(m_pstClientInfo->szSendTmpBuffer_,
               0,
               PACKET_HEADER_SIZE + pstPacket->dwSize_);

        // 投递发送请求
        BOOL bRet = 
            m_ref_IOCP.PostSendRequst(m_pstClientInfo->sctClientSocket_,
                                      m_pstClientInfo->SendBuffer_);

        m_pstClientInfo->CriticalSection_.Unlock();

        DWORD dwRet = WaitForSingleObject(m_hGetProcessInfoOver, INFINITE);
        if (dwRet != WAIT_OBJECT_0)
        {
#ifdef DEBUG
            OutputDebugString(_T("等待获取目标进程信息失败"));
#endif // DEBUG
            break;
        }

        // 解析获取的信息，插入列表



    } while (FALSE);


    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}
