#pragma once
#include "Raven_Bot.h"
#include <vector>
#include <iosfwd>
#include <map>

#include "game/MovingEntity.h"
#include "misc/utils.h"
#include "Raven_TargetingSystem.h"


class Raven_Game;

class Raven_HumanPlayer : public Raven_Bot
{ 
	static Raven_HumanPlayer *singleton;

	Raven_HumanPlayer(Raven_Game* world = NULL, Vector2D pos = Vector2D()) :Raven_Bot(world, pos) 
	{this->m_bPossessed = true;};

	~Raven_HumanPlayer() {};

	public :
		static void SetInstance(Raven_Game* world)
		{
			singleton->m_pWorld = world;
		}
		static Raven_HumanPlayer* GetInstance() 
		{
			if (!singleton) singleton = new Raven_HumanPlayer();
			return singleton;
		}
		
		void SetParameter(Raven_Game* world) { this->m_pWorld = world; };
};

