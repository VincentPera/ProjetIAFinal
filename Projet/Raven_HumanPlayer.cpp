#include "Raven_HumanPlayer.h"



Raven_HumanPlayer::Raven_HumanPlayer(Raven_Game* world, Vector2D pos) :Raven_Bot(world, pos)
{
	this->m_bPossessed = true;
}


Raven_HumanPlayer::~Raven_HumanPlayer()
{
}
