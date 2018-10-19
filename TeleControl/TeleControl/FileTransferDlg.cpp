// FileTransferDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TeleControl.h"
#include "FileTransferDlg.h"
#include "afxdialogex.h"
#include "FileTransferStruct.h"


// CFileTransferDlg 对话框

IMPLEMENT_DYNAMIC(CFileTransferDlg, CDialogEx)

CFileTransferDlg::CFileTransferDlg(CString &ref_csIPAndPort,
                                   PCLIENTINFO pstClientInfo,
                                   CCommunicationIOCP &ref_IOCP,
                                   CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FILETRANSFER, pParent)
    , m_ref_IOCP(ref_IOCP)
    , m_ref_csIPAndPort(ref_csIPAndPort)
{
    m_pstClientInfo = pstClientInfo;
    m_hGetTargetDeviceEvent = CreateEvent(NULL,
                                          FALSE,
                                          FALSE,
                                          NULL);
    if (m_hGetTargetDeviceEvent == NULL)
    {
        OutputDebugString(_T("获取盘符完成事件创建失败"));
    }

    m_hGetTargetFileListEvent = CreateEvent(NULL,
                                            FALSE,
                                            FALSE,
                                            NULL);
    if (m_hGetTargetFileListEvent == NULL)
    {
        OutputDebugString(_T("获取文件列表事件创建失败"));
    }
}

CFileTransferDlg::~CFileTransferDlg()
{
    if (m_hGetTargetDeviceEvent != NULL)
    {
        CloseHandle(m_hGetTargetDeviceEvent);
    }

    if (m_hGetTargetFileListEvent != NULL)
    {
        CloseHandle(m_hGetTargetFileListEvent);
    }
}

void CFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CMB_SERVER_DEVICE, m_cmbServerDevice);
    DDX_Control(pDX, IDC_EDT_SERVER_FILEPATH, m_edtServerFilePath);
    DDX_Control(pDX, IDC_CMB_SERVER_FILELIST_STYLE, m_cmbServerFileListStyle);
    DDX_Control(pDX, IDC_CMB_TARGETHOST_FILELIST_STYLE, m_cmbTargetHostFileListStyle);
    DDX_Control(pDX, IDC_LST_SERVER_FILELIST, m_lstServerFileList);
    DDX_Control(pDX, IDC_LST_TARGETHOST_FILELIST, m_lstTargetHostFileList);
    DDX_Control(pDX, IDC_CMB_TARGETHOST_DEVICE, m_cmbTargetHostDevice);
    DDX_Control(pDX, IDC_LST_TRANSFERTASK, m_lstTransferTask);
    DDX_Control(pDX, IDC_EDT_TARGETHOST_FILEPATH, m_edtTargetHostFilePath);
}


BEGIN_MESSAGE_MAP(CFileTransferDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SERVER_SKIP, &CFileTransferDlg::OnBnClickedBtnServerSkip)
    ON_NOTIFY(NM_DBLCLK, IDC_LST_SERVER_FILELIST, &CFileTransferDlg::OnNMDblclkLstServerFilelist)
    ON_CBN_SELCHANGE(IDC_CMB_SERVER_FILELIST_STYLE, &CFileTransferDlg::OnCbnSelchangeCmbServerFilelistStyle)
    ON_CBN_SELCHANGE(IDC_CMB_SERVER_DEVICE, &CFileTransferDlg::OnCbnSelchangeCmbServerDevice)
    ON_CBN_SELCHANGE(IDC_CMB_TARGETHOST_FILELIST_STYLE, &CFileTransferDlg::OnCbnSelchangeCmbTargethostFilelistStyle)
    ON_CBN_SELCHANGE(IDC_CMB_TARGETHOST_DEVICE, &CFileTransferDlg::OnCbnSelchangeCmbTargethostDevice)
    ON_BN_CLICKED(IDC_BTN_TARGETHOST_SKIP, &CFileTransferDlg::OnBnClickedBtnTargethostSkip)
    ON_BN_CLICKED(IDC_BTN_GETFILE, &CFileTransferDlg::OnBnClickedBtnGetfile)
    ON_BN_CLICKED(IDC_BTN_PUTFILE, &CFileTransferDlg::OnBnClickedBtnPutfile)
    ON_NOTIFY(NM_DBLCLK, IDC_LST_TARGETHOST_FILELIST, &CFileTransferDlg::OnNMDblclkLstTargethostFilelist)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CFileTransferDlg 消息处理程序


void CFileTransferDlg::OnBnClickedBtnServerSkip()
{
    if (IsDirectory(m_cmbServerDevice,
                    m_edtServerFilePath))
    {
        ShowFileList(m_lstServerFileList,
                     m_cmbServerDevice,
                     m_edtServerFilePath,
                     m_iServerActiveStyleIdx);
    }
}


BOOL CFileTransferDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    // 设置标题
    CString csTitle = _T("文件传输 - ");
    csTitle += m_ref_csIPAndPort;
    SetWindowText(csTitle);

    // 获取图标链
    SHFILEINFO stSfi;
    HIMAGELIST hImageListSmall = 
        (HIMAGELIST)SHGetFileInfo(_T(""), 0,
                                  &stSfi, sizeof(stSfi),
                                  SHGFI_SMALLICON | SHGFI_SYSICONINDEX);

    m_pSysSmallIconImageList = CImageList::FromHandle(hImageListSmall);

    // 普通图片规格
    HIMAGELIST *phNormalIconImageList = new HIMAGELIST;
    Shell_GetImageLists(phNormalIconImageList, NULL);
    m_pSysBigIconImageList = CImageList::FromHandle(*phNormalIconImageList);

    // 绑定本机文件列表
    m_lstServerFileList.SetImageList(m_pSysBigIconImageList,
                               LVSIL_NORMAL);
    m_lstServerFileList.SetImageList(m_pSysSmallIconImageList,
                               LVSIL_SMALL);
    m_lstServerFileList.SetImageList(m_pSysSmallIconImageList,
                               LVSIL_STATE);

    // 绑定目标文件列表
    m_lstTargetHostFileList.SetImageList(m_pSysBigIconImageList,
                                         LVSIL_NORMAL);
    m_lstTargetHostFileList.SetImageList(m_pSysSmallIconImageList,
                                         LVSIL_SMALL);
    m_lstTargetHostFileList.SetImageList(m_pSysSmallIconImageList,
                                         LVSIL_STATE);

    delete phNormalIconImageList;

    // ***************Server**************
    // Set Table title.
    int iIdx = 0;
    m_lstServerFileList.InsertColumn(iIdx++, _T("名称"), LVCFMT_LEFT, 150);
    m_lstServerFileList.InsertColumn(iIdx++, _T("修改日期"), LVCFMT_LEFT, 70);
    m_lstServerFileList.InsertColumn(iIdx++, _T("类型"), LVCFMT_LEFT, 70);
    m_lstServerFileList.InsertColumn(iIdx++, _T("大小"), LVCFMT_LEFT, 70);

    // Initialize the combox that select style.
    iIdx = 0;
    // ***********************************
    // *Alarm* The order have to same with 
    //         elements of tagFileListStyle.
    // ***********************************
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("普通图标"));
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("小图标"));
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("列表"));
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("详细信息"));

    m_cmbServerFileListStyle.SetCurSel(3);
    m_iServerActiveStyleIdx = 3;
    ChangeListStyle(m_lstServerFileList,
                    m_iServerActiveStyleIdx,
                    -1,
                    m_iServerActiveStyleIdx);

    // Get Driver
    DWORD dwRet = 
        GetLogicalDriveStrings(MAXBYTE - 1,
                               m_csDevice.GetBufferSetLength(MAXBYTE - 1));
    m_csDevice.ReleaseBuffer();
    if (dwRet == 0)
    {
        OutputDebugString(_T("获取盘符名称失败"));
    }
    else if (dwRet > MAXBYTE)
    {
        CString csFailInfo;
        csFailInfo.Format(_T("盘符名称存储缓冲区过小, 需要字节数：%u"), dwRet);
        OutputDebugString(csFailInfo);
    }

    // 分割盘符
    CString csDriveName(_T(""));
    DWORD cntDriveNum = 0;
    TCHAR *pcTmpChar = m_csDevice.GetBuffer();
    for (size_t cntI = 0; cntI < dwRet; cntI++)
    {
        if (pcTmpChar[cntI] != _T('\0'))
        {
            csDriveName += pcTmpChar[cntI];
        }
        else
        {
            if (csDriveName != _T(""))
            {
                m_cmbServerDevice.InsertString(cntDriveNum, csDriveName);
                ++cntDriveNum;
                csDriveName.Empty();
            }
        }
    }

    m_cmbServerDevice.SetCurSel(0);

    // Show files of server client.
    ShowFileList(m_lstServerFileList,
                 m_cmbServerDevice,
                 m_edtServerFilePath,
                 m_iServerActiveStyleIdx);

    // ******************Target******************
    // Set title of table.
    iIdx = 0;
    m_lstTargetHostFileList.InsertColumn(iIdx++, _T("名称"), LVCFMT_LEFT, 150);
    m_lstTargetHostFileList.InsertColumn(iIdx++, _T("修改日期"), LVCFMT_LEFT, 70);
    m_lstTargetHostFileList.InsertColumn(iIdx++, _T("类型"), LVCFMT_LEFT, 70);
    m_lstTargetHostFileList.InsertColumn(iIdx++, _T("大小"), LVCFMT_LEFT, 70);

    // Initialization style selection.
    iIdx = 0;
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("普通图标"));
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("小图标"));
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("列表"));
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("详细信息"));

    m_cmbTargetHostFileListStyle.SetCurSel(3);
    m_iTargetHostActiveStyleIdx = 3;
    ChangeListStyle(m_lstServerFileList,
                    m_iTargetHostActiveStyleIdx,
                    -1,
                    m_iTargetHostActiveStyleIdx);

    // Get driver.
    PPACKETFORMAT pstPacket =
        (PPACKETFORMAT)m_pstClientInfo->szSendTmpBuffer_;

    m_pstClientInfo->CriticalSection_.Lock();
    pstPacket->ePacketType_ = PT_FILE_DEVICE;
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

    
    // 等待后盘符获取完毕事件触发
    WaitForSingleObject(m_hGetTargetDeviceEvent, INFINITE);

    // 分割盘符
    csDriveName = _T("");
    cntDriveNum = 0;
    pcTmpChar = m_csTargetHostDevice.GetBuffer();
    for (size_t cntI = 0;
         cntI < (m_uiTargetHostDeviceLen / sizeof(TCHAR));
         cntI++)
    {
        if (pcTmpChar[cntI] != _T('\0'))
        {
            csDriveName += pcTmpChar[cntI];
        }
        else
        {
            if (csDriveName != _T(""))
            {
                m_cmbTargetHostDevice.InsertString(cntDriveNum,
                                                   csDriveName);
                ++cntDriveNum;
                csDriveName.Empty();
            }
        }
    }
    m_cmbTargetHostDevice.SetCurSel(0);


    // Show filelist of target host.
    ShowFileList(m_lstTargetHostFileList,
                 m_cmbTargetHostDevice,
                 m_edtTargetHostFilePath,
                 m_iTargetHostActiveStyleIdx);


    //******************Transmission list******************
    // Set table title.
    iIdx = 0;
    m_lstTransferTask.InsertColumn(iIdx++, _T("任务类型"), LVCFMT_LEFT, 150);
    m_lstTransferTask.InsertColumn(iIdx++, _T("文件名称"), LVCFMT_LEFT, 70);
    m_lstTransferTask.InsertColumn(iIdx++, _T("总大小"), LVCFMT_LEFT, 70);
    m_lstTransferTask.InsertColumn(iIdx++, _T("已传输大小"), LVCFMT_LEFT, 70);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
} //! CFileTransferDlg::OnInitDialog END

void CFileTransferDlg::ShowFileList(CListCtrl &ref_lstTarget,
                                    CComboBox &ref_cmbDevice,
                                    CEdit &ref_edtFilePath,
                                    const int &ref_iActiveStyleIdx)
{
    // Clean all items in list.
    ref_lstTarget.DeleteAllItems();

    // Get driver.
    CString csTargetDrive;
    int iIndex = ref_cmbDevice.GetCurSel();
    if (iIndex < 0)
    {
        OutputDebugString(_T("盘符下拉框未选择\r\n"));
        return;
    }
    ref_cmbDevice.GetLBText(iIndex, csTargetDrive);
   
    // Get subpath.
    CString csTargetPath;
    ref_edtFilePath.GetWindowText(csTargetPath);

    // Get full path.
    CString csCompeletPath = csTargetDrive + csTargetPath;

    // Target host.
    if (ref_lstTarget == m_lstTargetHostFileList)
    {
        // 投递请求消息获取目标路径的文件
        PPACKETFORMAT pstPacket = 
            (PPACKETFORMAT)m_pstClientInfo->szSendTmpBuffer_;

        // *注意* 写入数据时要加锁
        m_pstClientInfo->CriticalSection_.Lock();
        pstPacket->ePacketType_ = PT_FILE_LIST;
        pstPacket->dwSize_ = 
            (csCompeletPath.GetLength() + 1) * sizeof(TCHAR);

        memmove(pstPacket->szContent_,
                csCompeletPath.GetBuffer(),
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

        WaitForSingleObject(m_hGetTargetFileListEvent, INFINITE);

        int iIdx = 0;
        std::basic_string<TCHAR> strFileList = m_csFileList.GetString();
        // 遍历插入文件列表
        while (!strFileList.empty())
        {
            // First is index of Icon.
            // Secound is Filename.
            // Third is write time of file.
            // Forth is size of file.
            // Fifth is type of file.
            std::match_results<const TCHAR*> sMatchResult;
            std::basic_regex<TCHAR> rgxFileInfoMode(
                _T("([^\\?]*)\\?([^\\?]*)\\?([^\\?]*)\\?([^\\?]*)\\?"
                   "([^|\\?]*)\\|"));

            bool bFound = std::regex_search<TCHAR>(strFileList.c_str(),
                                                   sMatchResult,
                                                   rgxFileInfoMode);

            if (bFound)
            {
                std::basic_string<TCHAR> strIconIndex = sMatchResult[1];
                std::basic_string<TCHAR> strFileName = sMatchResult[2];
                std::basic_string<TCHAR> strFileWriteTime = sMatchResult[3];
                std::basic_string<TCHAR> strFileSize = sMatchResult[4];
                std::basic_string<TCHAR> strFileType = sMatchResult[5];

                // Insert file info.
                int iIconIndex = _ttoi(strIconIndex.c_str());
                // File name.
                m_lstTargetHostFileList.InsertItem(
                    iIdx,
                    strFileName.c_str(),
                    iIconIndex);

                // File type.
                m_lstTargetHostFileList.SetItemText(iIdx,
                                                    FLCT_FILETYPE,
                                                    strFileType.c_str());

                // File size. Filesize is null when it's "0".
                if (_T("0") == strFileSize)
                {
                    strFileSize = _T("");
                }
                m_lstTargetHostFileList.SetItemText(iIdx,
                                                    FLCT_FILESIZE,
                                                    strFileSize.c_str());

                // Add time if filelist's style is report. 
                if (m_iTargetHostActiveStyleIdx == FLS_REPORT)
                {
                    m_lstTargetHostFileList.SetItemText(
                        iIdx,
                        FLCT_WRITETIME,
                        strFileWriteTime.c_str());
                }
                // 插入的位置索引递增
                ++iIdx;

                // Get remaind text.
                strFileList = sMatchResult.suffix();
            }
            else
            {
                OutputDebugString(_T("正则表达式有误"));
            }
        } //! while (Traversing and insert filelist) END
    } // if (Target host) END
    // Local host
    else if (ref_lstTarget == m_lstServerFileList)
    {
        CFileFind Finder;
        CString csWildcard(csCompeletPath + _T("\\*.*"));

        BOOL bWorking = Finder.FindFile(csWildcard);
        int iIdx = 0;
        // 遍历文件列表
        while (bWorking)
        {
            bWorking = Finder.FindNextFile();
            CString csFilePath = Finder.GetFilePath();

            // 获取文件属性
            DWORD dwFileAttribute = GetFileAttributes(csFilePath);

            // 获取文件对应的系统图标索引
            SHFILEINFO stSfi;
            SHGetFileInfo(csFilePath,
                          dwFileAttribute,
                          &stSfi,
                          sizeof(stSfi),
                          SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

            ref_lstTarget.InsertItem(iIdx,
                                     Finder.GetFileName(),
                                     stSfi.iIcon);

            CFileStatus FileStatus;
            CFile::GetStatus(csFilePath, FileStatus);

            if (ref_iActiveStyleIdx == 3)
            {
                ref_lstTarget.SetItemText(
                    iIdx,
                    FLCT_WRITETIME,
                    FileStatus.m_mtime.Format(_T("%Y/%m/%d %H:%M")));
            }

            ++iIdx;
        } //! while 遍历文件列表 END

        Finder.Close();
    } // if (Local host) END

    UpdateData();
} // CFileTransferDlg::ShowFileList END

void CFileTransferDlg::ChangeListStyle(CListCtrl &ref_lstTarget,
                                       int &ref_iActiveStyleIdx,
                                       int iOldIndex,
                                       int iNewIndex)
{
    if (iOldIndex == -1 && iNewIndex == -1)
    {
        return;
    }

    DWORD adwStyle[4] = {
        LVS_ICON,
        LVS_SMALLICON,
        LVS_LIST,
        LVS_REPORT,
    };

    // 修改列表控件风格
    DWORD dwAddStyle = adwStyle[iNewIndex];
    DWORD dwRemoveStyle = 0;
    if (iOldIndex != -1)
    {
        dwRemoveStyle = adwStyle[iOldIndex];
    }

    ref_lstTarget.ModifyStyle(dwRemoveStyle, dwAddStyle, TRUE);
    ref_iActiveStyleIdx = iNewIndex;
}

CString CFileTransferDlg::GetSubName(CComboBox &ref_cmbDevice,
                                     CEdit &ref_edtFilePath,
                                     CString &ref_csFilename)
{
    CString csDrive;
    CString csSubPath;
    
    // Get Driver.
    int iIndex = ref_cmbDevice.GetCurSel();
    ref_cmbDevice.GetLBText(iIndex, csDrive);

    // Get subpath.
    ref_edtFilePath.GetWindowText(csSubPath);

    // Add '\' if the lastest character isn't '\'.
    if (csSubPath.Right(1) != _T("\\") &&
        !csSubPath.IsEmpty())
    {
        csSubPath += _T("\\");
    }

    ref_csFilename = csSubPath + ref_csFilename;

    return csDrive;
}

void CFileTransferDlg::BackParentDirctory(CComboBox &ref_cmbDevice,
                                          CEdit &ref_edtFilePath)
{
    CString csDrive;
    CString csSubPath;
    // Get Driver.
    int iIndex = ref_cmbDevice.GetCurSel();
    ref_cmbDevice.GetLBText(iIndex, csDrive);

    // Get subpath.
    ref_edtFilePath.GetWindowText(csSubPath);

    // *注意* FindFile要执行成功，需要保证后面没有反斜杠
    CString csCompleteFileName = csDrive + csSubPath;
    if (csCompleteFileName.Right(1) == _T("\\"))
    {
        csCompleteFileName =
            csCompleteFileName.Left(csCompleteFileName.GetLength() - 1);
    }

    CFileFind TargetFind;
    BOOL bRet = TargetFind.FindFile(csCompleteFileName);
    CString csParent;

    if (bRet)
    {
        TargetFind.FindNextFile();
        csParent = TargetFind.GetRoot();
    }

    if (csParent.Right(1) == _T("\\"))
    {
        csParent =
            csParent.Left(csParent.GetLength() - 1);
    }

    csSubPath = csParent.Right(csParent.GetLength() - 3);

    ref_edtFilePath.SetWindowText(csSubPath);

    if (ref_cmbDevice == m_cmbServerDevice)
    {
        OnBnClickedBtnServerSkip();
    }
    else
    {
        OnBnClickedBtnTargethostSkip();
    }
}

BOOL CFileTransferDlg::IsDirectory(CComboBox &ref_cmbDevice,
                                   CEdit &ref_edtFilePath,
                                   const CString *TargetFile /*= NULL*/)
{
    // Get full path.
    CString csFileSubPath;
    CString csFileCompleteName;
    CString csDrive;
    
    int iIndex = ref_cmbDevice.GetCurSel();
    ref_cmbDevice.GetLBText(iIndex, csDrive);

    ref_edtFilePath.GetWindowText(csFileSubPath);

    CString csTargetFileCompleteName = csDrive + csFileSubPath;
    if (TargetFile != NULL)
    {
        if (csFileSubPath != _T(""))
        {
            csTargetFileCompleteName += _T("\\");
        }
        csTargetFileCompleteName += *TargetFile;
    }

    // Get attributes of file.
    DWORD dwFileAttribute = GetFileAttributes(csTargetFileCompleteName);

    return (dwFileAttribute & FILE_ATTRIBUTE_DIRECTORY);
}

void CFileTransferDlg::OnNMDblclkLstServerFilelist(NMHDR *pNMHDR, 
                                                   LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = 
        reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    int iItem = pNMItemActivate->iItem;
    if (iItem >= 0)
    {
        CString csTargetFileTitle;
        csTargetFileTitle = m_lstServerFileList.GetItemText(iItem, 0); 
        
        // If it is '..', go to parent direcotry.
        if (csTargetFileTitle == _T(".."))
        {
            BackParentDirctory(m_cmbServerDevice, 
                               m_edtServerFilePath);
            return;
        }
        else if (csTargetFileTitle == _T("."))
        {
            // If it is '.', quit.
            return;
        }

        // 拼接子名字
        // 
        CString csTargetFileSubName = csTargetFileTitle;
        GetSubName(m_cmbServerDevice,
                   m_edtServerFilePath,
                   csTargetFileSubName);

        if (IsDirectory(m_cmbServerDevice,
                        m_edtServerFilePath,
                        &csTargetFileTitle))
        {
            // Update text of title.
            m_edtServerFilePath.SetWindowText(csTargetFileSubName);
            ShowFileList(m_lstServerFileList,
                         m_cmbServerDevice,
                         m_edtServerFilePath,
                         m_iServerActiveStyleIdx);
        }
    }

    *pResult = 0;
}


void CFileTransferDlg::OnCbnSelchangeCmbServerFilelistStyle()
{
    int iIndex = m_cmbServerFileListStyle.GetCurSel();
    ChangeListStyle(m_lstServerFileList,
                    m_iServerActiveStyleIdx,
                    m_iServerActiveStyleIdx,
                    iIndex);
}


// Deal with show filelist when ComboBox is change
// that device of local host.
void CFileTransferDlg::OnCbnSelchangeCmbServerDevice()
{
    // Clean edit of path.
    m_edtServerFilePath.SetWindowText(_T(""));

    // Show file list.
    ShowFileList(m_lstServerFileList,
                 m_cmbServerDevice,
                 m_edtServerFilePath,
                 m_iServerActiveStyleIdx);
} //! CFileTransferDlg::OnCbnSelchangeCmbServerDevice End

// Deal with
void CFileTransferDlg::OnCbnSelchangeCmbTargethostFilelistStyle()
{
    // TODO: 在此添加控件通知处理程序代码
    
}

// Deal with show filelist when ComboBox is change
// that device of target host.
void CFileTransferDlg::OnCbnSelchangeCmbTargethostDevice()
{
    m_edtTargetHostFilePath.SetWindowText(_T(""));

    ShowFileList(m_lstTargetHostFileList,
                 m_cmbTargetHostDevice,
                 m_edtTargetHostFilePath,
                 m_iTargetHostActiveStyleIdx);
}

// Deal with click the 'go' button of target host.
void CFileTransferDlg::OnBnClickedBtnTargethostSkip()
{
    if (IsDirectory(m_cmbTargetHostDevice,
                    m_edtTargetHostFilePath))
    {
        ShowFileList(m_lstTargetHostFileList,
                     m_cmbTargetHostDevice,
                     m_edtTargetHostFilePath,
                     m_iTargetHostActiveStyleIdx);
    }
} //! CFileTransferDlg::OnBnClickedBtnTargethostSkip END


// Deal with click the '<' button.
void CFileTransferDlg::OnBnClickedBtnGetfile()
{
    // Format: "FilePath?FileName|FileName|..."
    CString csFilesListSendToTargetHost;

    // Get name of files need to downlord, and add mission to task manager.
    POSITION posI = m_lstTargetHostFileList.GetFirstSelectedItemPosition();
    if (NULL == posI)
    {
        TRACE(_T("No items were selected!\n"));
    }
    else
    {
        CString csFileName;
        CString csFileSize;
        CString csFileFullName;

        // Get full path.
        CString csFileSubPath;
        CString csDriveLetter;
    
        int iIndex = m_cmbTargetHostDevice.GetCurSel();
        m_cmbTargetHostDevice.GetLBText(iIndex, csDriveLetter);

        m_edtTargetHostFilePath.GetWindowText(csFileSubPath);

        csFilesListSendToTargetHost = csDriveLetter + csFileSubPath + _T("?");

        // Traversing Item.
        while (posI)
        {
            int iItemIndex = m_lstTargetHostFileList.GetNextSelectedItem(posI);

            csFileName =
                m_lstTargetHostFileList.GetItemText(iItemIndex, FLCT_FILENAME);
            csFileSize = 
                m_lstTargetHostFileList.GetItemText(iItemIndex, FLCT_FILESIZE);

            csFileFullName = csDriveLetter + csFileSubPath + csFileName;

            // Add to the send list.
            csFilesListSendToTargetHost += csFileName + _T(":0") + _T("|");

            //*************************************
            //*ALARM* This memory will free when manager distroy.
            //*************************************
            PFILETRANSPORTTASK pstTaskInfo = new FILETRANSPORTTASK;
            pstTaskInfo->csFileFullPath_ = csFileFullName;
            pstTaskInfo->eTaskType_ = FTT_GETFILE;
            pstTaskInfo->uint64FileTotalSize_ = _ttoi(csFileSize);
            pstTaskInfo->uint64TransmissionSize_ = 0;
            pstTaskInfo->eTaskState_ = FTS_START;

            // Add task to manager.
            m_TransportTaskManager.InsertGetFileTask(csFileName, pstTaskInfo);
        } // while "Traversing Item" END
    } //! if "Item is selected" END

    // Send file command and file list to target host.
    SendDataUseIOCP(m_pstClientInfo,
                    m_ref_IOCP,
                    csFilesListSendToTargetHost,
                    PT_FILECOMMAND_GETFILE);

} //! CFileTransferDlg::OnBnClickedBtnGetfile END


void CFileTransferDlg::OnBnClickedBtnPutfile()
{
    // TODO: 在此添加控件通知处理程序代码
}




// Deal with double clicks in file list of target host.
void CFileTransferDlg::OnNMDblclkLstTargethostFilelist(NMHDR *pNMHDR, 
                                                       LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = 
        reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    int iItem = pNMItemActivate->iItem;
    if (iItem >= 0)
    {
        CString csTargetFileTitle;
        csTargetFileTitle = m_lstTargetHostFileList.GetItemText(iItem, 0); 
        
        // If it is '..', go to parent direcotry.
        if (csTargetFileTitle == _T(".."))
        {
            BackParentDirctory(m_cmbTargetHostDevice, 
                               m_edtTargetHostFilePath);
            return;
        }
        else if (csTargetFileTitle == _T("."))
        {
            // If it is '.', quit.
            return;
        }

        // 拼接子名字
        CString csTargetFileSubName = csTargetFileTitle;
        GetSubName(m_cmbTargetHostDevice,
                   m_edtTargetHostFilePath,
                   csTargetFileSubName);

        if (IsDirectory(m_cmbTargetHostDevice,
                        m_edtTargetHostFilePath,
                        &csTargetFileTitle))
        {
            // Modify the text of title.
            m_edtTargetHostFilePath.SetWindowText(csTargetFileSubName);
            ShowFileList(m_lstTargetHostFileList,
                         m_cmbTargetHostDevice,
                         m_edtTargetHostFilePath,
                         m_iTargetHostActiveStyleIdx);
        }
    }

    *pResult = 0;
} //! CFileTransferDlg::OnNMDblclkLstTargethostFilelist END

void CFileTransferDlg::OnClose()
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    CDialogEx::OnClose();
}
