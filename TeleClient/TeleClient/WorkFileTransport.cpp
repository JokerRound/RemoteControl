//******************************************************************************
// License:     MIT
// Author:      Hoffman
// GitHub:      https://github.com/JokerRound
// Create Time: 2018-12-08
// Description: 
//      The achieve of class CWorkFileTransport.
//
// Modify Log:
//      2018-12-08    Hoffman
//      Info: a. Add below member method.
//              a.1. WorkBody();
//******************************************************************************

#include "stdafx.h"
#include "WorkFileTransport.h"
#include "StructShare.h"
#include "FileTransportThread.h"
#include "TeleClientDlg.h"
#include "CommunicationIOCP.h"


CWorkFileTransport::CWorkFileTransport(PVOID pvContext)
{
    m_stParam = *(PFILETRANSPORTTHREADPARAM)pvContext;
}

CWorkFileTransport::~CWorkFileTransport()
{
}


BOOL CWorkFileTransport::WorkBody() noexcept(FALSE)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
    DWORD dwLine = 0;
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG

    // Analysis parament.
    PFILETRANSPORTTHREADPARAM pFileTransportThreadParam = &m_stParam;
    CTeleClientDlg *pTeleClientDlg = 
        pFileTransportThreadParam->pTeleClientDlg_;
    CString csFileListToGet = *(pFileTransportThreadParam->pcsFileListToGet_);
    CCommunicationIOCP *pIOCP = pFileTransportThreadParam->pIOCP_;
    PCLIENTINFO pstClientInfo = pFileTransportThreadParam->pstClientInfo_;
    
    // Thread had initilazed, signal event. 
    pTeleClientDlg->m_pevtGetFileThreadInitializeEvent->SetEvent();

    // Get path.
    CPath phFilePath;
    int iIndex = csFileListToGet.Find(_T("?"));
    phFilePath = csFileListToGet.Left(iIndex).GetString();
    csFileListToGet = 
        csFileListToGet.Right(csFileListToGet.GetLength() - iIndex -1);

    // Get every filename.
    std::basic_string<TCHAR> strFileList = csFileListToGet.GetString();
    std::match_results<const TCHAR*> sMatchResult;
    std::basic_regex<TCHAR> Rgx(_T("(.*)?:(.*)?:(.*)?\\|"));

    bool bRet = false;

    // Get filename and transport data.
    while (!strFileList.empty())
    {
        CPath pathFileNameWithPath = phFilePath;
        bRet = std::regex_search<TCHAR>(strFileList.c_str(),
                                        sMatchResult,
                                        Rgx);
        if (!bRet)
        {
#ifdef DEBUG
            bOutputErrMsg = TRUE;
            dwLine = __LINE__;
#endif // DEBUG
            break;
        }

        std::basic_string<TCHAR> strFileName = sMatchResult[1];
        ULONGLONG ullFileTransportStartPos = 
            _tcstoui64(((std::basic_string<TCHAR>)sMatchResult[2]).c_str(),
                       NULL, 
                       10);
        ULONG ulTaskId =
            _tcstoul(((std::basic_string<TCHAR>)sMatchResult[3]).c_str(), 
                       NULL,
                       10);
        std::basic_string<TCHAR> strTaskId = sMatchResult[3];

        pathFileNameWithPath.Append(strFileName.c_str());

        CFile fTargetFile;
        // Begin file operation.
        do
        {
            // Check 
            if (pTeleClientDlg->CheckPauseFlag(pathFileNameWithPath))
            {
                // Jump out.
                break;
            }

            if (pathFileNameWithPath.IsDirectory())
            {
                // TODO: Have directory.
                break;
            }

            // Open file with read mode.
            bRet = fTargetFile.Open(pathFileNameWithPath, CFile::modeRead);
            if (!bRet)
            {
#ifdef DEBUG
                bOutputErrMsg = TRUE;
                dwLine = __LINE__;
#endif // DEBUG
                break;
            }

            // Move file point to start postion.
            fTargetFile.Seek(ullFileTransportStartPos, CFile::begin);

            // The formate is "FileFullName:FilePointPos|FileData"
            CString csDataBlock;

            // Read data and transport to Server.
            do
            {
                // Deal with pause event.
                if (pTeleClientDlg->CheckPauseFlag(pathFileNameWithPath))
                {
                    break;
                }

                csDataBlock.Empty();

                DWORD dwSize = fTargetFile.Read(
                    csDataBlock.GetBufferSetLength(FILE_TRANSPORT_MAXSIZE),
                    FILE_TRANSPORT_MAXSIZE - 1);
                csDataBlock.ReleaseBuffer();

                if (0 == dwSize)
                {
                    // File data transport over.
#ifdef DEBUG
                    OutputDebugStringWithInfo(
                        _T("File data transport over.\r\n"),
                        __FILET__,
                        __LINE__);
#endif // DEBUG

                    break;
                }

                bRet = SendDataUseIOCP(pstClientInfo,
                                       *pIOCP,
                                       csDataBlock,
                                       dwSize,
                                       pathFileNameWithPath,
                                       fTargetFile.GetPosition(),
                                       ulTaskId);
                if (!bRet)
                {
#ifdef DEBUG
                    OutputDebugStringWithInfo(_T("File data sends failed.\r\n"),
                                              __FILET__,
                                              __LINE__);
#endif // DEBUG
                    break;
                }
            } while (TRUE); // while "Read data" END

            fTargetFile.Close();
        } while (FALSE); // while "Begin file operation" END

#ifdef DEBUG
        if (bOutputErrMsg && dwLine != 0)
        {
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);

            bOutputErrMsg = FALSE;
            dwLine = 0;
        }
#endif // DEBUG 

        // Get remainder text.
        strFileList = sMatchResult.suffix();
    } //! while "Get filename and transport data" END

#ifdef DEBUG
    if (bOutputErrMsg && dwLine != 0)
    {
        dwError = GetLastError();
        GetErrorMessage(dwError, csErrorMessage);
        OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);

        bOutputErrMsg = FALSE;
        dwLine = 0;
    }

    OutputDebugStringWithInfo(_T("The file transport work executive over.\r\n"),
                              __FILET__,
                              __LINE__);
#endif // DEBUG 

    return bRet;
} //! CWorkFileTransport::WorkBody() END
