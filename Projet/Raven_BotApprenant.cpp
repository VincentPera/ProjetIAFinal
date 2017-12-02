#include "Raven_BotApprenant.h"
#include "Raven_Bot.h"

#include "Raven_SteeringBehaviors.h"
#include "time/Regulator.h"
//Weapon system
#include "Raven_WeaponSystem.h"
#include "Raven_SensoryMemory.h"

//Necessaire pour m_pBrain
#include "goals/Raven_Goal_Types.h"
#include "goals/Goal_Think.h"

Raven_BotApprenant::Raven_BotApprenant(Raven_Game* world, Vector2D v) :Raven_Bot(world, v)
{
	vector<unsigned int> topology;
	topology.push_back(3);
	topology.push_back(2);
	topology.push_back(1);
	this->m_neuralNet = new Net(topology);
}


Raven_BotApprenant::~Raven_BotApprenant()
{
}



//function to override to fit with neural net
void Raven_BotApprenant::Update() 
{
	//process the currently active goal. Note this is required even if the bot
	//is under user control. This is because a goal is created whenever a user 
	//clicks on an area of the map that necessitates a path planning request.
	m_pBrain->Process();

	//Calculate the steering force and update the bot's velocity and position
	UpdateMovement();

	//if the bot is under AI control but not scripted
	if (!isPossessed())
	{
		//examine all the opponents in the bots sensory memory and select one
		//to be the current target
		if (m_pTargetSelectionRegulator->isReady())
		{
			m_pTargSys->Update();
		}

		//appraise and arbitrate between all possible high level goals
		if (m_pGoalArbitrationRegulator->isReady())
		{
			m_pBrain->Arbitrate();
		}

		//update the sensory memory with any visual stimulus
		if (m_pVisionUpdateRegulator->isReady())
		{
			m_pSensoryMem->UpdateVision();
		}

		//select the appropriate weapon to use from the weapons currently in
		//the inventory
		if (m_pWeaponSelectionRegulator->isReady())
		{
			m_pWeaponSys->SelectWeapon();
		}

		//this method aims the bot's current weapon at the current target
		//and takes a shot if a shot is possible
		//m_pWeaponSys->TakeAimAndShoot();
	}
}
