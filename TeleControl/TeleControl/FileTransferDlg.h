#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "CommunicationIOCP.h"
#include "StructShare.h"
#include "FileTransportManager.h"
#include "RecvFileDataThread.h"

// CFileTransferDlg 对话框

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
    // The queue of data from target host.
    std::queue<FILEDATAINQUEUE> m_queFileData;
    // Thread recevie file data.
    CRecvFileDataThread *m_pthdRecvFileData = NULL;
    BOOL m_bProcessQuit = FALSE;
    CCriticalSection m_CriticalSection;
    CEvent *m_pevtHadFiletoReceive = NULL;

    const CString m_acsTaskType[NUM_FILETASKTYPE] = {
        _T("Download"),
        _T("Upload"),
    };
    const CString m_acsTaskStatus[NUM_FILETASKSTATUS] = {
        _T("Start"),
        _T("Transporting"),
        _T("Pause"),
        _T("Finish")
    };

    void FreeResource();
    void UpdateTransportList();
public:
    void InsertFileDataToQueue(_In_ FILEDATAINQUEUE &ref_stFileData);
    FILEDATAINQUEUE GetFileDataFromQueue();
    BOOL CheckFileDataQueueEmpty();


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
    CComboBox m_cmbServerDevice;
    // The file path of server.
    CEdit m_edtServerFilePath;
    // the file list of target host.
    CString m_csFileList;
    // 目标端的盘符
    CString m_csTargetHostDevice;
    // 目标端的盘符字节数
    size_t m_uiTargetHostDeviceLen = 0;

    afx_msg void OnBnClickedBtnServerSkip();
    virtual BOOL OnInitDialog();
    void ShowFileList(CListCtrl &lstTarget,
                      CComboBox &cmbDevice, CEdit & edtFilePath, const int & iActiveStyleIdx);

    void ChangeListStyle(CListCtrl &lstTarget,
                         int &iActiveStyleIdx,
                         int iOldIndex,
                         int iNewIndex);

    CString GetSubName(CComboBox &ref_cmbDevice,
                       CEdit &ref_edtFilePath, 
                       CString &ref_csFilename);

    void BackParentDirctory(CComboBox &ref_cmbDevice,
                            CEdit &ref_edtFilePath);

    BOOL IsDirectory(CComboBox &ref_cmbDevice,
                     CEdit &ref_edtFilePath, 
                     const CString *TargetFile = NULL);

    // Server端文件列表风格
    CComboBox m_cmbServerFileListStyle;
    CComboBox m_cmbTargetHostFileListStyle;
    CListCtrl m_lstServerFileList;
    CListCtrl m_lstTargetHostFileList;

    // 目标主机盘符
    CComboBox m_cmbTargetHostDevice;
    // 传输任务列表
    CListCtrl m_lstTransferTask;
    // 目标主机文件路径
    CEdit m_edtTargetHostFilePath;

    // The manager of task transmission.
    CFileTransportManager m_TransportTaskManager;

    afx_msg void OnNMDblclkLstServerFilelist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnCbnSelchangeCmbServerFilelistStyle();
    afx_msg void OnCbnSelchangeCmbServerDevice();

    afx_msg void OnCbnSelchangeCmbTargethostFilelistStyle();
    afx_msg void OnCbnSelchangeCmbTargethostDevice();
    afx_msg void OnBnClickedBtnTargethostSkip();
    afx_msg void OnBnClickedBtnGetfile();
    afx_msg void OnBnClickedBtnPutfile();
    afx_msg void OnNMDblclkLstTargethostFilelist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnClose();
protected:
    afx_msg LRESULT OnHasfiledata(WPARAM wParam, LPARAM lParam);
};
