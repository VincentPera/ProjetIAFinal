#include "Raven_HumanPlayer.h"
#include "Debug/DebugConsole.h"
#include "lua/Raven_Scriptor.h"
#include "Raven_SensoryMemory.h"

Raven_HumanPlayer::Raven_HumanPlayer(Raven_Game* _world, Vector2D pos)
	:Raven_Bot(_world,pos)
{
	m_bPossessed = true;

	//create variables useful for the training
	m_pTargSys = new Raven_TargetingSystem(this);
	m_pSensoryMem = new Raven_SensoryMemory(this, script->GetDouble("Bot_MemorySpan"));
}