//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-07-24
// Description: 
//      The define for class CFileTransforDlg.
//
// Modify Log:
//      2018-11-13    Hoffman
//      Info: a. Add below member variable.
//              a.1. m_ullNextTaskId;
//
//      2018-11-14    Hoffman
//      Info: a. Add below member variable.
//              a.1. m_adwStyle;
//
//      2018-11-27    Hoffman
//      Info: a. Add below member variable.
//              a.1. m_apMenu;
//******************************************************************************

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "CommunicationIOCP.h"
#include "StructShare.h"
#include "FileTransportManager.h"
#include "RecvFileDataThread.h"
#include "FileTransferTaskListCtrl.h"

// CFileTransferDlg 对话框
typedef void (CFileTransferDlg::*PFNC_CLICKBTNSKIP)();
class CFileTransferDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CFileTransferDlg)
public:
    CFileTransferDlg(CString &ref_csIPAndPort,
                     PCLIENTINFO pstClientInfo,
                     CCommunicationIOCP &ref_IOCP,
                     CWnd* pParent = NULL);   // 标准构造函数
    virtual ~CFileTransferDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FILETRANSFER };
#endif
private:
    CString m_csDevice;
    CImageList *m_pSysSmallIconImageList = NULL;
    CImageList *m_pSysBigIconImageList = NULL;

    // IOCP
    CCommunicationIOCP &m_ref_IOCP;
    // Ip and port.
    CString &m_ref_csIPAndPort;
    // Client context.
    PCLIENTINFO m_pstClientInfo = NULL;

    // The id for next task.
    ULONG m_ulNextTaskId = 0;

    // Thread recevie file data.
    CRecvFileDataThread *m_pthdRecvFileData = NULL;
    BOOL m_bProcessQuit = FALSE;
    CCriticalSection m_CriticalSection;
    CEvent *m_pevtHadFiletoReceive = NULL;

    CMenu *m_apMenu[TOTAL_FTDMT_NUM] = { NULL };
    CMenu *m_apTransportListMenu[TOTAL_TTLMT_NUM] = { NULL };
    const CString m_acsTaskType[NUM_FILETASKTYPE] = {
        _T("Download"),
        _T("Upload"),
    };
    const CString m_acsTaskStatus[NUM_FILETASKSTATUS] = {
        _T("Pause"),
        _T("Transporting..."),
        _T("Pending"),
        _T("Error"),
        _T("Finish")
    };

    CPath *m_appathFilePath[NUM_PARTICIPANT] = {
        &m_pathServerFilePath,
        &m_pathTartetHostFilePath
    };

    PFNC_CLICKBTNSKIP m_apfncClickBtnSkip[NUM_PARTICIPANT] = {
        &CFileTransferDlg::OnBnClickedBtnServerSkip,
        &CFileTransferDlg::OnBnClickedBtnTargethostSkip
    };

    CEdit *m_apedtPath[NUM_PARTICIPANT] = {
        &m_edtServerFilePath,
        &m_edtTargetHostFilePath
    };

    DWORD m_adwStyle[4] = {
        LVS_ICON,
        LVS_SMALLICON,
        LVS_LIST,
        LVS_REPORT,
    };
    void FreeResource();
    void UpdateTransportList();
public:
    


    BOOL CheckProcessQuitFlag() const
    {
        return m_bProcessQuit;
    }
    

    BOOL WaitRecvFileEvent();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()
public:
    // The event that had got driver.
    HANDLE m_hGetTargetDeviceEvent = NULL;
    // The event that had got file list.
    HANDLE m_hGetTargetFileListEvent = NULL;
    // The sytle of server's filelist.
    int m_iServerActiveStyleIdx = 0;
    // The sytle of target host's filelist.
    int m_iTargetHostActiveStyleIdx = 0;
    // The driver list of server.
    CComboBox m_cmbServerDriver;
    // The edit control of server's path.
    CEdit m_edtServerFilePath;
    // The path object of server.
    CPath m_pathServerFilePath;
    // The file list of target host.
    CString m_csTargetHostFileList;
    // The driver list of target host.
    CString m_csTargetHostDriverList;
    // 目标端的盘符字节数
    size_t m_uiTargetHostDriverLen = 0;

    // Server端文件列表风格
    CComboBox m_cmbServerFileListStyle;
    CComboBox m_cmbTargetHostFileListStyle;
    CListCtrl m_lstServerFileList;
    CListCtrl m_lstTargetHostFileList;

    // The combobox control of target host's driver.
    CComboBox m_cmbTargetHostDriver;
    // The list of transfer task.
    CFileTransferTaskListCtrl m_lstTransferTaskList;
    // The edit control of target host's path.
    CEdit m_edtTargetHostFilePath;
    // The path object of target host.
    CPath m_pathTartetHostFilePath;

    // The manager of task transmission.
    CFileTransportManager m_TransportTaskManager;
    afx_msg void OnBnClickedBtnServerSkip();
    virtual BOOL OnInitDialog();
    void ShowFileList(CListCtrl &lstTarget,
                      const int &iActiveStyleIdx);

    void ChangeListStyle(CListCtrl &lstTarget,
                         int &iActiveStyleIdx,
                         int iOldIndex,
                         int iNewIndex);

    CString GetSubName(CComboBox &ref_cmbDevice,
                       CEdit &ref_edtFilePath, 
                       CString &ref_csFilename);

    BOOL CFileTransferDlg::BackParentDirctory(
        FILETRANSMITTIONPARTICIPANTTYPE eParticipantType);

    BOOL IsDirectory(CComboBox &ref_cmbDevice,
                     CEdit &ref_edtFilePath, 
                     const CString *TargetFile = NULL);


    afx_msg void OnNMDblclkLstServerFilelist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnCbnSelchangeCmbServerFilelistStyle();
    afx_msg void OnCbnSelchangeCmbServerDriver();

    afx_msg void OnCbnSelchangeCmbTargethostFilelistStyle();
    afx_msg void OnCbnSelchangeCmbTargethostDriver();
    afx_msg void OnBnClickedBtnTargethostSkip();
    afx_msg void OnBnClickedBtnGetfile();
    afx_msg void OnBnClickedBtnPutfile();
    afx_msg void OnNMDblclkLstTargethostFilelist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnClose();
protected:
    afx_msg LRESULT OnHasfiledata(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFiledlgupdate(WPARAM wParam, LPARAM lParam);
public:
    afx_msg void OnDestroy();
    afx_msg void OnNMRClickLstTransfertask(NMHDR *pNMHDR, LRESULT *pResult);
};
