#ifndef GOAL_GET_ITEM_WITH_DODGE_H
#define GOAL_GET_ITEM_WITH_DODGE_H
#include "Goal_GetItem.h"


class Goal_GetItemWithDodge : public Goal_GetItem {

private:
    bool        m_bClockwise;
    Vector2D    m_vStrafeTarget;
    bool        isDodging;
    void        Goal_GetItemWithDodge::initGoToItem();
    void        Goal_GetItemWithDodge::initDodge();

public:
    Goal_GetItemWithDodge(
        Raven_Bot* pBot,
        int        item ) : Goal_GetItem( pBot, item ) 
    {
        m_bClockwise = RandBool();
    };
    
    void Activate() override;
    int Process() override;
    void Terminate() override;

};


#endif //GOAL_GET_ITEM_WITH_DODGE_H


