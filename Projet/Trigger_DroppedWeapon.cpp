#include "Trigger_DroppedWeapon.h"
#include "lua/Raven_Scriptor.h"
#include "Raven_ObjectEnumerations.h"
#include "Raven_Bot.h"
#include "Raven_WeaponSystem.h"
#include "armory/Weapon_RocketLauncher.h"
#include "armory/Weapon_RailGun.h"
#include "armory/Weapon_ShotGun.h"
#include "armory/Weapon_Blaster.h"
#include "armory/Weapon_Grenade.h"


#include "Debug/DebugConsole.h"

// Constructor
Trigger_DroppedWeapon::Trigger_DroppedWeapon(Vector2D pos, unsigned int weapon, int ammo, Raven_Game* world, int node_idx) :
	Trigger<Raven_Bot>(BaseGameEntity::GetNextValidID()),
	m_WeaponType(weapon),
	m_Ammo(ammo),
	m_pWorld(world)
{
	SetPos(pos);
	SetGraphNodeIndex(node_idx);
	SetEntityType(weapon);
	AddCircularTriggerRegion(pos, 5);

	//create the vertex buffer for the rocket shape
	const int NumRocketVerts = 8;
	const Vector2D rip[NumRocketVerts] = { Vector2D(0, 3),
		Vector2D(1, 2),
		Vector2D(1, 0),
		Vector2D(2, -2),
		Vector2D(-2, -2),
		Vector2D(-1, 0),
		Vector2D(-1, 2),
		Vector2D(0, 3) };

	for (int i = 0; i<NumRocketVerts; ++i)
	{
		m_vecRLVB.push_back(rip[i]);
	}
}

// Update
void Trigger_DroppedWeapon::Update() {
	if (!m_pWorld->isTeamMatch())
	{
		SetToBeRemovedFromGame();
	}
}

// If the item is picked, then erase it
void Trigger_DroppedWeapon::Try(Raven_Bot* bot) {
	if (m_pWorld->isTeamMatch()) {
		if (this->isActive() &&  this->isTouchingTrigger(bot->Pos(), bot->BRadius()))
		{
			bot->GetWeaponSys()->AddWeapon(EntityType());
			debug_con << "Bot " << bot->ID() << " picked a weapon " << EntityType() << "";

			Raven_Weapon* w = 0;

			switch (EntityType())
			{
			case type_rail_gun:

				w = new RailGun(bot); break;


			case type_shotgun:

				w = new ShotGun(bot); break;

			case type_rocket_launcher:

				w = new RocketLauncher(bot); break;

			case type_grenade:

				w = new Grenade(bot); break;

			}//end switch


			 //if the bot holds a weapon of this type, add its ammo
			Raven_Weapon* present = bot->GetWeaponSys()->GetWeaponFromInventory(EntityType());

			if (present)
			{
				present->IncrementRounds(m_Ammo);
			}
			delete w;
			SetToBeRemovedFromGame();
			SetInactive();
		}
	}
}

// Render the weapon
void Trigger_DroppedWeapon::Render()
{
	if (isActive())
	{
		//Raven_Team::PenColor(m_Team);
		gdi->BlackBrush();
		gdi->Circle(m_vPosition, 5);
	}
}