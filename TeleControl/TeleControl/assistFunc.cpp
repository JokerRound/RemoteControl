#include "stdafx.h"
#include "assistFunc.h"

// To get finally info with file name, file line and info output,
// Then call OutputDebugString.
void OutputDebugStringWithInfo(const CString csOuput,
                               const CString csFileNmae,
                               DWORD dwFileLine)
{
    CString csFinallyInfo;
    csFinallyInfo.Format(_T("%s (%d): %s"),
                         csFileNmae.GetString(),
                         dwFileLine,
                         csOuput.GetString());

    OutputDebugString(csFinallyInfo);

} //! OutputDebugStringWithInfo END


// 
void GetErrorMessage(DWORD dwError, CString &csMessage)
{
    // Get default system locale.
    DWORD dwSystemLocal = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    // Buffer that gets the error message.
    HLOCAL hLocal = NULL;

    // Get description of error meesage.
    BOOL bRet = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        dwSystemLocal,
        (LPTSTR)&hLocal,
        0,
        NULL);
    if (!bRet)
    {
        // Failed.
        // Try to get message from netmsg.dll.
        HMODULE hDll = LoadLibraryEx(_T("netmsg.dll"),
                                     NULL,
                                     DONT_RESOLVE_DLL_REFERENCES);
        if (NULL != hDll)
        {
            bRet = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                hDll,
                dwError,
                dwSystemLocal,
                (LPTSTR)&hLocal,
                0,
                NULL);

            FreeLibrary(hDll);
        }
    }

    if (bRet && (NULL != hLocal))
    {
        csMessage = (LPTSTR)LocalLock(hLocal);
    }
    else
    {
#ifdef DEBUG
        OutputDebugStringWithInfo(_T("Can't find the error message."), 
                                  __FILET__, 
                                  __LINE__);
#endif // DEBUG
    }

    LocalFree(hLocal);
} //! GetErrorMessage END

// Package the process that send data.
BOOL SendDataUseIOCP(CLIENTINFO *&ref_pstClientInfo,
                     CCommunicationIOCP &ref_IOCP,
                     CString &ref_csData,
                     PACKETTYPE ePacketType)
{
    PPACKETFORMAT pstPacket =
        (PPACKETFORMAT)ref_pstClientInfo->szSendTmpBuffer_;

    //*************************************
    //*ALARM* Synchronize
    //*************************************
    ref_pstClientInfo->CriticalSection_.Lock();
    pstPacket->ePacketType_ = ePacketType;

    pstPacket->dwSize_ = (ref_csData.GetLength() + 1) * sizeof(TCHAR);

    memmove(pstPacket->szContent_,
            ref_csData.GetBuffer(),
            pstPacket->dwSize_);

    ref_pstClientInfo->SendBuffer_.Write(
        (PBYTE)ref_pstClientInfo->szSendTmpBuffer_,
        PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // Clean send buffer temporary.
    memset(ref_pstClientInfo->szSendTmpBuffer_,
           0,
           PACKET_HEADER_SIZE + pstPacket->dwSize_);

    // Make IOCP to deal with send.
    BOOL bRet =
        ref_IOCP.PostSendRequst(ref_pstClientInfo->sctClientSocket_,
                                ref_pstClientInfo->SendBuffer_);

    ref_pstClientInfo->SendBuffer_.ClearBuffer();
    ref_pstClientInfo->CriticalSection_.Unlock();

    return bRet;
} //! SendDataUseIOCP END
