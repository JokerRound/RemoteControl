//******************************************************************************
// License:     MIT
// Author:      Hoffman
// Create Time: 2018-10-15
// Description: 
//      Declares of assist function.
//
// Modify Log:
//      2018-10-15    Hoffman
//      Info: Add declares of below functions.
//              OutputDebugStringWithInfo();
//              GetErrorMessage();
//******************************************************************************

#pragma once
#ifndef ASSISTFUNC_H_
#define ASSISTFUNC_H_
#include "StructShare.h"
#include "CommunicationIOCP.h"
#include "MacroShare.h"

void OutputDebugStringWithInfo(_In_ const CString csOuput,
                               _In_ const CString csFileNmae,
                               _In_ DWORD dwFileLine);


void GetErrorMessage(_In_ DWORD dwError,
                     _In_ CString &ref_csMessage);



#endif // !ASSISTFUNC_H_
