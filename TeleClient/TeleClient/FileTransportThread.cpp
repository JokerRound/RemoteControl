#include "stdafx.h"
#include "StructShare.h"
#include "FileTransportThread.h"
#include "TeleClientDlg.h"


CFileTransportThread::CFileTransportThread()
{
}


CFileTransportThread::~CFileTransportThread()
{
}

bool CFileTransportThread::OnThreadEventRun(LPVOID lpParam)
{
    PFILETRANSPORTTHREADPARAM pFileTransportThreadParam =
        (PFILETRANSPORTTHREADPARAM)lpParam;
    CTeleClientDlg *pTeleClientDlg = 
        pFileTransportThreadParam->pTeleClientDlg_;
    CString csFileListToGet = 
        *(pFileTransportThreadParam->pcsFileListToGet_);
    
    // Thread had initilazed, signal event. 
    pTeleClientDlg->m_GetFileThreadInitializeEvent.SetEvent();

    // Get path.
    std::basic_string<TCHAR> strFilePath;
    int iIndex = csFileListToGet.Find(_T("?"));
    strFilePath = csFileListToGet.Left(iIndex).GetString();
    csFileListToGet = 
        csFileListToGet.Right(csFileListToGet.GetLength() - iIndex -1);

    // Get every filename.
    std::basic_string<TCHAR> strFileList = csFileListToGet.GetString();
    std::match_results<const TCHAR*> sMatchResult;
    std::basic_regex<TCHAR> Rgx(_T("(.*)?:(.*)?\\|"));

    bool bRet = false;
    while (!strFileList.empty())
    {
        bRet = std::regex_search<TCHAR>(strFileList.c_str(),
                                        sMatchResult,
                                        Rgx);
        if (!bRet)
        {
#ifdef DEBUG

#endif // DEBUG
            break;
        }

        std::basic_string<TCHAR> strFileNameWithPath = 
            strFilePath + (std::basic_string<TCHAR>)sMatchResult[1];
        int iFileTransportStartPos = 
            _ttoi(((std::basic_string<TCHAR>)sMatchResult[2]).c_str());



        // Get remainder text.
        strFileList = sMatchResult.suffix();
    }

    return bRet;
} //! CFileTransportThread::OnThreadEventRun END
