#include "TeamSimple.h"
#include "Messaging/Telegram.h"
#include "Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"
#include "raven_bot.h"

TeamSimple::TeamSimple()
{
}

TeamSimple::TeamSimple(Vector2D lootposition, std::string name)
{
	members = std::list<Raven_Bot*>();
	name_team = name;
	lootingLocation = lootposition;
	target = 0;
}

void TeamSimple::UpdateNewTarget(Raven_Bot* new_target,int id_sender) {
	target = new_target; //Add target
	std::list<Raven_Bot*>::iterator curBot = members.begin();
	Raven_Bot* leader = *curBot;
	this->AddTeamLeader(leader);
	for (curBot; curBot != members.end(); ++curBot) { //dispatch message to all members
		Raven_Bot* bot_courant = *curBot;
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
			id_sender,
			bot_courant->ID(),
			Msg_UpdatingTarget,
			NO_ADDITIONAL_INFO);
	}
}

void TeamSimple::ClearTarget(int id_sender) {
	target = 0;
	std::list<Raven_Bot*>::iterator curBot = members.begin();
	Raven_Bot* leader = *curBot;
	this->AddTeamLeader(leader);
	for (curBot; curBot != members.end(); ++curBot) { //dispatch message to all members
		Raven_Bot* bot_courant = *curBot;
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
			id_sender,
			bot_courant->ID(),
			Msg_TargetKilled,
			NO_ADDITIONAL_INFO);
	}
}

void TeamSimple::AddSpawnPoint(Vector2D spawnPoint) {
	m_SpawnPoints.push_back(spawnPoint);
}


void TeamSimple::Addmember(Raven_Bot * new_memeber)
{
	members.push_back(new_memeber);
}

TeamSimple::~TeamSimple()
{
}
