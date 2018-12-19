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

} //! OutputDebugStringWithInfo() END


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
} //! GetErrorMessage() END

