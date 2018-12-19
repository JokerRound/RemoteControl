//******************************************************************************
// License:     MIT
// Author:      Hoffman
// GitHub:      https://github.com/JokerRound
// Create Time: 2018-11-30
// Description: 
//      The achieve of class CWinThreadpool.
//
// Modify Log:
//      2018-11-30    Hoffman
//      Info: a. Add below member methods.
//              a.1. Destroy();
//              a.1. AddWorkItem();
//              a.1. DeleteWorkItem();
//              a.1. SubmitWorkItem();
//              a.1. NoWorkItem();
//            b. Modify below member methods.
//              b.1. ~CWinThreadpool():
//                  b.1.1. Call Clear() member methods.
//
//      2018-12-02    Hoffman
//      Info: a. Add below member methdos.
//              a.1. WaitForWorkItem();
//
//      2018-12-18    Hoffman
//      Info: a. Add below member methods.
//              a.1. FindWorkItem();
//              a.2. UpdateWorkItem();
//******************************************************************************

#include "stdafx.h"
#include "WinThreadpool.h"

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Traversing the map for deleting the memory, then erase it.
//******************************************************************************
void CWinThreadpool::Destroy() noexcept
{
    if (!m_mapWorkItem.empty())
    {
        WORKITEMTYPE eI = m_eFirstWorkItemType;

        do
        {
            DeleteWorkItem(eI);

            eI = (WORKITEMTYPE)(eI + 1);
        } while (eI != TOTAL_NUM_WIT);
    }
}

CWinThreadpool::CWinThreadpool() noexcept
{
}

CWinThreadpool::~CWinThreadpool() noexcept
{
    Clear();
}

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Call insert of std::map if pWorkItem isn't NULL.
//******************************************************************************
void CWinThreadpool::AddWorkItem(_In_ WORKITEMTYPE eWorkItemType,
                                 _In_ CWork *pWorkItem) noexcept
{
    if (NULL == pWorkItem)
    {
        return;
    }

    m_mapWorkItem[eWorkItemType] = pWorkItem;
}

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      It delete the work from commited work item queue.
//      Then delete the work item if find it from map.
//******************************************************************************
void CWinThreadpool::DeleteWorkItem(_In_ WORKITEMTYPE eWorkItemType,
                                    _In_ BOOL bCancelPendingWork /*= TRUE*/) 
    noexcept
{
    if (NULL == m_mapWorkItem[eWorkItemType])
    {
        return;
    }

    WaitForThreadpoolWorkCallbacks(m_mapWorkItem[eWorkItemType]->GetPTPWork(),
                                   bCancelPendingWork);
    if (NULL != m_mapWorkItem[eWorkItemType])
    {
        delete m_mapWorkItem[eWorkItemType];
        m_mapWorkItem[eWorkItemType] = NULL;
    }

    m_mapWorkItem.erase(eWorkItemType);
}

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-12-18
// Last Time:           2018-12-18
// Logical Descrition:  
//      It uses the find method of map, result is a iterator, then 
//      compare with the end iterator.
//******************************************************************************
BOOL CWinThreadpool::FindWorkItem(WORKITEMTYPE eWorkItemType) noexcept
{
    return m_mapWorkItem.find(eWorkItemType) != m_mapWorkItem.end();
} //! CWinThreadpool::FindWorkItem() END

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-12-18
// Last Time:           2018-12-18
// Logical Descrition:  
//      Find the the target work item, then delete source work item and replace
//      new work item.
//******************************************************************************
BOOL CWinThreadpool::UpdateWorkItem(WORKITEMTYPE eWorkItemType,
                                    CWork *pWorkItem) noexcept
{
    auto itorTargetWorkItem = m_mapWorkItem.find(eWorkItemType);
    if (m_mapWorkItem.end() != itorTargetWorkItem)
    {
        delete itorTargetWorkItem->second;
        itorTargetWorkItem->second = pWorkItem;

        return TRUE;
    }

    return FALSE;
} //! CWinThreadpool::UpdateWorkItem() END

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Check and call api SubmitThreadpoolWork.
//******************************************************************************
void CWinThreadpool::SubmitWorkItem(_In_ WORKITEMTYPE eWorkItemType) noexcept
{
    if (NULL == m_mapWorkItem[eWorkItemType])
    {
        return;
    }

    SubmitThreadpoolWork(m_mapWorkItem[eWorkItemType]->GetPTPWork());
}

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Call Destory() fucntion.
//******************************************************************************
void CWinThreadpool::Clear() noexcept
{
    Destroy();
} //! CWinThreadpool::Clear() END

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-11-30
// Last Time:           2018-11-30
// Logical Descrition:  
//      Call insert of std::map if pWorkItem isn't NULL.
//******************************************************************************
BOOL CWinThreadpool::NoWorkItem() const noexcept
{
    return m_mapWorkItem.empty();
} //! CWinThreadpool::NoWorkItem() END

//******************************************************************************
// Author:              Hoffman
// Create Time:         2018-12-02
// Last Time:           2018-12-02
// Logical Descrition:  
//      Check type isn't TOTAL_NUM_WIT, then Call CWork's WaitForFinish(),
//      or traversing all work item type for waiting.
//******************************************************************************
void CWinThreadpool::WaitForWorkItem(WORKITEMTYPE eWorkItemType)
{
    if (TOTAL_NUM_WIT != eWorkItemType)
    {
        m_mapWorkItem[eWorkItemType]->WaitForFinish();
        return;
    }

    WORKITEMTYPE eI = m_eFirstWorkItemType;
    do
    {
        if (NULL != m_mapWorkItem[eI])
        {
            m_mapWorkItem[eI]->WaitForFinish();
        }

        eI = (WORKITEMTYPE)(eI + 1);
    } while (eI < TOTAL_NUM_WIT);
} //! CWinThreadpool::WaitForWorkItem() END