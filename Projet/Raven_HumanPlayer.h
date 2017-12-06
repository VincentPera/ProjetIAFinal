#pragma once
#include "Raven_Bot.h"
#include "Raven_Game.h"

class Raven_Game;

class Raven_HumanPlayer : public Raven_Bot
{

public :
	Raven_HumanPlayer(Raven_Game* _world, Vector2D pos);
};

