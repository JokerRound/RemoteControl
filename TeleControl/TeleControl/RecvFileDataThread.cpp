//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-11-01
// Description: 
//      The achieve of class CRecvFileDataThread's member methods.
//
// Modify Log:
//      2018-11-01    Hoffman
//      Info: a. Add achieve of below member method.
//              a.1. OnThreadEventRun();
//
//      2018-11-13    Hoffman
//      Info: a. Modify ahcieve of below member method.
//              a.1. OnThreadEventRun(): 
//                  a.1.1. Remove for check destination exist or not.
//
//      2018-11-14    Hoffman
//      Info: a. Modify achieve of below member methods.
//              a.1. OnThreadEventRun(): 
//                  a.1.1. Modify deal with it had occured error.
//
//      2018-12-18    Hoffman
//      Info: a. Modify below member method.
//              a.1. OnThreadEventRun():  
//                  a.1.1. Add bHasError flag to check it needs to send messaege
//                         to FileTransportDlg or not.
//******************************************************************************

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
    BOOL bOutputErrMsg = FALSE;
#endif // DEBUG
    // Analysis parament.
    CFileTransferDlg *pFileTransferDlg = (CFileTransferDlg *)lpParam;

    BOOL bRet = FALSE;
    BOOL bHasError = FALSE;
    // Work start.
    while (TRUE)
    {
        // Wait work event.
        // Quit loop when the file transport dialog begin to destroy or 
        // wait event failed.
        if (!pFileTransferDlg->WaitRecvFileEvent() 
            || pFileTransferDlg->CheckProcessQuitFlag())
        {
            return true;
        }

        // Give up this chance if queue is empty.
        if (pFileTransferDlg->m_TransportTaskManager.CheckFileDataQueueEmpty())
        {
            continue;
        }

        BOOL bNoError = FALSE;
        PFILEDATAINQUEUE pstFileData = NULL;
        PFILETRANSPORTTASK pstTargetTask = NULL;
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
                bHasError = TRUE;
                break;
            }

            // Get currently file transport task.
            pstTargetTask = 
                pFileTransferDlg->m_TransportTaskManager.GetTask(
                    pstFileData->ulTaskId_);
            if (NULL == pstTargetTask)
            {
#ifdef DEBUG
                OutputDebugStringWithInfo(_T("Don't fond currently "
                                             "file transport task\r\n"),
                                          __FILET__,
                                          __LINE__);
#endif // DEBUG
                bHasError = TRUE;
                break;
            }
            
            // Get file object corresponded task.
            CFile *pfTargetFile = NULL;
            HANDLE hTargetFile = INVALID_HANDLE_VALUE;
            do
            {
                pfTargetFile =
                    pFileTransferDlg->m_TransportTaskManager.GetFileObject(
                        pstTargetTask->pathFileNameWithPathDst_);
                if (NULL != pfTargetFile)
                {
                    // Get successfully, jump out.
                    break;
                }


                // Open the file.
                hTargetFile = CreateFile(pstTargetTask->pathFileNameWithPathDst_,
                                         GENERIC_WRITE,
                                         NULL,
                                         NULL,
                                         OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL);
                if (INVALID_HANDLE_VALUE == hTargetFile)
                {
#ifdef DEBUG
                    dwLine = __LINE__;
                    bOutputErrMsg = TRUE;
#endif // DEBUG
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
                    bOutputErrMsg = TRUE;
#endif // DEBUG
                    break;
                }

                // Add this file object into file transport manager.
                pFileTransferDlg->m_TransportTaskManager.InsertFileObject(
                    pstTargetTask->pathFileNameWithPathDst_,
                    pfTargetFile);

            } while (FALSE); //! while "Get file object corresponded task" END

            // Get file object had failed.
            if (NULL == pfTargetFile)
            {
                if (INVALID_HANDLE_VALUE != hTargetFile)
                {
                    CloseHandle(hTargetFile);
                }

                // Jmp
                bHasError = TRUE;
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

            // Free the memory of file data.
            if (NULL != pstFileData)
            {
                delete pstFileData;
                pstFileData = NULL;
            }

            // Delete the had finished task.
            if (pstTargetTask->ullTransmissionSize_ ==
                pstTargetTask->ullFileTotalSize_)
            {
#ifdef DEBUG
                OutputDebugStringWithInfo(_T("File transport successful!\r\n"),
                                          __FILET__,
                                          __LINE__);
#endif // DEBUG

                bRet = pFileTransferDlg->
                    m_TransportTaskManager.DeleteTaskAndFileObject(
                        pstTargetTask->pathFileNameWithPathDst_);
#ifdef DEBUG
                if (!bRet)
                {
                    OutputDebugStringWithInfo(_T("Delete task failed.\r\n"),
                                              __FILET__,
                                              __LINE__);
                }
#endif // DEBUG
                pstTargetTask = NULL;
            }
        } while (FALSE); // while "Deal with file data" END

#ifdef DEBUG
        if (bOutputErrMsg && 0 != dwLine)
        {
            dwError = GetLastError();
            GetErrorMessage(dwError, csErrorMessage);
            OutputDebugStringWithInfo(csErrorMessage, __FILET__, dwLine);
            
            bOutputErrMsg = FALSE;
            dwLine = 0;
        }
#endif // DEBUG

        // Notify the file transport dialog to update UI 
        // the task had occured error.
        if (bHasError)
        {
            if (NULL != pstTargetTask)
            {
                pstTargetTask->eTaskStatus_ = FTS_ERROR;
                pFileTransferDlg->SendMessage(WM_FILEDLGUPDATE,
                                              (WPARAM)pstTargetTask,
                                              FDUT_ERROR);
            }
        }    
    } //! while "Work start" END

#ifdef DEBUG
    OutputDebugStringWithInfo(_T("The receive file data thread quit.\r\n"),
                              __FILET__,
                              __LINE__);
#endif // DEBUG

    return true;
} //! CRecvFileDataThread::OnThreadEventRun END 