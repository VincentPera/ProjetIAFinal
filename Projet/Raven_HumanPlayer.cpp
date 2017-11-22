#include "Raven_HumanPlayer.h"

Raven_HumanPlayer::Raven_HumanPlayer(Raven_Game* _world, Vector2D pos)
	:Raven_Bot(_world,pos)
{
	m_bPossessed = true;
}
