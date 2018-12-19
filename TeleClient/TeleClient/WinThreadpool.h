//******************************************************************************
// License:     MIT
// Author:      Hoffman
// GitHub:      https://github.com/JokerRound
// Create Time: 2018-11-30
// Description: 
//      The header file of class CWinThreadpool.
//
// Modify Log:
//      2018-11-30    Hoffman
//      Info: a. Add below member methods.
//              a.1. Destroy();
//              a.1. AddWorkItem();
//              a.1. DeleteWorkItem();
//              a.1. SubmitWorkItem();
//              a.1. NoWorkItem();
//            b. Add below member variables.
//              b.1. m_mapWorkItem;
//            c. Add below type.
//              c.1. enum tagWorkItemType.
//
//      2018-12-02    Hoffman
//      Info: a. Add below member methods.
//              a.1. WaitForWorkItem();
//
//      2018-12-18    Hoffman
//      Info: a. Add below member methods.
//              a.1. FindWorkItem();
//              a.2. UpdateWorkItem();
//******************************************************************************

#pragma once
#ifndef WINTHREADPOOL_H_
#define WINTHREADPOOL_H_
#include "stdafx.h"
#include <map>
#include "Work.h"

typedef enum tagWorkItemType
{
    WIT_FILETRANSPORT,

    TOTAL_NUM_WIT,
} WORKITEMTYPE, *PWORKITEMTYPE;

class CWinThreadpool
{
private:
    // The type of file work item.
    // note: You should modify it when you modify the WORKITEMTYPE.
    WORKITEMTYPE m_eFirstWorkItemType = WIT_FILETRANSPORT;
    std::map<WORKITEMTYPE, CWork *> m_mapWorkItem;

    //**************************************************************************
    // FUNCTION:    Delete all work item and free resource.
    // NOTE:        
    //      1. This functions will delete the heap memory point.
    //**************************************************************************
    void Destroy() noexcept;
public:
    CWinThreadpool() noexcept;
    virtual ~CWinThreadpool() noexcept;

    //**************************************************************************
    // FUNCTION:    Add a work item and hold it.
    // RETURN:      None
    // PARAMETER:   
    //      pWorkItem:  The object point of class CWork.
    // NOTE:        
    //      1. The pWorkItem can't be NULL.
    //      2. The pWorkItem must be a heap memory point.
    //**************************************************************************
    void AddWorkItem(_In_ WORKITEMTYPE eWorkItemType,
                     _In_ CWork *pWorkItem) noexcept;

    //**************************************************************************
    // FUNCTION:    Delete a work item by type.
    // PARAMETER:   
    //      eWorkItemType:      The type of work item to delete.
    //      bCancelPendingWork: Cancel the work item has had 
    //                          in execute queue or not, default it's TRUE.
    // NOTE:
    //      1. It's a block function.
    //**************************************************************************
    void DeleteWorkItem(_In_ WORKITEMTYPE eWorkItemType,
                        _In_ BOOL bCancelPendingWork = TRUE) noexcept;


    //**************************************************************************
    // FUNCTION:    Using the work item type to find in the map.
    // RETURN:      TRUE is find target work item, FALSE is not find.
    // PARAMETER:   
    //      eWorkItemType:  The type of target work item.
    //**************************************************************************
    BOOL FindWorkItem(_In_ WORKITEMTYPE eWorkItemType) noexcept;

    //**************************************************************************
    // FUNCTION:    Update a existed work item object.
    // PARAMETER:   
    //      eWorkItemType:  The type of target work item.
    //      pWorkItem:      The new work item object.
    //**************************************************************************
    BOOL UpdateWorkItem(_In_ WORKITEMTYPE eWorkItemType, 
                        _In_ CWork *pWorkItem) noexcept;

    //**************************************************************************
    // FUNCTION:    Submit a work item by type.
    // PARAMETER:   
    //      eWorkItemType:  The type of work item to submit.
    //**************************************************************************
    void SubmitWorkItem(_In_ WORKITEMTYPE eWorkItemType) noexcept;

    //**************************************************************************
    // FUNCTION:    Clear all work item holded.
    //**************************************************************************
    void Clear() noexcept;

    //**************************************************************************
    // FUNCTION:    Clear all work item holded.
    // RETURN:      No work item (TRUE) or have work item (FALSE).
    //**************************************************************************
    BOOL NoWorkItem() const noexcept;


    //**************************************************************************
    // FUNCTION:    Wait for a work item has finished.
    // PARAMETER:   
    //      eWorkItemType:  The type of work item you want to wait.
    //                      It's TOTAL_NUM_WIT default, wait for all work item.
    // NOTE:        
    //      1. It's a block function.
    //**************************************************************************
    void WaitForWorkItem(WORKITEMTYPE eWorkItemType = TOTAL_NUM_WIT);

};

#endif // !WINTHREADPOOL_H_
