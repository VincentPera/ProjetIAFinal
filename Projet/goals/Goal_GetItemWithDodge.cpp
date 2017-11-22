#include "Goal_GetItemWithDodge.h"
#include "../Raven_SteeringBehaviors.h"
#include "../navigation/Raven_PathPlanner.h"
#include "Goal_Wander.h"


// Goal_GetItemWithDodge
// The bot go to the item just as Goal_GetItem
// But if he is targetted by an other bot, he dodges and shots him
// If he disapears of FOV, the bot continues to go to the item

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_GetItemWithDodge::Activate() {
    m_iStatus = active;

    // If targeted, init dodge
    if (m_pOwner->GetTargetSys()->isTargetWithinFOV()) {
        initDodge();
        isDodging = true;
    }
    // If not targeted, init mouvement to the item
    else {
        initGoToItem();
        isDodging = false();
    }

}

//------------------------------- Process -------------------------------------
//-----------------------------------------------------------------------------
int Goal_GetItemWithDodge::Process() {

    // Active goal if inactive
    ActivateIfInactive();

    // Abort if the item disapeared
    if ( hasItemBeenStolen() ) {
        Terminate();
    }
    else if (!isDodging && m_pOwner->GetTargetSys()->isTargetWithinFOV()) {
        // Someone targets the bot while getting the item : dodge
        RemoveAllSubgoals();
        initDodge();
        isDodging = true;
    }
    else if (isDodging && !m_pOwner->GetTargetSys()->isTargetWithinFOV()) {
        // The bots was targeted, but not anymore : go to the item 
        m_pOwner->GetSteering()->SeekOff();
        initGoToItem();
        isDodging = false;
    }
    else if ( !isDodging ) {
        // Process the subgoals (if not targeted, ie. while he dodges)
        m_iStatus = ProcessSubgoals();
    }
    else {
        m_iStatus = inactive;
    }
    return m_iStatus;

}

//------------------------------- Terminate -------------------------------------
//-------------------------------------------------------------------------------
void Goal_GetItemWithDodge::Terminate() {
    // Finish dodge
    if ( isDodging ) {
        m_pOwner->GetSteering()->SeekOff();
    }
}


void Goal_GetItemWithDodge::initDodge() {

    m_pOwner->GetSteering()->SeekOn();

    // Try to dodge by the right
    if ( m_bClockwise ) {
        if ( m_pOwner->canStepRight( m_vStrafeTarget ) ) {
            m_pOwner->GetSteering()->SetTarget( m_vStrafeTarget );
        }
        else {
            m_bClockwise = !m_bClockwise;
            m_iStatus = inactive;
        }
    }
    // Try to dodge by the left
    else {
        if ( m_pOwner->canStepLeft( m_vStrafeTarget ) ) {
            m_pOwner->GetSteering()->SetTarget( m_vStrafeTarget );
        }
        else {
            m_bClockwise = !m_bClockwise;
            m_iStatus = inactive;
        }
    }
}

void Goal_GetItemWithDodge::initGoToItem() {
    m_pGiverTrigger = 0;

    //request a path to the item
    m_pOwner->GetPathPlanner()->RequestPathToItem( m_iItemToGet );

    //the bot may have to wait a few update cycles before a path is calculated
    //so for appearances sake it just wanders
    AddSubgoal( new Goal_Wander( m_pOwner ) );
}


