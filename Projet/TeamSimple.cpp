#include "TeamSimple.h"
#include "Messaging/Telegram.h"
#include "Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"
#include "raven_bot.h"

TeamSimple::TeamSimple(Vector2D lootposition,std::string name)
{
	name_team = name;
	lootingLocation = lootposition;
	target = 0;
}

void TeamSimple::UpdateNewTarget(Raven_Bot* new_target,int id_sender) {
	target = new_target; //Add target
	for (int i = 0; i < members.size();i++) { //dispatch message to all members
		Raven_Bot* bot_courant = members[i];
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
			id_sender,
			bot_courant->ID(),
			Msg_UpdatingTarget,
			NO_ADDITIONAL_INFO);
	}
}

void TeamSimple::ClearTarget(int id_sender) {
	target = 0;
	for (int i = 0; i < members.size(); i++) { //dispatch message to all members
		Raven_Bot* bot_courant = members[i];
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
			id_sender,
			bot_courant->ID(),
			Msg_TargetKilled,
			NO_ADDITIONAL_INFO);
	}
}



TeamSimple::~TeamSimple()
{
}
