#include "stdafx.h"
#include "FileTransferDlg.h"
#include "RecvFileDataThread.h"


CRecvFileDataThread::CRecvFileDataThread()
{
}


CRecvFileDataThread::~CRecvFileDataThread()
{
}

bool CRecvFileDataThread::OnThreadEventRun(LPVOID lpParam)
{
#ifdef DEBUG
    DWORD dwError = -1;
    CString csErrorMessage;
#endif // DEBUG

    // Analysis parament.
    CFileTransferDlg *pFileTransferDlg = (CFileTransferDlg *)lpParam;

    // Work start.
    while (TRUE)
    {
        // Quit loop when the file transport dialog begin to destroy.
        if (pFileTransferDlg->CheckProcessQuitFlag())
        {
            break;
        }

        // Give up currently time when the queue of file data is empty.
        if (pFileTransferDlg->CheckFileDataQueueEmpty())
        {
            // Loop if event isn't signaled.
            if (!pFileTransferDlg->WaitRecvFileEvent())
            {
                continue;
            }
        }

        // Deal with file data.
        do
        {
            FILEDATAINQUEUE stFileData = 
                pFileTransferDlg->GetFileDataFromQueue();
            if (0 == stFileData.FileDataBuffer_.GetBufferLen())
            {
#ifdef DEBUG
                OutputDebugStringWithInfo(_T("The file data is empty.\r\n"),
                                          __FILET__,
                                          __LINE__);
#endif // DEBUG
                break;
            }

            // Get currently file transport task.
            PFILETRANSPORTTASK pstTargetTask = NULL;
            pstTargetTask = 
                pFileTransferDlg->m_TransportTaskManager.GetTask(
                    stFileData.csFileFullName_);
            if (NULL == pstTargetTask)
            {
#ifdef DEBUG
                OutputDebugStringWithInfo(_T("Don't fond currently "
                                             "file transport task\r\n"),
                                          __FILET__,
                                          __LINE__);
#endif // DEBUG
                break;
            }
            
            // Check file exist or not.
            DWORD dwIndex = 1;
            do
            {
                HANDLE hFileTest = CreateFile(stFileData.csFileFullName_,
                                              GENERIC_READ,
                                              NULL,
                                              NULL,
                                              OPEN_EXISTING,
                                              FILE_ATTRIBUTE_NORMAL,
                                              NULL);
                dwError = GetLastError();
                if (ERROR_FILE_NOT_FOUND == dwError)
                {
                    break;
                }
                // File had existed.
                else
                {
                    // Jump out The task is working or had had new file name.
                    if (FTS_TRANSPORTING == pstTargetTask->eTaskStatus_)
                    {
                        if (pstTargetTask->bHasNewFileName_)
                        {
                            stFileData.csFileFullName_ =
                                pstTargetTask->csFilePath_ +
                                pstTargetTask->csFileNewName_;
                        }
                        break;
                    }

                    // split the name from full name.
                    CString csAddInfo;
                    CString csFileName;
                    CString csFileExt;
                    _tsplitpath(stFileData.csFileFullName_.GetString(), 
                                NULL,
                                NULL,
                                csFileName.GetBufferSetLength(MAX_PATH),
                                csFileExt.GetBufferSetLength(MAX_PATH));
                    csFileName.ReleaseBuffer();
                    csFileExt.ReleaseBuffer();

                    // Combination new name.
                    csAddInfo.Format(_T("(%u)"), dwIndex);
                    csFileName += csAddInfo;
                    
                    // Update task info.
                    pstTargetTask->csFileNewName_ = csFileName + csFileExt;
                    pstTargetTask->bHasNewFileName_ = TRUE;

                    // Modify the file name.
                    stFileData.csFileFullName_ =
                        pstTargetTask->csFilePath_ + 
                        pstTargetTask->csFileNewName_;
                } // else "File had existed" END

                CloseHandle(hFileTest);
            } while (TRUE); //! while "Check file exist or not" END

            // Change the task state.
            pstTargetTask->eTaskStatus_ = FTS_TRANSPORTING;
            
            // Open the file.
            HANDLE hTargetFile = CreateFile(stFileData.csFileFullName_,
                                            GENERIC_WRITE, 
                                            NULL,
                                            NULL,
                                            OPEN_ALWAYS,
                                            FILE_ATTRIBUTE_NORMAL,
                                            NULL);
            if (INVALID_HANDLE_VALUE == hTargetFile)
            {
#ifdef DEBUG
                dwError = GetLastError();
                GetErrorMessage(dwError, csErrorMessage);
                OutputDebugStringWithInfo(csErrorMessage, __FILET__, __LINE__);
#endif // DEBU
                break;
            }

            CFile fTargetFile(hTargetFile);
            // Seek point position.
            fTargetFile.Seek(pstTargetTask->ullTransmissionSize_, CFile::begin);
            
            // Write to data.
            fTargetFile.Write(stFileData.FileDataBuffer_.GetBuffer(), 
                              stFileData.FileDataBuffer_.GetBufferLen());
            
            // Update info of this task.
            pstTargetTask->ullTransmissionSize_ = stFileData.ullFilePointPos_;

            // Notify the file transport to update UI.
            

            // Close the file.
            fTargetFile.Close();
        } while (FALSE); // while "Deal with file data" END

    } //! while "Work start" END

    return true;
} //! CRecvFileDataThread::OnThreadEventRun END 