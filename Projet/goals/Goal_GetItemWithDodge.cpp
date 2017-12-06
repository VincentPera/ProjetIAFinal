#include "Goal_GetItemWithDodge.h"
#include "../Raven_SteeringBehaviors.h"
#include "../navigation/Raven_PathPlanner.h"
#include "Debug/DebugConsole.h"
#include "Goal_Wander.h"


// Goal_GetItemWithDodge
// The bot go to the item just as Goal_GetItem
// But if he is targetted by an other bot, he dodges and shots him
// If he disapears of FOV, the bot continues to go to the item

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
void Goal_GetItemWithDodge::Activate() {
    m_iStatus = active;
    currentNbChange = 0;
    isDodging = true;

    // If targeted, init dodge
    if (!isDodging) {
        initDodge();
        isDodging = true;
    }
    // If not targeted, init mouvement to the item
    else {
        initGoToItem();
        isDodging = false;
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
        m_iStatus = failed;

    }
    else if ( currentNbChange >= nbFramesBetweenChange ) {
        // The bot have to change goal
        currentNbChange = 0;
        if ( !isDodging && m_pOwner->GetTargetSys()->isTargetWithinFOV() ) {
            // Someone targets the bot while getting the item : dodge
            RemoveAllSubgoals();
            initDodge();
            isDodging = true;
        }
        else if ( isDodging && !m_pOwner->GetTargetSys()->isTargetWithinFOV() ) {
            // The bots was targeted, but not anymore : go to the item 
            m_pOwner->GetSteering()->SeekOff();
            initGoToItem();
            isDodging = false;
        }
    }
    else if ( !isDodging ) {
        // Process subgoals if going to the item
        m_iStatus = ProcessSubgoals();
        currentNbChange++;
    }
    else if ( isDodging && m_pOwner->isAtPosition( m_vStrafeTarget ) ) {
        // The bot finish the Dodge -> Go to the item
        currentNbChange = 0;
        m_iStatus = completed;
        m_pOwner->GetSteering()->SeekOff();
        initGoToItem();
        isDodging = false;
    }
    else {
        currentNbChange++;
    }
    return m_iStatus;

}

//------------------------------- Terminate -------------------------------------
//-------------------------------------------------------------------------------
void Goal_GetItemWithDodge::Terminate() {
    // Finish the dodge (if dodging)
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


