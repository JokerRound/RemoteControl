//******************************************************************************
// License:     MIT
// Author:      Hoffman
// GitHub:      https://github.com/JokerRound
// Create Time: 2018-11-30
// Description: 
//      The ahcieve of class CWork.
//
// Modify Log:
//      2018-11-30    Hoffman
//      Info: a. Add below member methods.
//              a.1. WorkCallBack();
//              a.2. WorkBody();
//              a.3. CWork(): overload by pvContext;
//              a.4. GetPTPWork();
//            b. Delete below member method.
//              a.1. CWork(): default.
//            c. Modify below member method.  
//              c.1. ~CWork();
//                  c.1.1. Add close active for work.
//******************************************************************************

#include "stdafx.h"
#include "Work.h"

void CWork::Init()
{
    m_pstWork = CreateThreadpoolWork(
        (PTP_WORK_CALLBACK)WorkCallBack,
        (PVOID)this,
        NULL);
    if (NULL == m_pstWork)
    {
        std::runtime_error("The call CreateThreadpooolWork failed.");
    }
}


//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Call Init function;
//******************************************************************************
CWork::CWork() noexcept(FALSE)
{
    try
    {
        Init();
    }
    catch (const std::exception &excptObject)
    {
        throw;
    }
}

CWork::~CWork()
{
    WaitForThreadpoolWorkCallbacks(m_pstWork, TRUE);

    CloseThreadpoolWork(m_pstWork);
}

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Call Workbody functions, take some finish operative.
//******************************************************************************
void CWork::WorkCallBack(_In_ PTP_CALLBACK_INSTANCE pstInstance, 
                         _In_ PVOID pvContext,
                         _In_ PTP_WORK pstWork) noexcept(FALSE)
{
    CWork *pThis = (CWork *)pvContext;

    try
    {
        bool bRet = pThis->WorkBody();
        if (!bRet)
        {
            // TODO: Ouput debug info.
        }
    }
    catch (const std::exception &expctObject)
    {
        MessageBoxA(NULL, expctObject.what(), "Error", MB_OK);
    }
}

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      This functions achieve will wirte by child class.
//******************************************************************************
BOOL CWork::WorkBody() noexcept(FALSE)
{
    return true;
}

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Return the member variable m_pstWork directly;
//******************************************************************************
PTP_WORK CWork::GetPTPWork() const noexcept
{
    return m_pstWork;
}
    
//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-12-02
// Last Time:           2018-12-02
// Logical Descrition:  
//      Call api to wait;
//******************************************************************************
void CWork::WaitForFinish()
{ 
    WaitForThreadpoolWorkCallbacks(m_pstWork, FALSE);
}

void CWork::UpdateWorkContext(PVOID pvContext)
{
}
