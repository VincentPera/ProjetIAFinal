#pragma once
#include "Raven_Bot.h"
class Raven_HumanPlayer :
	public Raven_Bot
{
public:
	Raven_HumanPlayer(Raven_Game* world, Vector2D pos);
	~Raven_HumanPlayer();
};

