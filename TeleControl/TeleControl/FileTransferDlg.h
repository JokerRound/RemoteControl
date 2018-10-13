#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "CommunicationIOCP.h"
#include "StructShare.h"

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
    // 盘符
    CString m_csDevice;
    CImageList *m_pSysSmallIconImageList = NULL;
    CImageList *m_pSysBigIconImageList = NULL;


    // IOCP
    CCommunicationIOCP &m_ref_IOCP;
    // IP和端口
    CString &m_ref_csIPAndPort;
    // Client上下文
    PCLIENTINFO m_pstClientInfo = NULL;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    // 盘符获取完毕事件
    HANDLE m_hGetTargetDeviceEvent = NULL;
    // 文件列表获取完毕事件
    HANDLE m_hGetTargetFileListEvent = NULL;
    // Server端当前文件列表风格
    int m_iServerActiveStyleIdx = 0;
    // 目标当前文件列表风格
    int m_iTargetHostActiveStyleIdx = 0;
    // 服务器盘符列表
    CComboBox m_cmbServerDevice;
    // 服务器端文件路径
    CEdit m_edtServerFilePath;
    // 目标端的文件列表
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
    afx_msg void OnNMDblclkLstServerFilelist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnCbnSelchangeCmbServerFilelistStyle();
    afx_msg void OnCbnSelchangeCmbServerDevice();
    // 目标主机盘符
    CComboBox m_cmbTargetHostDevice;
    // 传输任务列表
    CListCtrl m_lstTransferTask;
    // 目标主机文件路径
    CEdit m_edtTargetHostFilePath;
    afx_msg void OnCbnSelchangeCmbTargethostFilelistStyle();
    afx_msg void OnCbnSelchangeCmbTargethostDevice();
    afx_msg void OnBnClickedBtnTargethostSkip();
    afx_msg void OnBnClickedBtnGetfile();
    afx_msg void OnBnClickedBtnPutfile();
    afx_msg void OnClose();
    afx_msg void OnNMDblclkLstTargethostFilelist(NMHDR *pNMHDR, LRESULT *pResult);
};
