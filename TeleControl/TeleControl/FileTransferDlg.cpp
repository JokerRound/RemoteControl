//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      Achieve of class CFileTransferdlg's member method.
//
// Modify Log:
//      2018-11-10    Hoffman
//      Info: Modify achieve of below methods.
//              UpdateTransportList(): 
//                  1. Add DstFile and SrcFile columns.
//              OnNMDblclkLstServerFilelist(): 
//                  1. Optimization for deal whith path.
//              OnFiledlgupdate(): 
//                  1. Modify flush list control ability.
//
//      2018-11-13    Hoffman
//      Info: Midify achieve of below methods.
//              OnBnClickedBtnGetfile(): 
//                  1. Initional the destination file name.
//
//      2018-11-14    Hoffman
//      Info: Midify achieve of below methods.
//              OnFiledlgupdate(): 
//                  1. Modify deal with it had occured error.
//                  2. Add deal with for FDUT_ERROR
//              OnBnClickedBtnGetfile(): 
//                  1. Modify deal with get uniq file name.
//******************************************************************************

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
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
#endif // DEBUG

    m_pstClientInfo = pstClientInfo;
    m_hGetTargetDeviceEvent = CreateEvent(NULL,
                                          FALSE,
                                          FALSE,
                                          NULL);
    if (NULL == m_hGetTargetDeviceEvent)
    {
        OutputDebugString(_T("获取盘符完成事件创建失败"));
    }

    m_hGetTargetFileListEvent = CreateEvent(NULL,
                                            FALSE,
                                            FALSE,
                                            NULL);
    if (NULL == m_hGetTargetFileListEvent)
    {
        OutputDebugString(_T("获取文件列表事件创建失败"));
    }

    m_pevtHadFiletoReceive = new CEvent(FALSE, FALSE);
    if (NULL == m_pevtHadFiletoReceive)
    {
#ifdef DEBUG
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
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

void CFileTransferDlg::FreeResource()
{
    m_bProcessQuit = TRUE;

    if (NULL != m_pevtHadFiletoReceive)
    {
        m_pevtHadFiletoReceive->SetEvent();

        delete m_pevtHadFiletoReceive;
        m_pevtHadFiletoReceive = NULL;
    }

    if (NULL != m_pthdRecvFileData)
    {
        delete m_pthdRecvFileData;
        m_pthdRecvFileData = NULL;
    }
}

void CFileTransferDlg::UpdateTransportList()
{
    std::vector<PFILETRANSPORTTASK> vctAllTask;
    m_TransportTaskManager.GetAllValue(vctAllTask);

    // Insert task info.
    do
    {
        if (vctAllTask.empty())
        {
#ifdef DEBUG
            OutputDebugStringWithInfo(_T("No task in mamanger.\r\n"),
                                      __FILET__,
                                      __LINE__);
#endif // DEBUG
            break;
        }

        int iIdx = 0;
        // Insert task to list.
        for (const PFILETRANSPORTTASK pstTask : vctAllTask)
        {

            m_lstTransferTaskList.InsertItem(iIdx,
                                             pstTask->phFileNameWithPathDst_);
            m_lstTransferTaskList.SetItemText(iIdx,
                                              FTLC_SRCFILE,
                                              pstTask->phFileNameWithPathSrc_);
            // Insert task type.
            m_lstTransferTaskList.SetItemText(
                iIdx,
                FTLC_TASKSTATUS,
                m_acsTaskType[pstTask->eTaskType_]);

            // Insert total size of file.
            CString csTotalSize;
            _ui64tot(pstTask->ullFileTotalSize_,
                     csTotalSize.GetBufferSetLength(MAXBYTE), 
                     10);
            csTotalSize.ReleaseBuffer();
            m_lstTransferTaskList.SetItemText(iIdx,
                                          FTLC_TOTALSIZE,
                                          csTotalSize);

            // Insert transmitted size.
            CString csTransmittedSize;
            _ui64tot(pstTask->ullTransmissionSize_,
                     csTransmittedSize.GetBufferSetLength(MAXBYTE),
                     10);
            csTransmittedSize.ReleaseBuffer();
            m_lstTransferTaskList.SetItemText(iIdx,
                                          FTLC_TRANSMITTEDSIZE,
                                          csTransmittedSize);

            // Insert task status.
            m_lstTransferTaskList.SetItemText(
                iIdx,
                FTLC_TASKSTATUS,
                m_acsTaskStatus[pstTask->eTaskStatus_]);

            ++iIdx;
        } //! for "Insert task to list" END
    } while (FALSE);  // while "Insert task info" END
} //! CFileTransferDlg::UpdateTransportList END



void CFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CMB_SERVER_DEVICE, m_cmbServerDriver);
    DDX_Control(pDX, IDC_EDT_SERVER_FILEPATH, m_edtServerFilePath);
    DDX_Control(pDX, IDC_CMB_SERVER_FILELIST_STYLE, m_cmbServerFileListStyle);
    DDX_Control(pDX, IDC_CMB_TARGETHOST_FILELIST_STYLE, m_cmbTargetHostFileListStyle);
    DDX_Control(pDX, IDC_LST_SERVER_FILELIST, m_lstServerFileList);
    DDX_Control(pDX, IDC_LST_TARGETHOST_FILELIST, m_lstTargetHostFileList);
    DDX_Control(pDX, IDC_CMB_TARGETHOST_DEVICE, m_cmbTargetHostDriver);
    DDX_Control(pDX, IDC_LST_TRANSFERTASK, m_lstTransferTaskList);
    DDX_Control(pDX, IDC_EDT_TARGETHOST_FILEPATH, m_edtTargetHostFilePath);
}


BEGIN_MESSAGE_MAP(CFileTransferDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SERVER_SKIP, &CFileTransferDlg::OnBnClickedBtnServerSkip)
    ON_NOTIFY(NM_DBLCLK, IDC_LST_SERVER_FILELIST, &CFileTransferDlg::OnNMDblclkLstServerFilelist)
    ON_CBN_SELCHANGE(IDC_CMB_SERVER_FILELIST_STYLE, &CFileTransferDlg::OnCbnSelchangeCmbServerFilelistStyle)
    ON_CBN_SELCHANGE(IDC_CMB_SERVER_DEVICE, &CFileTransferDlg::OnCbnSelchangeCmbServerDriver)
    ON_CBN_SELCHANGE(IDC_CMB_TARGETHOST_FILELIST_STYLE, &CFileTransferDlg::OnCbnSelchangeCmbTargethostFilelistStyle)
    ON_CBN_SELCHANGE(IDC_CMB_TARGETHOST_DEVICE, &CFileTransferDlg::OnCbnSelchangeCmbTargethostDriver)
    ON_BN_CLICKED(IDC_BTN_TARGETHOST_SKIP, &CFileTransferDlg::OnBnClickedBtnTargethostSkip)
    ON_BN_CLICKED(IDC_BTN_GETFILE, &CFileTransferDlg::OnBnClickedBtnGetfile)
    ON_BN_CLICKED(IDC_BTN_PUTFILE, &CFileTransferDlg::OnBnClickedBtnPutfile)
    ON_NOTIFY(NM_DBLCLK, IDC_LST_TARGETHOST_FILELIST, &CFileTransferDlg::OnNMDblclkLstTargethostFilelist)
    ON_WM_CLOSE()
    ON_MESSAGE(WM_HASFILEDATA, &CFileTransferDlg::OnHasfiledata)
    ON_MESSAGE(WM_FILEDLGUPDATE, &CFileTransferDlg::OnFiledlgupdate)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CFileTransferDlg 消息处理程序


void CFileTransferDlg::OnBnClickedBtnServerSkip()
{
    m_edtServerFilePath.GetWindowText(m_pathServerFilePath);

    if (m_pathServerFilePath.IsDirectory())
    {
        ShowFileList(m_lstServerFileList,
                     m_iServerActiveStyleIdx);
    }
} //! CFileTransferDlg::OnBnClickedBtnServerSkip END


BOOL CFileTransferDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
#endif // DEBUG

    // Set title of window.
    CString csTitle = _T("File Transport - ");
    csTitle += m_ref_csIPAndPort;
    SetWindowText(csTitle);

    // Get Icon list small type.
    SHFILEINFO stSfi;
    HIMAGELIST hImageListSmall = 
        (HIMAGELIST)SHGetFileInfo(_T(""), 0,
                                  &stSfi, sizeof(stSfi),
                                  SHGFI_SMALLICON | SHGFI_SYSICONINDEX);

    m_pSysSmallIconImageList = CImageList::FromHandle(hImageListSmall);

    // Get Icon list normal type.
    HIMAGELIST *phNormalIconImageList = new HIMAGELIST;
    Shell_GetImageLists(phNormalIconImageList, NULL);
    m_pSysBigIconImageList = CImageList::FromHandle(*phNormalIconImageList);

    // Bind image list with local host file list.
    m_lstServerFileList.SetImageList(m_pSysBigIconImageList,
                               LVSIL_NORMAL);
    m_lstServerFileList.SetImageList(m_pSysSmallIconImageList,
                               LVSIL_SMALL);
    m_lstServerFileList.SetImageList(m_pSysSmallIconImageList,
                               LVSIL_STATE);

    // Bind image list with target host file list.
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
    m_lstServerFileList.InsertColumn(iIdx++,
                                     _T("File Name"), 
                                     LVCFMT_LEFT, 
                                     200);
    m_lstServerFileList.InsertColumn(iIdx++, 
                                     _T("Modify Time"), 
                                     LVCFMT_LEFT, 
                                     70);
    m_lstServerFileList.InsertColumn(iIdx++, _T("File Type"), LVCFMT_LEFT, 70);
    m_lstServerFileList.InsertColumn(iIdx++, _T("Size"), LVCFMT_LEFT, 70);

    // Initialize the combox that select style.
    iIdx = 0;
    // ***********************************
    // * Alarm * The order have to same with 
    //           elements of tagFileListStyle.
    // ***********************************
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("Normal Icon"));
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("Small Icon"));
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("List"));
    m_cmbServerFileListStyle.InsertString(iIdx++, _T("Report"));

    // Set report list style.
    m_cmbServerFileListStyle.SetCurSel(3);
    m_iServerActiveStyleIdx = 3;
    ChangeListStyle(m_lstServerFileList,
                    m_iServerActiveStyleIdx,
                    -1,
                    m_iServerActiveStyleIdx);

    // Get Driver.
    DWORD dwRet = 
        GetLogicalDriveStrings(MAXWORD,
                               m_csDevice.GetBufferSetLength(MAXWORD));
    m_csDevice.ReleaseBuffer();
    if (dwRet == 0)
    {
#ifdef DEBUG
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
    }
    else if (dwRet > MAXWORD)
    {
        CString csFailInfo;
        csFailInfo.Format(_T("盘符名称存储缓冲区过小, 需要字节数：%u"), dwRet);
        OutputDebugString(csFailInfo);
    }

    // Cut driver.
    CString csDriveName;
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
                m_cmbServerDriver.InsertString(cntDriveNum, csDriveName);
                ++cntDriveNum;
                csDriveName.Empty();
            }
        }
    }

    m_cmbServerDriver.SetCurSel(0);
    m_cmbServerDriver.GetLBText(0, m_pathServerFilePath);
    m_edtServerFilePath.SetWindowText(m_pathServerFilePath);

    // Show files of server client.
    ShowFileList(m_lstServerFileList,
                 m_iServerActiveStyleIdx);

    // ******************Target******************
    // Set title of table.
    iIdx = 0;
    m_lstTargetHostFileList.InsertColumn(iIdx++, 
                                         _T("File Name"), LVCFMT_LEFT, 150);
    m_lstTargetHostFileList.InsertColumn(iIdx++,
                                         _T("Modify Time"),
                                         LVCFMT_LEFT,
                                         70);
    m_lstTargetHostFileList.InsertColumn(iIdx++, 
                                         _T("File Type"),
                                         LVCFMT_LEFT,
                                         70);
    m_lstTargetHostFileList.InsertColumn(iIdx++, _T("Size"), LVCFMT_LEFT, 70);

    // Initialization style selection.
    iIdx = 0;
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("Normal Icon"));
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("Small Icon"));
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("List"));
    m_cmbTargetHostFileListStyle.InsertString(iIdx++, _T("Report"));

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

    
    // Wait the event it gets driver successfully.
    WaitForSingleObject(m_hGetTargetDeviceEvent, INFINITE);

    // Cut driver.
    csDriveName = _T("");
    cntDriveNum = 0;
    pcTmpChar = m_csTargetHostDriverList.GetBuffer();
    for (size_t cntI = 0;
         cntI < (m_uiTargetHostDriverLen / sizeof(TCHAR));
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
                m_cmbTargetHostDriver.InsertString(cntDriveNum,
                                                   csDriveName);
                ++cntDriveNum;
                csDriveName.Empty();
            }
        }
    }
    m_cmbTargetHostDriver.SetCurSel(0);
    m_cmbTargetHostDriver.GetLBText(0, m_pathTartetHostFilePath);
    m_edtTargetHostFilePath.SetWindowText(m_pathTartetHostFilePath);


    // Show filelist of target host.
    ShowFileList(m_lstTargetHostFileList,
                 m_iTargetHostActiveStyleIdx);

    //******************Transmission list******************
    // Set table title.
    iIdx = 0;
    m_lstTransferTaskList.InsertColumn(iIdx++,
                                       _T("Destination File"),
                                       LVCFMT_LEFT,
                                       200);
    m_lstTransferTaskList.InsertColumn(iIdx++,
                                       _T("Source File"),
                                       LVCFMT_LEFT,
                                       200);
    m_lstTransferTaskList.InsertColumn(iIdx++,
                                       _T("Task Type"),
                                       LVCFMT_LEFT,
                                       70);
    m_lstTransferTaskList.InsertColumn(iIdx++,
                                       _T("Total Size"),
                                       LVCFMT_LEFT,
                                       70);
    m_lstTransferTaskList.InsertColumn(iIdx++,
                                       _T("Transmitted Szie"),
                                       LVCFMT_LEFT,
                                       70);
    m_lstTransferTaskList.InsertColumn(iIdx++,
                                       _T("Task Status"),
                                       LVCFMT_LEFT,
                                       70);

    UpdateTransportList();

    //******************Recevie File data thread******************

    // Create thread objet.
    //*****************************************************
    //* Alarm * This memroy will free when dialog destory.
    //*****************************************************
    m_pthdRecvFileData = new CRecvFileDataThread;
    if (NULL == m_pthdRecvFileData)
    {
#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
    }

    bRet = m_pthdRecvFileData->StartThread(this);
    if (!bRet)
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("The thread that receive file data"
                                     " start faild."),
                                  __FILET__,
                                  __LINE__);
#endif // DEBUG
    }


    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
} //! CFileTransferDlg::OnInitDialog END

void CFileTransferDlg::ShowFileList(CListCtrl &ref_lstTarget,
                                    //CComboBox &ref_cmbDevice,
                                    //CEdit &ref_edtFilePath,
                                    const int &ref_iActiveStyleIdx)
{
    // Clean all items in list.
    ref_lstTarget.DeleteAllItems();
   
    // Target host.
    if (ref_lstTarget == m_lstTargetHostFileList)
    {
        SendDataUseIOCP(m_pstClientInfo,
                        m_ref_IOCP,
                        m_pathTartetHostFilePath,
                        PT_FILE_LIST);

        WaitForSingleObject(m_hGetTargetFileListEvent, INFINITE);

        int iIdx = 0;
        std::basic_string<TCHAR> strFileList = m_csTargetHostFileList.GetString();
        // Traversing and insert result into file list.
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
                // Index of insert position increment.
                ++iIdx;

                // Get remaind text.
                strFileList = sMatchResult.suffix();
            }
            else
            {

            }
        } //! while "Traversing and insert filelist" END
    } //! if "Target host" END
    // Local host
    else if (ref_lstTarget == m_lstServerFileList)
    {
        CFileFind Finder;
        CString csWildcard(m_pathServerFilePath + _T("\\*.*"));

        BOOL bWorking = Finder.FindFile(csWildcard);

        int iIdx = 0;
        // Traversing local file list.
        while (bWorking)
        {
            bWorking = Finder.FindNextFile();
            CString csFilePath = Finder.GetFilePath();

            // Get file attributes.
            DWORD dwFileAttribute = GetFileAttributes(csFilePath);

            // Get icon index of file type.
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
        } //! while "Traversing local file list" END

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

    // Modify style of list control.
    DWORD dwAddStyle = m_adwStyle[iNewIndex];
    DWORD dwRemoveStyle = 0;
    if (iOldIndex != -1)
    {
        dwRemoveStyle = m_adwStyle[iOldIndex];
    }

    ref_lstTarget.ModifyStyle(dwRemoveStyle, dwAddStyle, TRUE);
    ref_iActiveStyleIdx = iNewIndex;
} //! CFileTransferDlg::ChangeListStyle END

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

BOOL CFileTransferDlg::BackParentDirctory(
    FILETRANSMITTIONPARTICIPANTTYPE eParticipantType)
                                         
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
#endif // DEBUG
    BOOL bNoError = FALSE;

    do
    {
        // Change path to parent.
        bNoError = m_appathFilePath[eParticipantType]->RemoveFileSpec();
        if (!bNoError)
        {
#ifdef DEBUG
            dwLine = __LINE__;
#endif // DEBUG
        }

        // Put parent path into edit control.
        m_apedtPath[eParticipantType]->SetWindowText(
            *m_appathFilePath[eParticipantType]);

        // Skip.
        (this->*m_apfncClickBtnSkip[eParticipantType])();
    } while (FALSE);

    if (!bNoError)
    {
#ifdef DEBUG
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);
#endif // DEBU
    }

    return bNoError;
} //! CFileTransferDlg::BackParentDirctory END

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
        csTargetFileTitle = m_lstServerFileList.GetItemText(iItem,
                                                            FLCT_FILENAME); 
        
        // If it is '..', go to parent direcotry.
        if (csTargetFileTitle == _T(".."))
        {
            BackParentDirctory(FTPT_SERVER);
            m_edtServerFilePath.SetWindowText(m_pathServerFilePath);
            return;
        }
        else if (csTargetFileTitle == _T("."))
        {
            // If it is '.', quit.
            return;
        }

        m_pathServerFilePath.Append(csTargetFileTitle);
        
        OnBnClickedBtnServerSkip();
    }

    *pResult = 0;
}


// Deal with the message user change style use 
// the server list style combox control.
void CFileTransferDlg::OnCbnSelchangeCmbServerFilelistStyle()
{
    int iIndex = m_cmbServerFileListStyle.GetCurSel();
    ChangeListStyle(m_lstServerFileList,
                    m_iServerActiveStyleIdx,
                    m_iServerActiveStyleIdx,
                    iIndex);
} //! CFileTransferDlg::OnCbnSelchangeCmbServerFilelistStyle END


// Deal with show filelist when ComboBox is change
// that device of local host.
void CFileTransferDlg::OnCbnSelchangeCmbServerDriver()
{
    int iIdx = m_cmbServerDriver.GetCurSel();
    m_cmbServerDriver.GetLBText(iIdx, m_pathServerFilePath);

    // Show file list.
    ShowFileList(m_lstServerFileList,
                 m_iServerActiveStyleIdx);
} //! CFileTransferDlg::OnCbnSelchangeCmbServerDevice END

// Deal with the message user change style use 
// the target host list style combox control.
void CFileTransferDlg::OnCbnSelchangeCmbTargethostFilelistStyle()
{
    int iIndex = m_cmbTargetHostFileListStyle.GetCurSel();
    ChangeListStyle(m_lstTargetHostFileList,
                    m_iTargetHostActiveStyleIdx,
                    m_iTargetHostActiveStyleIdx,
                    iIndex);
} //! CFileTransferDlg::OnCbnSelchangeCmbTargethostFilelistStyle END

// Deal with show filelist when ComboBox is change
// that device of target host.
void CFileTransferDlg::OnCbnSelchangeCmbTargethostDriver()
{
    int iIdx = m_cmbServerDriver.GetCurSel();
    m_cmbServerDriver.GetLBText(iIdx, m_pathServerFilePath);

    ShowFileList(m_lstTargetHostFileList,
                 m_iTargetHostActiveStyleIdx);
} //! CFileTransferDlg::OnCbnSelchangeCmbTargethostDriver END

// Deal with click the 'go' button of target host.
void CFileTransferDlg::OnBnClickedBtnTargethostSkip()
{
    m_edtTargetHostFilePath.GetWindowText(m_pathTartetHostFilePath);

    if (m_pathTartetHostFilePath.IsDirectory())
    {
        ShowFileList(m_lstTargetHostFileList,
                     m_iTargetHostActiveStyleIdx);
    }
} //! CFileTransferDlg::OnBnClickedBtnTargethostSkip END


// Deal with click the '<' button.
void CFileTransferDlg::OnBnClickedBtnGetfile()
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG
    // Format: "FilePath?FileName:pos:id|FileName:pos:id|..."
    CString csFilesListSendToTargetHost;
    BOOL bRet = FALSE;

    do
    {
        // Get name of files need to downlord, and add mission to task manager.
        POSITION posI = m_lstTargetHostFileList.GetFirstSelectedItemPosition();
        if (NULL == posI)
        {
            break;
        }

        CString csFileName;
        CString csFileSize;

        // Get full path.
        CPath phFileNameWithPathSrc = m_pathTartetHostFilePath;

        csFilesListSendToTargetHost = m_pathTartetHostFilePath + _T("?");

        // Traversing Item.
        while (posI)
        {
            int iItemIndex = m_lstTargetHostFileList.GetNextSelectedItem(posI);

            csFileName =
                m_lstTargetHostFileList.GetItemText(iItemIndex, FLCT_FILENAME);
            csFileSize =
                m_lstTargetHostFileList.GetItemText(iItemIndex, FLCT_FILESIZE);

            phFileNameWithPathSrc.Append(csFileName);

            // Insert file name into send list.
            TCHAR szTaskId[MAXBYTE] = { 0 };
 
            csFilesListSendToTargetHost +=
                csFileName + _T(":0") + _T(":") + 
                _ui64tow(m_ullNextTaskId, szTaskId, 10) + _T("|");


            //*************************************
            //*ALARM* This memory will free when manager distroy.
            //*************************************
            PFILETRANSPORTTASK pstTaskInfo = new FILETRANSPORTTASK;
            pstTaskInfo->phFileNameWithPathSrc_ = phFileNameWithPathSrc;
            pstTaskInfo->eTaskType_ = FTT_GETFILE;
            pstTaskInfo->ullFileTotalSize_ = _ttoi(csFileSize);
            pstTaskInfo->ullTransmissionSize_ = 0;
            pstTaskInfo->eTaskStatus_ = FTS_PAUSE;
            pstTaskInfo->ullId_ = m_ullNextTaskId++;
            pstTaskInfo->phFileNameWithPathDst_ = phFileNameWithPathSrc;

            // Check file exist or not.
            if (pstTaskInfo->phFileNameWithPathDst_.FileExists())
            {
                // Create uniq name.
                CPath phUniqFileNameWithPath;
                CPath phFileNameDst = pstTaskInfo->phFileNameWithPathDst_;
                CPath phFilePathDst = pstTaskInfo->phFileNameWithPathDst_;
                phFileNameDst.StripPath();
                phFilePathDst.RemoveFileSpec();


                TCHAR szUniqFileNameWithPath[MAX_PATH] = { 0 };
                bRet = PathYetAnotherMakeUniqueName(szUniqFileNameWithPath,
                                                    phFilePathDst,
                                                    NULL,
                                                    phFileNameDst);
                if (!bRet)
                {
#ifdef DEBUG
                    dwError = GetLastError();
                    GetErrorMessage(dwError, csErrorMessage);
                    OutputDebugStringWithInfo(csErrorMessage,
                                              __FILET__,
                                              __LINE__);
#endif // DEBUG 
                    continue;
                }

                pstTaskInfo->phFileNameWithPathDst_ =
                    szUniqFileNameWithPath;
            }

            // Add task to manager.
            m_TransportTaskManager.InsertGetFileTask(phFileNameWithPathSrc,
                                                     pstTaskInfo);
        } // while "Traversing Item" END

        UpdateTransportList();

        // Send file command and file list to target host.
        SendDataUseIOCP(m_pstClientInfo,
                        m_ref_IOCP,
                        csFilesListSendToTargetHost,
                        PT_FILECOMMAND_GETFILE);

    } while (FALSE);
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
            BackParentDirctory(FTPT_CLIENT);
            return;
        }
        else if (csTargetFileTitle == _T("."))
        {
            // If it is '.', quit.
            return;
        }

        // 拼接子名字
        CString csTargetFileSubName = csTargetFileTitle;
        GetSubName(m_cmbTargetHostDriver,
                   m_edtTargetHostFilePath,
                   csTargetFileSubName);

        if (IsDirectory(m_cmbTargetHostDriver,
                        m_edtTargetHostFilePath,
                        &csTargetFileTitle))
        {
            // Modify the text of title.
            m_edtTargetHostFilePath.SetWindowText(csTargetFileSubName);
            ShowFileList(m_lstTargetHostFileList,
                         //m_cmbTargetHostDriver,
                         //m_edtTargetHostFilePath,
                         m_iTargetHostActiveStyleIdx);
        }
    }

    *pResult = 0;
} //! CFileTransferDlg::OnNMDblclkLstTargethostFilelist END

void CFileTransferDlg::OnClose()
{
    ShowWindow(SW_HIDE);
    //CDialogEx::OnClose();
    //FreeResource();
}

BOOL CFileTransferDlg::WaitRecvFileEvent()
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
#endif // DEBUG

    BOOL bRet = FALSE;
    DWORD dwRet = WaitForSingleObject(m_pevtHadFiletoReceive->m_hObject, 
                                      INFINITE);
    if (WAIT_OBJECT_0 == dwRet)
    {
        bRet = TRUE;
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("Has file event had signaled\r\n"), 
                                  __FILET__, 
                                  __LINE__);
#endif // DEBUG
    }

    return bRet;
} //! CFileTransferDlg::WaitRecvFileEvent END

afx_msg LRESULT CFileTransferDlg::OnHasfiledata(WPARAM wParam, LPARAM lParam)
{
    //Throw the file data to queue.
    PFILEDATAINQUEUE pstFileData = (PFILEDATAINQUEUE)wParam;

    m_TransportTaskManager.InsertFileDataToQueue(pstFileData);

    m_pevtHadFiletoReceive->SetEvent();
    return 0;
} //! CFileTransferDlg::OnHasfiledata END

// Deal with WM_FILEDLGUPDATE message.
afx_msg LRESULT CFileTransferDlg::OnFiledlgupdate(WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG
    BOOL bNoError = FALSE;

    do
    {
        // Analysis parament.
        PFILETRANSPORTTASK pstTaskInfo = (PFILETRANSPORTTASK)wParam;

        if (NULL == pstTaskInfo)
        {
#ifdef DEBUG
            OutputDebugStringWithInfo(_T("The task point is null.\r\n"),
                                      __FILET__,
                                      __LINE__);
#endif // DEBUG
            break;
        }

        FILEDLGUPDATETYPE eUpdateType = (FILEDLGUPDATETYPE)lParam;

        // Get index.
        int iIdx = 0;
        while (iIdx < m_lstTransferTaskList.GetItemCount())
        {
            if (pstTaskInfo->phFileNameWithPathDst_.m_strPath ==
                m_lstTransferTaskList.GetItemText(
                    iIdx,
                    FTLC_DSTFILE))
            {
                break;
            }

            ++iIdx;
        }

        switch (eUpdateType)
        {
            case FDUT_TASKINFO:
            {
                // Update the already transmitted size.
                CString csTransmittedSize;
                _ui64tot(pstTaskInfo->ullTransmissionSize_,
                         csTransmittedSize.GetBufferSetLength(MAXBYTE),
                         10);
                csTransmittedSize.ReleaseBuffer();
                bNoError = 
                    m_lstTransferTaskList.SetItemText(iIdx,
                                                      FTLC_TRANSMITTEDSIZE,
                                                      csTransmittedSize);
                if (!bNoError)
                {
#ifdef DEBUG
                    OutputDebugStringWithInfo(_T("Modify transmitted size "
                                                 "falied.\r\n"),
                                              __FILET__,
                                              __LINE__);
#endif // DEBUG
                }

                if (pstTaskInfo->ullFileTotalSize_ ==
                    pstTaskInfo->ullTransmissionSize_)
                {
                    pstTaskInfo->eTaskStatus_ = FTS_FINISH;
                }

                // Update the status of task.
                bNoError =
                    m_lstTransferTaskList.SetItemText(
                        iIdx,
                        FTLC_TASKSTATUS,
                        m_acsTaskStatus[pstTaskInfo->eTaskStatus_]);
                if (!bNoError)
                {
#ifdef DEBUG
                    dwLine = __LINE__;
                    bOutputErrMsg = TRUE;
#endif // DEBUG
                }

                break;
            } //! case "FDUT_TASKINFO" END
            case FDUT_ERROR:
            {
                bNoError =
                    m_lstTransferTaskList.SetItemText(
                        iIdx,
                        FTLC_TASKSTATUS,
                        m_acsTaskStatus[pstTaskInfo->eTaskStatus_]);
                if (bNoError)
                {
#ifdef DEBUG
                    dwLine = __LINE__;
                    bOutputErrMsg = TRUE;
#endif // DEBUG
                }

                break;
            }
            default:
            {
                break;
            }
        }
    } while (FALSE);


#ifdef DEBUG
    if (bOutputErrMsg && 0 != dwLine)
    {
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);
    }
#endif // DEBUG

    UpdateData();
    return 0;
} //! CFileTransferDlg::OnFiledlgupdate END

void CFileTransferDlg::OnDestroy()
{
    CDialogEx::OnDestroy();

    FreeResource();
}