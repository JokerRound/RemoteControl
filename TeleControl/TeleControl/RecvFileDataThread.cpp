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
    DWORD dwLine = 0;
#endif // DEBUG

    // Analysis parament.
    CFileTransferDlg *pFileTransferDlg = (CFileTransferDlg *)lpParam;

    // Work start.
    while (TRUE)
    {
        // Wait work event.
        // Quit loop when the file transport dialog begin to destroy or 
        // wait event failed.
        if (!pFileTransferDlg->WaitRecvFileEvent() 
            || pFileTransferDlg->CheckProcessQuitFlag())
        {
            break;
        }

        // Give up this chance if queue is empty.
        if (pFileTransferDlg->m_TransportTaskManager.CheckFileDataQueueEmpty())
        {
            continue;
        }


        BOOL bNoError = FALSE;
        PFILEDATAINQUEUE pstFileData = NULL;
        // Deal with file data.
        do
        {
            pstFileData = 
                pFileTransferDlg->m_TransportTaskManager.GetFileDataFromQueue();
            if (0 == pstFileData->FileDataBuffer_.GetBufferLen())
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
                    pstFileData->phFileNameWithPath_);
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
            
            // Get file object corresponded task.
            CFile *pfTargetFile = NULL;
            HANDLE hTargetFile = INVALID_HANDLE_VALUE;
            do
            {
                pfTargetFile =
                    pFileTransferDlg->m_TransportTaskManager.GetFileObject(
                        pstTargetTask->phFileNameWithPathDst_);
                if (NULL != pfTargetFile)
                {
                    // Get successfully, jump out.
                    break;
                }

                // Check file exist or not.
                CPath phFileName = pstFileData->phFileNameWithPath_;
                CPath phFilePath = pstFileData->phFileNameWithPath_;
                phFileName.StripPath();
                phFilePath.RemoveFileSpec();
                if (pstFileData->phFileNameWithPath_.FileExists())
                {
                    // Create uniq name.
                    CPath phUniqFileNameWithPath;
                    do
                    {

                        TCHAR szUniqFileNameWithPath[MAX_PATH] = { 0 };
                        bNoError =
                            PathYetAnotherMakeUniqueName(szUniqFileNameWithPath,
                                                         phFilePath,
                                                         NULL,
                                                         phFileName);
                        if (!bNoError)
                        {
#ifdef DEBUG
                            dwLine = __LINE__;
#endif // DEBUG
                            break;
                        }

                        pstTargetTask->phFileNameWithPathDst_ = 
                            szUniqFileNameWithPath;
                    } while (
                        pstTargetTask->phFileNameWithPathDst_.FileExists());

                    // Jump control.
                    if (!bNoError)
                    {
                        break;
                    }
                }

                // Open the file.
                hTargetFile = CreateFile(pstTargetTask->phFileNameWithPathDst_,
                                         GENERIC_WRITE,
                                         NULL,
                                         NULL,
                                         OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL);
                if (INVALID_HANDLE_VALUE == hTargetFile)
                {
                    break;
                }

                //******************************************************** 
                //* Alarm * This memory will free when delete the task
                //          or the file transport dialog will be destroyed.
                //******************************************************** 
                pfTargetFile = new CFile(hTargetFile);
                if (NULL == pfTargetFile)
                {
#ifdef DEBUG
                    dwLine = __LINE__;
#endif // DEBUG
                    break;
                }

                // Add this file object into file transport manager.
                pFileTransferDlg->m_TransportTaskManager.InsertFileObject(
                    pstTargetTask->phFileNameWithPathDst_,
                    pfTargetFile);

            } while (FALSE); //! while "Get file object corresponded task" END

            // Get file object had failed.
            if (NULL == pfTargetFile)
            {
                if (INVALID_HANDLE_VALUE != hTargetFile)
                {
                    CloseHandle(hTargetFile);
                }

                // Notify the file transport to update UI.

                break;
            }

            // Change the task state.
            pstTargetTask->eTaskStatus_ = FTS_TRANSPORTING;
            
            // Seek point position.
            pfTargetFile->Seek(pstTargetTask->ullTransmissionSize_,
                               CFile::begin);
            
            // Write to data.
            pfTargetFile->Write(pstFileData->FileDataBuffer_.GetBuffer(), 
                                pstFileData->FileDataBuffer_.GetBufferLen());
            
            // Update info of this task.
            pstTargetTask->ullTransmissionSize_ = pstFileData->ullFilePointPos_;

            // Notify the file transport to update UI.
            pFileTransferDlg->SendMessage(WM_FILEDLGUPDATE, 
                                          (WPARAM)pstTargetTask,
                                          FDUT_TASKINFO);

            // Delete the had finished task.
            if (pstTargetTask->ullTransmissionSize_ ==
                pstTargetTask->ullFileTotalSize_)
            {
                BOOL bRet =
                    pFileTransferDlg->
                    m_TransportTaskManager.DeleteTaskAndFileObject(
                        pstTargetTask->phFileNameWithPathDst_);
                if (!bRet)
                {
#ifdef DEBUG
                    OutputDebugStringWithInfo(_T("Delete task failed.\r\n"),
                                              __FILET__,
                                              __LINE__);
#endif // DEBUG
                }
            }



            bNoError = TRUE;
        } while (FALSE); // while "Deal with file data" END

        // Free the memory of file data.
        if (NULL != pstFileData)
        {
            delete pstFileData;
            pstFileData = NULL;
        }

        if (!bNoError && 0 != dwLine)
        {
#ifdef DEBUG
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);
#endif // DEBUG
        }


    } //! while "Work start" END

    return true;
} //! CRecvFileDataThread::OnThreadEventRun END 