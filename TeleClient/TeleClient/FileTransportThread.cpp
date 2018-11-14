#include "stdafx.h"
#include "StructShare.h"
#include "FileTransportThread.h"
#include "TeleClientDlg.h"
#include "CommunicationIOCP.h"


CFileTransportThread::CFileTransportThread()
{
}


CFileTransportThread::~CFileTransportThread()
{
}

bool CFileTransportThread::OnThreadEventRun(LPVOID lpParam)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
#endif // DEBUG

    // Analysis parament.
    PFILETRANSPORTTHREADPARAM pFileTransportThreadParam =
        (PFILETRANSPORTTHREADPARAM)lpParam;
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
        CPath phFileNameWithPath = phFilePath;
        bRet = std::regex_search<TCHAR>(strFileList.c_str(),
                                        sMatchResult,
                                        Rgx);
        if (!bRet)
        {
#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBUG
            break;
        }

        std::basic_string<TCHAR> strFileName = sMatchResult[1];
        ULONGLONG ullFileTransportStartPos = 
            _tcstoui64(((std::basic_string<TCHAR>)sMatchResult[2]).c_str(),
                       NULL, 
                       10);
        ULONGLONG ullTaskId =
            _tcstoui64(((std::basic_string<TCHAR>)sMatchResult[3]).c_str(), 
                       NULL,
                       10);
        std::basic_string<TCHAR> strTaskId = sMatchResult[3];

        phFileNameWithPath.Append(strFileName.c_str());
        CFile fTargetFile;
        // Begin file operation.
        do
        {
            if (phFileNameWithPath.IsDirectory())
            {
                // TODO: Have directory.
                break;
            }

            // Open file with read mode.
            bRet = fTargetFile.Open(phFileNameWithPath, CFile::modeRead);
            if (!bRet)
            {
#ifdef DEBUG
                dwError = GetLastError();
                GetErrorMessage(dwError, csErrorMessage);
                OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
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
                csDataBlock.Empty();

                DWORD dwSize = fTargetFile.Read(
                    csDataBlock.GetBufferSetLength(FILE_TRANSPORT_MAXSIZE),
                    FILE_TRANSPORT_MAXSIZE - 1);
                csDataBlock.ReleaseBuffer();

                if (0 == dwSize)
                {
                    break;
                }

                bRet = SendDataUseIOCP(pstClientInfo,
                                       *pIOCP,
                                       csDataBlock,
                                       dwSize,
                                       phFileNameWithPath,
                                       fTargetFile.GetPosition(),
                                       ullTaskId);
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

        // Get remainder text.
        strFileList = sMatchResult.suffix();
    } //! while "Get filename and transport data" END

    return bRet;
} //! CFileTransportThread::OnThreadEventRun END
