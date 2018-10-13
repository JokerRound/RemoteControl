#pragma once
#ifndef ASSISTFUNC_H_
#define ASSISTFUNC_H_


void OutputDebugStringWithInfo(_In_ const CString csOuput,
                               _In_ const CString csFileNmae,
                               _In_ DWORD dwFileLine);


void GetErrorMessage(DWORD dwError, CString &csMessage);

#endif // !ASSISTFUNC_H_
