
// TeleClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "assistFunc.h"
#include "TeleClient.h"
#include "TeleClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTeleClientDlg 对话框



CTeleClientDlg::CTeleClientDlg(CWnd* pParent /*=NULL*/)
    : CDialog(IDD_TELECLIENT_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_sctConnect = INVALID_SOCKET;


    if (NULL == m_pevtGetFileThreadInitializeEvent)
    {
        m_pevtGetFileThreadInitializeEvent = new CEvent(FALSE, FALSE);
    }
}

void CTeleClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTeleClientDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_BREAKEVENTSIGNAL, &CTeleClientDlg::OnBreakeventsignal)
    ON_WM_TIMER()
    ON_MESSAGE(WM_CREATECMDRECVTHREAD, &CTeleClientDlg::OnCreatecmdrecvthread)
    ON_MESSAGE(WM_SENDORDERTOCMD, &CTeleClientDlg::OnSendordertocmd)
    ON_MESSAGE(WM_GETFILE, &CTeleClientDlg::OnGetfile)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CTeleClientDlg 消息处理程序

BOOL CTeleClientDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);            // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标

    ShowWindow(SW_HIDE);

    // 在此添加额外的初始化代码
    do
    {
        // 创建Socket
        m_sctConnect = socket(AF_INET,
                              SOCK_STREAM,
                              IPPROTO_TCP);

        if (m_sctConnect == INVALID_SOCKET)
        {
            OutputDebugString(_T("创建Socket失败"));
            break;
        }

        // 读取配置获取服务器地址
        GetPrivateProfileString(
            _T("Base"),
            _T("ServerIp"),
            NULL,
            m_stGetProfile.csServerAddr_.GetBufferSetLength(MAXBYTE),
            MAXBYTE,
            m_csInitFile);
        m_stGetProfile.csServerAddr_.ReleaseBuffer();

        // 获取服务器端口
        m_stGetProfile.dwPort_ = GetPrivateProfileInt(_T("Base"),
                                                      _T("Port"),
                                                      NULL,
                                                      m_csInitFile);

        // 初始化服务器的地址信息
        sockaddr_in stServerAddrInfo = { 0 };
        stServerAddrInfo.sin_family = AF_INET;
        stServerAddrInfo.sin_port = htons((short)m_stGetProfile.dwPort_);

        USES_CONVERSION;
        stServerAddrInfo.sin_addr.S_un.S_addr = 
            inet_addr(T2A(m_stGetProfile.csServerAddr_.GetString()));


        // 为IOCP进程添加附加信息
        m_stIOCPThreadAddtionData.pTeleClientDlg = this;
        // 创建IOCP
        BOOL bRet = m_IOCP.Create(&m_stIOCPThreadAddtionData);
        if (!bRet)
        {
            OutputDebugString(_T("IOCP创建失败"));
            break;
        }

        // 创建事件
        SECURITY_ATTRIBUTES stSa = { 0 };
        stSa.nLength = sizeof(stSa);
        stSa.bInheritHandle = FALSE;
        m_hBreakEvent = CreateEvent(&stSa,
                                    FALSE,
                                    FALSE,
                                    NULL);
        if (m_hBreakEvent == NULL)
        {
            OutputDebugString(_T("中断事件创建失败"));
            break;
        }

        // 开启线程尝试连接服务器
        m_pstConnectThreadParam = new CONNECTTHREADPARAM;
        m_pstConnectThreadParam->pIOCP_ = &m_IOCP;
        m_pstConnectThreadParam->sctConnectSocket_ = m_sctConnect;
        m_pstConnectThreadParam->stServerAddrInfo_ = stServerAddrInfo;
        m_pstConnectThreadParam->phBreakEvent_ = &m_hBreakEvent;
        m_pstConnectThreadParam->pTeleClientDlg_ = this;

        m_pthdConnectThread = new CConnectThread();
        bRet = m_pthdConnectThread->StartThread(m_pstConnectThreadParam);
        if (!bRet)
        {
            break;
        }

        // 结束
        return TRUE;
    } while (FALSE);

    // 异常处理
    if (m_sctConnect != INVALID_SOCKET)
    {
        closesocket(m_sctConnect);
        m_sctConnect = INVALID_SOCKET;
    }

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTeleClientDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTeleClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

afx_msg LRESULT CTeleClientDlg::OnBreakeventsignal(WPARAM wParam, LPARAM lParam)
{
    // 设置中断事件为触发状态
    SetEvent(m_hBreakEvent);
    m_bBreakEventHasSigal = TRUE;
    return 0;
}

void CTeleClientDlg::OnTimer(UINT_PTR nIDEvent)
{
    CString csTmpData;
    BOOL bRet = SendDataUseIOCP(m_pstClientInfo, m_IOCP, csTmpData, PT_HEARTBEAT);

    if (bRet)
    {
        OutputDebugString(_T("投递心跳包成功\r\n"));
    }
    else
    {
        if (!m_bBreakEventHasSigal)
        {
            OutputDebugString(_T("开启中断事件"));
            // 发送消息给窗口, 开启重连事件
            SendMessage(WM_BREAKEVENTSIGNAL, 0, 0);
        }
    }


    CDialog::OnTimer(nIDEvent);
} //! CTeleClientDlg::OnTimer END

void CTeleClientDlg::SetConnectSocket(const SOCKET & sctTarget)
{
    m_sctConnect = sctTarget;
}


afx_msg LRESULT CTeleClientDlg::OnCreatecmdrecvthread(WPARAM wParam, LPARAM lParam)
{
    // 启动CMDRecvThread线程接收数据
    m_bCmdQuit = FALSE;

    do
    {
        m_pthdCmdRecvThread = new CCmdRecvThread;
        if (m_pthdCmdRecvThread == NULL)
        {
#ifdef DEBUG
            OutputDebugString(_T("创建CMDRecvThread线程失败\r\n"));
#endif // DEBUG
            break;
        }

        m_pthdCmdRecvThread->StartThread(this);
    } while (FALSE);

    return m_pthdCmdRecvThread != NULL;
}

// Send data read from pipe to server.
BOOL CTeleClientDlg::ReadDataFromCmd()
{
#ifdef DEBUG
    DWORD dwError = 0;
    CString csErrorMessage;
#endif // DEBUG

    BOOL bRet = FALSE;
    DWORD dwWritedBytes = 0;
    DWORD dwBytesRead = 0;
    DWORD dwTotalBytesAvail = 0;
    DWORD dwBytesLeftThisMessage = 0;
    DWORD dwReadBytes = 0;
    DWORD dwReadFlag = 0;

    do
    {
        if (m_pstClientInfo == NULL)
        {
#ifdef DEBUG
            OutputDebugStringWithInfo(
                _T("The client info is null. "
                   "Maybe it had disconnected."),
                __FILET__,
                __LINE__);
#endif // DEBUG
            m_bCmdQuit = TRUE;
            break;
        }

        // View has data in pipe or not.
        bRet = PeekNamedPipe(m_pstClientInfo->hServerCmdReadPipe_,
                             NULL, 0,
                             &dwBytesRead,
                             &dwTotalBytesAvail,
                             &dwBytesLeftThisMessage);
        if (!bRet)
        {
#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
        }

        // Has data in pipe.
        if (dwTotalBytesAvail > 0)
        {
            PPACKETFORMAT pstPacket =
                (PPACKETFORMAT)m_pstClientInfo->szSendTmpBuffer_;

            // Read data from pipe.
            bRet = ReadFile(m_pstClientInfo->hServerCmdReadPipe_,
                            pstPacket->szContent_,
                            PACKET_CONTENT_MAXSIZE - 1,
                            &dwReadBytes,
                            NULL);
            if (!bRet)
            {
#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
                // Jump out.
                break;
            }

            // Send data to server.
            pstPacket->dwSize_ = dwReadBytes;
            pstPacket->ePacketType_ = PT_CMD_REPLY;
            
            m_pstClientInfo->CriticalSection_.Lock();
            m_pstClientInfo->SendBuffer_.Write(
                (PBYTE)m_pstClientInfo->szSendTmpBuffer_,
                PACKET_HEADER_SIZE + pstPacket->dwSize_);
            
            memset(m_pstClientInfo->szSendTmpBuffer_,
                   0,
                   PACKET_HEADER_SIZE + pstPacket->dwSize_);

            bRet = m_IOCP.PostSendRequst(m_pstClientInfo->sctClientSocket_,
                                         m_pstClientInfo->SendBuffer_);

            m_pstClientInfo->SendBuffer_.ClearBuffer();
            m_pstClientInfo->CriticalSection_.Unlock();
        } //! if "Has data in pipe" END
    } while (FALSE);

    return bRet;
} //! CTeleClientDlg::ReadDataFromCmd END

// 将服务端发送过来的指令发给CMD
afx_msg LRESULT CTeleClientDlg::OnSendordertocmd(WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
    DWORD dwError = 0;
    CString csErrorMessage;
#endif // DEBUG
    CString *pcsOrder = (CString *)wParam;

    // Change command string to single character.
    USES_CONVERSION;
    char *pszOrder = T2A((*pcsOrder).GetString());
    BOOL bRet = FALSE;
    DWORD dwWritedBytes = 0;
    DWORD dwBytesRead = 0;
    DWORD dwTotalBytesAvail = 0;
    DWORD dwBytesLeftThisMessage = 0;

    do
    {
        if (pcsOrder == NULL)
        {
#ifdef DEBUG
            OutputDebugStringWithInfo( _T("The cmd order is null."),
                                      __FILET__,
                                      __LINE__);
#endif // DEBUG
            break;
        }

        bRet = WriteFile(m_pstClientInfo->hServerCmdWritePipe_,
                         pszOrder,
                         (*pcsOrder).GetLength(),
                         &dwWritedBytes,
                         NULL);
        if (!bRet)
        {
#ifdef DEBUG
#endif // DEBUG
            break;
        }
    } while (FALSE);

    // The receive thread will quit and close handle if order is 'exit'.
    if (*pcsOrder == _T("exit\r\n"))
    {
        m_bCmdQuit = TRUE;
    }

    return bRet;
}


afx_msg LRESULT CTeleClientDlg::OnGetfile(WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
    DWORD dwError = 0;
    CString csErrorMessage;
#endif // DEBUG

    BOOL bRet = FALSE;
    CString *pcsFileListToGet = (CString *)wParam;
    PCLIENTINFO pstClientInfo = (PCLIENTINFO)lParam;
    PFILETRANSPORTTHREADPARAM pstFileTransportThreadParam = NULL;

    do
    {
        // Initilized the param of transport thread.
        pstFileTransportThreadParam = new FILETRANSPORTTHREADPARAM;
        if (NULL == pstFileTransportThreadParam)
        {
#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
            break;
        }

        pstFileTransportThreadParam->pcsFileListToGet_ = pcsFileListToGet;
        pstFileTransportThreadParam->pTeleClientDlg_ = this;
        pstFileTransportThreadParam->pIOCP_ = &m_IOCP;
        pstFileTransportThreadParam->pstClientInfo_ = pstClientInfo;

        // Create instance of work thread thransport.
        m_pthdFileTransportThread = new CFileTransportThread;

        // Thread begin to work.
        bRet = m_pthdFileTransportThread->
            StartThread(pstFileTransportThreadParam);
        if (!bRet)
        {
#ifdef DEBUG
            OutputDebugStringWithInfo(
                _T("The Transport thread start had failed.\r\n"),
                __FILET__,
                __LINE__);
#endif // DEBUG
            break;
        }

        DWORD dwRet = WaitForSingleObject(
            m_pevtGetFileThreadInitializeEvent->m_hObject,
            INFINITE);
        if (WAIT_FAILED == dwRet)
        {
#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
            break;
        }

    } while (FALSE);

    // Free the alloc memory of transport thread's parament.
    if (NULL != pstFileTransportThreadParam)
    {
        delete pstFileTransportThreadParam;
        pstFileTransportThreadParam = NULL;
    }

    return FALSE;
} //! CTeleClientDlg::OnGetfile END


void CTeleClientDlg::OnDestroy()
{
    if (NULL != m_pevtGetFileThreadInitializeEvent)
    {
        delete m_pevtGetFileThreadInitializeEvent;
        m_pevtGetFileThreadInitializeEvent = NULL;
    }

    CDialog::OnDestroy();
}
