#pragma once

#include "Triggers/Trigger.h"
#include "Raven_Bot.h"
#include "Raven_Game.h"

class Trigger_DroppedWeapon : public Trigger<Raven_Bot>
{
private:
	//vrtex buffers for rocket shape
	std::vector<Vector2D>         m_vecRLVB;
	std::vector<Vector2D>         m_vecRLVBTrans;

	unsigned int m_WeaponType;
	int m_Ammo;
	int m_Team;
	Raven_Game* m_pWorld;

public:
	Trigger_DroppedWeapon(Vector2D pos, unsigned int weapon, int ammo, Raven_Game* world, int node_idx);

	void Update();
	void Try(Raven_Bot* bot);
	void Render();
};


