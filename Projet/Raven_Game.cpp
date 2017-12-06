#include "Raven_Game.h"
#include "Raven_ObjectEnumerations.h"
#include "misc/WindowUtils.h"
#include "misc/Cgdi.h"
#include "Raven_SteeringBehaviors.h"
#include "lua/Raven_Scriptor.h"
#include "navigation/Raven_PathPlanner.h"
#include "game/EntityManager.h"
#include "2d/WallIntersectionTests.h"
#include "Raven_Map.h"
#include "Raven_Door.h"
#include "Raven_UserOptions.h"
#include "Time/PrecisionTimer.h"
#include "Raven_SensoryMemory.h"
#include "Raven_WeaponSystem.h"
#include "messaging/MessageDispatcher.h"
#include "Raven_Messages.h"
#include "GraveMarkers.h"
#include "TeamSimple.h"

#include "armory/Raven_Projectile.h"
#include "armory/Projectile_Rocket.h"
#include "armory/Projectile_Pellet.h"
#include "armory/Projectile_Slug.h"
#include "armory/Projectile_Bolt.h"
#include "armory/Projectile_Bomb.h"

#include "goals/Goal_Think.h"
#include "goals/Raven_Goal_Types.h"

#include "armory/Raven_Weapon.h"

#include "Debug/DebugConsole.h"

//uncomment to write object creation/deletion to debug console
//#define  LOG_CREATIONAL_STUFF


//----------------------------- ctor ------------------------------------------
//-----------------------------------------------------------------------------
Raven_Game::Raven_Game():m_pSelectedBot(NULL),
                         m_bPaused(false),
                         m_bRemoveABot(false),
                         m_pMap(NULL),
                         m_pPathManager(NULL),
                         m_pGraveMarkers(NULL)
{
	m_mode, m_human, m_learning_bot = 0;
	m_isRecording, m_isLearning, m_isUsingWeights = 0;
	m_inputFileName, m_weightFileName = "";

	//load in the default map
	LoadMap(script->GetString("StartMap"));
}

Raven_Game::Raven_Game(int mode, int human, int grenades, int learning_bot, int strategie_j1,
						int strategie_j2, int strategie_t1, int strategie_t2, int isRecording, 
						int isLearning, int isUsingWeights, string inputFileName, string outputFileName,
						string weightFileName):m_pSelectedBot(NULL),
							m_bPaused(false),
							m_bRemoveABot(false),
							m_pMap(NULL),
							m_pPathManager(NULL),
							m_pGraveMarkers(NULL)
{
	hasShot = false;
	m_mode = static_cast<GAME_MODE>(mode);
	m_human = human;
	m_learning_bot = learning_bot;
	m_strategy_players = { strategie_j1, strategie_j2 };
	m_strategy_teams = { strategie_t1, strategie_t2 };
	m_isRecording = isRecording;
	m_isLearning = isLearning;
	m_isUsingWeights = isUsingWeights;
	m_inputFileName = inputFileName;
	m_outputFileName = outputFileName;
	m_weightFileName = weightFileName;

	if (m_mode == TEAM_MATCH) { //Creation of both teams
		if (m_strategy_teams[0] == 0) { // TeamSimple
			Vector2D loot = Vector2D(360, 60); // TODO Change that.
			m_teams.push_back(new TeamSimple(loot, "Alpha"));
		}
		if (m_strategy_teams[1] == 0) { // TeamSimple
			Vector2D loot = Vector2D(60, 360); // TODO change that.
			m_teams.push_back(new TeamSimple(loot, "Beta"));
		}
	}

	if (grenades) {
		//load in the map with grenades in it.
		LoadMap(script->GetString("GrenadeMap"));
	} else {
		//load in the default map
		LoadMap(script->GetString("StartMap"));
	}
}


//------------------------------ dtor -----------------------------------------
//-----------------------------------------------------------------------------
Raven_Game::~Raven_Game()
{
  Clear();
  delete m_pPathManager;
  delete m_pMap;
  
  delete m_pGraveMarkers;
}


//---------------------------- Clear ------------------------------------------
//
//  deletes all the current objects ready for a map load
//-----------------------------------------------------------------------------
void Raven_Game::Clear()
{
#ifdef LOG_CREATIONAL_STUFF
    debug_con << "\n------------------------------ Clearup -------------------------------" <<"";
#endif

  //delete the bots
  std::list<Raven_Bot*>::iterator it = m_Bots.begin();
  for (it; it != m_Bots.end(); ++it)
  {
#ifdef LOG_CREATIONAL_STUFF

    debug_con << "deleting entity id: " << (*it)->ID() << " of type "
              << GetNameOfType((*it)->EntityType()) << "(" << (*it)->EntityType() << ")" <<"";
#endif

    delete *it;
  }

  //delete any active projectiles
  std::list<Raven_Projectile*>::iterator curW = m_Projectiles.begin();
  for (curW; curW != m_Projectiles.end(); ++curW)
  {
#ifdef LOG_CREATIONAL_STUFF
    debug_con << "deleting projectile id: " << (*curW)->ID() << "";
#endif

    delete *curW;
  }

  //clear the containers
  m_Projectiles.clear();
  m_Bots.clear();

  m_pSelectedBot = NULL;


}

//-------------------------------- Update -------------------------------------
//
//  calls the update function of each entity
//-----------------------------------------------------------------------------
void Raven_Game::Update()
{ 
  //don't update if the user has paused the game
  if (m_bPaused) return;

  m_pGraveMarkers->Update();

  //get any player keyboard input
  GetPlayerInput();
  
  //update all the queued searches in the path manager
  m_pPathManager->UpdateSearches();

  //update any doors
  std::vector<Raven_Door*>::iterator curDoor =m_pMap->GetDoors().begin();
  for (curDoor; curDoor != m_pMap->GetDoors().end(); ++curDoor)
  {
    (*curDoor)->Update();
  }

  //update any current projectiles
  std::list<Raven_Projectile*>::iterator curW = m_Projectiles.begin();
  while (curW != m_Projectiles.end())
  {
    //test for any dead projectiles and remove them if necessary
    if (!(*curW)->isDead())
    {
      (*curW)->Update();

      ++curW;
    }
    else
    {    
      delete *curW;

      curW = m_Projectiles.erase(curW);
    }   
  }
  
  //update the bots
  bool bSpawnPossible = true;
  
  std::list<Raven_Bot*>::iterator curBot = m_Bots.begin();
  for (curBot; curBot != m_Bots.end(); ++curBot)
  {
    //if this bot's status is 'respawning' attempt to resurrect it from
    //an unoccupied spawn point
    if ((*curBot)->isSpawning() && bSpawnPossible)
    {
		if (m_mode == TEAM_MATCH)
		{
			bSpawnPossible = AttemptToAddBotTeam(*curBot);
		}
		else 
		{
			bSpawnPossible = AttemptToAddBot(*curBot);
		}
    }
    
    //if this bot's status is 'dead' add a grave at its current location 
    //then change its status to 'respawning'
    else if ((*curBot)->isDead())
    {
      //create a grave
      m_pGraveMarkers->AddGrave((*curBot)->Pos());

      //change its status to spawning
      (*curBot)->SetSpawning();
    }

    //if this bot is alive update it.
    else if ( (*curBot)->isAlive())
    {
      (*curBot)->Update();
    }  
  } 

  //update the triggers
  m_pMap->UpdateTriggerSystem(m_Bots);

  //if the user has requested that the number of bots be decreased, remove one
  if (m_bRemoveABot && m_mode == DEATH_MATCH) // this behavior is only possible in deathmatch game
  { 
    if (!m_Bots.empty())
    {
      Raven_Bot* pBot = m_Bots.back();
      if (pBot == m_pSelectedBot)m_pSelectedBot=0;
      NotifyAllBotsOfRemoval(pBot);
      delete m_Bots.back();
      m_Bots.remove(pBot);
      pBot = 0;
    }

    m_bRemoveABot = false;
  }
}

void Raven_Game::OpenFile(std::string fileName) {
	FILE_CONTROL.OpenFile(m_outputFile, FILE_CONTROL.PATH + "TrainingData/"+fileName);
	//m_outputFile.open("TrainingData/" + fileName);
}

void Raven_Game::CloseFile() {
	FILE_CONTROL.CloseFile(m_outputFile);
}

void Raven_Game::WriteLine() {
	if (m_ThePlayer->GetTargetBot() != NULL) {
		// first : if the ennemy is visible
		m_outputFile << (m_ThePlayer->GetTargetSys()->isTargetWithinFOV()) << ";";
		// second : ennemy life
		m_outputFile << (m_ThePlayer->GetTargetBot()->Health() < 100) << ";";
		// third : player life
		//m_outputFile << (m_ThePlayer->Health() < 50) << ";";
		// fourth : the current weapon used
		//m_outputFile << (m_ThePlayer->GetWeaponSys()->GetCurrentWeaponType()) << ";";
		// fifth : the distance between the two bots
		//m_outputFile << ((m_ThePlayer->GetTargetBot()->Pos() - m_ThePlayer->Pos()).Distance) << ";";
		// last column : if the player shot
		m_outputFile << hasShot << "\n";
	}
	// Reset variables
	hasShot = false;
}


//----------------------------- AttemptToAddBot -------------------------------
//-----------------------------------------------------------------------------
bool Raven_Game::AttemptToAddBot(Raven_Bot* pBot)
{
  //make sure there are some spawn points available
  if (m_pMap->GetSpawnPoints().size() <= 0)
  {
    ErrorBox("Map has no spawn points!"); return false;
  }

  //we'll make the same number of attempts to spawn a bot this update as
  //there are spawn points
  int attempts = m_pMap->GetSpawnPoints().size();

  while (--attempts >= 0)
  { 
    //select a random spawn point
    Vector2D pos = m_pMap->GetRandomSpawnPoint();

    //check to see if it's occupied
    std::list<Raven_Bot*>::const_iterator curBot = m_Bots.begin();

    bool bAvailable = true;

    for (curBot; curBot != m_Bots.end(); ++curBot)
    {
      //if the spawn point is unoccupied spawn a bot
      if (Vec2DDistance(pos, (*curBot)->Pos()) < (*curBot)->BRadius())
      {
        bAvailable = false;
      }
    }

    if (bAvailable)
    {  
      pBot->Spawn(pos);

      return true;   
    }
  }

  return false;
}

//----------------------------- AttemptToAddBotTeam -------------------------------
//-----------------------------------------------------------------------------
bool Raven_Game::AttemptToAddBotTeam(Raven_Bot* pBot)
{
	//make sure there are some spawn points available for the team
	if (pBot->GetTeam()->GetSpawnPoints().size() <= 0)
	{
		ErrorBox("Map has no spawn points!"); return false;
	}

	//we'll make the same number of attempts to spawn a bot this update as
	//there are spawn points for his team
	int attempts = pBot->GetTeam()->GetSpawnPoints().size();

	while (--attempts >= 0)
	{
		//select a random spawn point
		Vector2D pos = pBot->GetTeam()->GetRandomSpawnPoint();

		//check to see if it's occupied
		std::list<Raven_Bot*>::const_iterator curBot = m_Bots.begin();

		bool bAvailable = true;

		for (curBot; curBot != m_Bots.end(); ++curBot)
		{
			//if the spawn point is unoccupied spawn a bot
			if (Vec2DDistance(pos, (*curBot)->Pos()) < (*curBot)->BRadius())
			{
				bAvailable = false;
			}
		}

		if (bAvailable)
		{
			pBot->Spawn(pos);

			return true;
		}
	}

	return false;
}



//-------------------------- AddBotsTeam --------------------------------------
//
//  Adds a bot in a team and switches on the default steering behavior
//-----------------------------------------------------------------------------
void Raven_Game::AddBotsTeam(unsigned int NumBotsToAdd)
{
	int currTeamId = 1;
	while (NumBotsToAdd--)
	{
		//create a bot. (its position is irrelevant at this point because it will
		//not be rendered until it is spawned)
		Raven_Bot* rb = new Raven_Bot(this, Vector2D());
		m_teams.at(currTeamId)->Addmember(rb); //Add to the team
		rb->SetTeam(m_teams.at(currTeamId), 0); //Let the player know his team
		rb->SetBotNumber((NumBotsToAdd % 3) + 1);

		// Add the current bot to the game
		AddBot(rb);

#ifdef LOG_CREATIONAL_STUFF
	debug_con << "Adding bot with ID " << ttos(rb->ID()) << " to team " << teams.at(currTeamId)->GetName() << "";
#endif
		// switch to the next team
		currTeamId = ++currTeamId % 2;
	}
}

//-------------------------- AddBotsSolo --------------------------------------
//
//  Adds a bot with a particular behavior and switches on the default steering behavior
//-----------------------------------------------------------------------------
void Raven_Game::AddBotsSolo(unsigned int NumBotsToAdd)
{
	int currentPlayer = 0;
	while (NumBotsToAdd--)
	{
		//create a bot. (its position is irrelevant at this point because it will
		//not be rendered until it is spawned)
		Raven_Bot* rb = new Raven_Bot(this, Vector2D());
		// Set his behavior
		rb->SetBrainBehavior(m_strategy_players[currentPlayer]);

		// Add the current bot to the game
		AddBot(rb);

		if (m_mode == SOLO && currentPlayer == 0) {
			m_ThePlayer = rb;
		}

#ifdef LOG_CREATIONAL_STUFF
		debug_con << "Adding bot with ID " << ttos(rb->ID()) << " with the behavior " << m_strategy_players[currentPlayer] << "";
#endif
		currentPlayer++;
	}
}

//-------------------------- AddBots --------------------------------------
//
//  Adds a bot and switches on the default steering behavior
//-----------------------------------------------------------------------------
void Raven_Game::AddBots(unsigned int NumBotsToAdd)
{ 
	while (NumBotsToAdd--) {
		//create a bot. (its position is irrelevant at this point because it will
		//not be rendered until it is spawned)
		Raven_Bot* rb = new Raven_Bot(this, Vector2D());

		// Add the current bot to the game
		AddBot(rb);
	}
    
#ifdef LOG_CREATIONAL_STUFF
  debug_con << "Adding bot with ID " << ttos(rb->ID()) << "";
#endif
}

//-------------------------- AddOneBot --------------------------------------
//
//  Add one bot and switches on his default steering behavior
//-----------------------------------------------------------------------------
void Raven_Game::AddBot(Raven_Bot* rb)
{
	//switch the default steering behaviors on
	rb->GetSteering()->WallAvoidanceOn();
	rb->GetSteering()->SeparationOn();

	m_Bots.push_back(rb);

	//register the bot with the entity manager
	EntityMgr->RegisterEntity(rb);
}


//------------------------ AddHumanPlayer-----------------------------------------
// instanciate a single human player controlled by default by the user in the game
//--------------------------------------------------------------------------------
void Raven_Game::AddHumanPlayer()
{
	Raven_HumanPlayer* hP = new Raven_HumanPlayer(this, Vector2D());
	// If the team mode is on
	if (m_mode == TEAM_MATCH) {
		// Add the humain to the alpha team
		m_teams.at(0)->Addmember(hP);
		hP->SetTeam(m_teams.at(0), 0); //Let the humain know his team
	}
	else if (m_mode == SOLO && m_isRecording)
	{
		//get the only bot in the map
		std::list<Raven_Bot*>::const_iterator curBot = m_Bots.begin();

		for (curBot; curBot != m_Bots.end(); ++curBot)
			if((*curBot) != hP)
				hP->GetTargetSys()->SetTarget((*curBot));
	}
	m_Bots.push_back(hP);
	m_ThePlayer = hP;
	EntityMgr->RegisterEntity(hP);
}

//------------------ AddBotApprenant ----------------------------------------
// Create a new BotApprenant
//---------------------------------------------------------------------------
void Raven_Game::AddBotApprenant() {
	// Create the learning bot
	Raven_BotApprenant* rBa = new Raven_BotApprenant(this, Vector2D());

	rBa->BecomeLearner();
	if (m_isLearning) {
		rBa->StartTraining(m_inputFileName);
	} else {
		// Load weights to start the game right now
		rBa->LoadTraining(m_weightFileName);
	}
	
	// Add the bot into the context
	m_Bots.push_back(rBa);
	EntityMgr->RegisterEntity(rBa);
}

//---------------------------- NotifyAllBotsOfRemoval -------------------------
//
//  when a bot is removed from the game by a user all remianing bots
//  must be notifies so that they can remove any references to that bot from
//  their memory
//-----------------------------------------------------------------------------
void Raven_Game::NotifyAllBotsOfRemoval(Raven_Bot* pRemovedBot)const
{
    std::list<Raven_Bot*>::const_iterator curBot = m_Bots.begin();
    for (curBot; curBot != m_Bots.end(); ++curBot)
    {
      Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
                              SENDER_ID_IRRELEVANT,
                              (*curBot)->ID(),
                              Msg_UserHasRemovedBot,
                              pRemovedBot);

    }
}
//-------------------------------RemoveBot ------------------------------------
//
//  removes the last bot to be added from the game
//-----------------------------------------------------------------------------
void Raven_Game::RemoveBot()
{
  m_bRemoveABot = true;
}

//--------------------------- AddBolt -----------------------------------------
//-----------------------------------------------------------------------------
void Raven_Game::AddBolt(Raven_Bot* shooter, Vector2D target)
{
	if (shooter == m_ThePlayer) {
		hasShot = true;
	}
  Raven_Projectile* rp = new Bolt(shooter, target);

  m_Projectiles.push_back(rp);
  
  #ifdef LOG_CREATIONAL_STUFF
  debug_con << "Adding a bolt " << rp->ID() << " at pos " << rp->Pos() << "";
  #endif
}

//------------------------------ AddRocket --------------------------------
void Raven_Game::AddRocket(Raven_Bot* shooter, Vector2D target)
{
	if (shooter == m_ThePlayer) {
		hasShot = true;
	}
  Raven_Projectile* rp = new Rocket(shooter, target);

  m_Projectiles.push_back(rp);
  
  #ifdef LOG_CREATIONAL_STUFF
  debug_con << "Adding a rocket " << rp->ID() << " at pos " << rp->Pos() << "";
  #endif
}

//------------------------- AddRailGunSlug -----------------------------------
void Raven_Game::AddRailGunSlug(Raven_Bot* shooter, Vector2D target)
{
	if (shooter == m_ThePlayer) {
		hasShot = true;
	}
  Raven_Projectile* rp = new Slug(shooter, target);

  m_Projectiles.push_back(rp);
  
  #ifdef LOG_CREATIONAL_STUFF
  debug_con << "Adding a rail gun slug" << rp->ID() << " at pos " << rp->Pos() << "";
#endif
}

//------------------------- AddShotGunPellet -----------------------------------
void Raven_Game::AddShotGunPellet(Raven_Bot* shooter, Vector2D target)
{
	if (shooter == m_ThePlayer) {
		hasShot = true;
	}
  Raven_Projectile* rp = new Pellet(shooter, target);

  m_Projectiles.push_back(rp);
  
  #ifdef LOG_CREATIONAL_STUFF
  debug_con << "Adding a shotgun shell " << rp->ID() << " at pos " << rp->Pos() << "";
#endif
}

//------------------------------ AddBomb --------------------------------
void Raven_Game::AddBomb(Raven_Bot* shooter, Vector2D target)
{
	if (shooter == m_ThePlayer) {
		hasShot = true;
	}
	Raven_Projectile* rp = new Bomb(shooter, target);

	m_Projectiles.push_back(rp);

#ifdef LOG_CREATIONAL_STUFF
	debug_con << "Adding a grenade bomb " << rp->ID() << " at pos " << rp->Pos() << "";
#endif
}


//----------------------------- GetBotAtPosition ------------------------------
//
//  given a position on the map this method returns the bot found with its
//  bounding radius of that position.
//  If there is no bot at the position the method returns NULL
//-----------------------------------------------------------------------------
Raven_Bot* Raven_Game::GetBotAtPosition(Vector2D CursorPos)const
{
  std::list<Raven_Bot*>::const_iterator curBot = m_Bots.begin();

  for (curBot; curBot != m_Bots.end(); ++curBot)
  {
    if (Vec2DDistance((*curBot)->Pos(), CursorPos) < (*curBot)->BRadius())
    {
      if ((*curBot)->isAlive())
      {
        return *curBot;
      }
    }
  }

  return NULL;
}

//-------------------------------- LoadMap ------------------------------------
//
//  sets up the game environment from map file
//-----------------------------------------------------------------------------
bool Raven_Game::LoadMap(const std::string& filename)
{  
  //clear any current bots and projectiles
  Clear();
  
  //out with the old
  delete m_pMap;
  delete m_pGraveMarkers;
  delete m_pPathManager;

  //in with the new
  m_pGraveMarkers = new GraveMarkers(script->GetDouble("GraveLifetime"));
  m_pPathManager = new PathManager<Raven_PathPlanner>(script->GetInt("MaxSearchCyclesPerUpdateStep"));
  m_pMap = new Raven_Map();

  //make sure the entity manager is reset
  EntityMgr->Reset();


  //load the new map data
  if (m_pMap->LoadMap(filename)) {
	  char * numbots;
	  numbots = (char *)malloc(strlen("NumBots") + 1);
	  strcpy(numbots, "NumBots");
	  strcat(numbots, std::to_string(m_mode).c_str());

	  if (m_mode == TEAM_MATCH) {
		  // AddSpawnPoints to each team
		  AddSpawnPointsTeams();
		  AddBotsTeam(script->GetInt(numbots) - m_human);
	  }
	  else if (m_mode == SOLO) {
		  AddBotsSolo(script->GetInt(numbots) - m_human);
	  } else {
		  AddBots(script->GetInt(numbots) - m_human);
	  }
	  if (m_human) {
		  AddHumanPlayer();
		  m_pSelectedBot = m_ThePlayer;
	  }
	  if (m_learning_bot) {
		  AddBotApprenant();
	  }
    return true;
  }

  return false;
}



//---------------------------- AddSpawnPointsTeam -----------------------------
//
//  Called when we want to assign spawn point to teams
//-----------------------------------------------------------------------------
void Raven_Game::AddSpawnPointsTeams() {
	int NumSpawnPoints = m_pMap->GetSpawnPoints().size();
	int currTeamId = 0;
	while (NumSpawnPoints--)
	{
		//assign the current SpawnPoint to the team
		m_teams.at(currTeamId)->AddSpawnPoint(m_pMap->GetSpawnPoints().at(NumSpawnPoints)); //Add to the team

#ifdef LOG_CREATIONAL_STUFF
		debug_con << "Adding spawn point (" << m_pMap->GetSpawnPoints().at(NumSpawnPoints).x << "," <<
			m_pMap->GetSpawnPoints().at(NumSpawnPoints).y << ") to team " << teams.at(currTeamId)->GetName() << "";
#endif
		// switch to the next team
		currTeamId = ++currTeamId % 2;
	}
}



//------------------------- ExorciseAnyPossessedBot ---------------------------
//
//  when called will release any possessed bot from user control
//-----------------------------------------------------------------------------
void Raven_Game::ExorciseAnyPossessedBot()
{
  if (m_pSelectedBot) m_pSelectedBot->Exorcise();
}

void Raven_Game::MoveToward(Vector2D dir)
{
	//clear any current goals
	m_pSelectedBot->GetSteering()->WallAvoidanceOn();
	m_pSelectedBot->GetBrain()->RemoveAllSubgoals();
	m_pSelectedBot->GetBrain()->AddGoal_SeekToPosition(Vector2D(m_pSelectedBot->Pos().x + dir.x, m_pSelectedBot->Pos().y + dir.y));
}

//-------------------------- ClickRightMouseButton -----------------------------
//
//  this method is called when the user clicks the right mouse button.
//
//  the method checks to see if a bot is beneath the cursor. If so, the bot
//  is recorded as selected.
//
//  if the cursor is not over a bot then any selected bot/s will attempt to
//  move to that position.
//-----------------------------------------------------------------------------
void Raven_Game::ClickRightMouseButton(POINTS p)
{
  Raven_Bot* pBot = m_ThePlayer;

  //if there is no selected bot just return;
  //if (!pBot && m_pSelectedBot == NULL) return;

  //if the cursor is over a different bot to the existing selection,
  //change selection

  //-------------------------------------------------------------
  //With a human player we assume you cannot select another bot
  //-------------------------------------------------------------

  /*if (pBot && pBot != m_pSelectedBot)
  { 
    if (m_pSelectedBot) m_pSelectedBot->Exorcise();
    m_pSelectedBot = pBot;

    return;
  }*/

  //if the user clicks on a selected bot twice it becomes possessed(under
  //the player's control)
  //if (pBot && pBot == m_pSelectedBot)
  //{
  //  m_pSelectedBot->TakePossession();

  //  //clear any current goals
  //  m_pSelectedBot->GetBrain()->RemoveAllSubgoals();
  //}

  //if the bot is possessed then a right click moves the bot to the cursor
  //position
  if (m_human) {
	  if (m_pSelectedBot->isPossessed())
	  {
		  //if the shift key is pressed down at the same time as clicking then the
		  //movement command will be queued
		  if (IS_KEY_PRESSED('Q'))
		  {
			  m_pSelectedBot->GetBrain()->QueueGoal_MoveToPosition(POINTStoVector(p));
		  }
		  else
		  {
			  //clear any current goals
			  m_pSelectedBot->GetBrain()->RemoveAllSubgoals();
			  m_pSelectedBot->GetBrain()->AddGoal_MoveToPosition(POINTStoVector(p));
		  }
	  }
  }
}

//---------------------- ClickLeftMouseButton ---------------------------------
//-----------------------------------------------------------------------------
void Raven_Game::ClickLeftMouseButton(POINTS p)
{
  if (m_pSelectedBot && m_pSelectedBot->isPossessed())
  {
	hasShot = true;
    m_pSelectedBot->FireWeapon(POINTStoVector(p));
  }
}





//---------------------- ScrollMouseButton ---------------------------------
//-----------------------------------------------------------------------------
void Raven_Game::ScrollMouseButton(bool scrollUp)
{
	// New weapons should be add at the end of the vector !
	std::vector<unsigned int> armes = { type_rail_gun, type_rocket_launcher, type_shotgun, type_blaster, type_grenade };

	if (m_human)
	{
		// Get the ref of the current weapon
		Raven_Weapon* current_weapon = PossessedBot()->GetWeaponSys()->GetCurrentWeapon();
		int indice = (current_weapon->GetType() - 6);
		do
		{
			// Get the next weapon on the vector
			if (scrollUp) {
				indice++;
			} else {
				indice--;
			}
			indice = mod(indice, armes.size());
		} while (PossessedBot()->GetWeaponSys()->GetWeaponFromInventory(armes.at(indice)) == NULL);

		// Change the weapon
		PossessedBot()->ChangeWeapon(armes.at(indice));
	}
}

int Raven_Game::mod(int a, int b) {
	return a >= 0 ? a % b : (b - abs(a%b)) % b;
}

//------------------------ GetPlayerInput -------------------------------------
//
//  if a bot is possessed the keyboard is polled for user input and any 
//  relevant bot methods are called appropriately
//-----------------------------------------------------------------------------
void Raven_Game::GetPlayerInput()const
{
  if (m_pSelectedBot && m_pSelectedBot->isPossessed())
  {
      m_pSelectedBot->RotateFacingTowardPosition(GetClientCursorPosition());
   }
}


//-------------------- ChangeWeaponOfPossessedBot -----------------------------
//
//  changes the weapon of the possessed bot
//-----------------------------------------------------------------------------
void Raven_Game::ChangeWeaponOfPossessedBot(unsigned int weapon)const
{
  //ensure one of the bots has been possessed
  if (m_pSelectedBot)
  {
    switch(weapon)
    {
    case type_blaster:
      
      PossessedBot()->ChangeWeapon(type_blaster); return;

    case type_shotgun:
      
      PossessedBot()->ChangeWeapon(type_shotgun); return;

    case type_rocket_launcher:
      
      PossessedBot()->ChangeWeapon(type_rocket_launcher); return;

    case type_rail_gun:
      
      PossessedBot()->ChangeWeapon(type_rail_gun); return;

	case type_grenade:

	  PossessedBot()->ChangeWeapon(type_grenade); return;

    }
  }
}

//---------------------------- isLOSOkay --------------------------------------
//
//  returns true if the ray between A and B is unobstructed.
//------------------------------------------------------------------------------
bool Raven_Game::isLOSOkay(Vector2D A, Vector2D B)const
{
  return !doWallsObstructLineSegment(A, B, m_pMap->GetWalls());
}

//------------------------- isPathObstructed ----------------------------------
//
//  returns true if a bot cannot move from A to B without bumping into 
//  world geometry. It achieves this by stepping from A to B in steps of
//  size BoundingRadius and testing for intersection with world geometry at
//  each point.
//-----------------------------------------------------------------------------
bool Raven_Game::isPathObstructed(Vector2D A,
                                  Vector2D B,
                                  double    BoundingRadius)const
{
  Vector2D ToB = Vec2DNormalize(B-A);

  Vector2D curPos = A;

  while (Vec2DDistanceSq(curPos, B) > BoundingRadius*BoundingRadius)
  {   
    //advance curPos one step
    curPos += ToB * 0.5 * BoundingRadius;
    
    //test all walls against the new position
    if (doWallsIntersectCircle(m_pMap->GetWalls(), curPos, BoundingRadius))
    {
      return true;
    }
  }

  return false;
}

void Raven_Game::ActiveFlocking(bool flock) {
	teamFlocking = flock;
	if (flock) {
		std::list<Raven_Bot*>::const_iterator it = m_Bots.begin();
		for (it; it != m_Bots.end(); ++it) {
			(*it)->GetSteering()->CohesionOn();
			(*it)->GetSteering()->AlignmentOn();
		}
	}
	else {
		std::list<Raven_Bot*>::const_iterator it = m_Bots.begin();
		for (it; it != m_Bots.end(); ++it) {
			(*it)->GetSteering()->CohesionOff();
			(*it)->GetSteering()->AlignmentOff();
		}
	}
}

//----------------------------- GetAllBotsInFOV ------------------------------
//
//  returns a vector of pointers to bots within the given bot's field of view
//-----------------------------------------------------------------------------
std::vector<Raven_Bot*>
Raven_Game::GetAllBotsInFOV(const Raven_Bot* pBot)const
{
  std::vector<Raven_Bot*> VisibleBots;

  std::list<Raven_Bot*>::const_iterator curBot = m_Bots.begin();
  for (curBot; curBot != m_Bots.end(); ++curBot)
  {
    //make sure time is not wasted checking against the same bot or against a
    // bot that is dead or re-spawning
    if (*curBot == pBot ||  !(*curBot)->isAlive()) continue;

    //first of all test to see if this bot is within the FOV
    if (isSecondInFOVOfFirst(pBot->Pos(),
                             pBot->Facing(),
                             (*curBot)->Pos(),
                             pBot->FieldOfView()))
    {
      //cast a ray from between the bots to test visibility. If the bot is
      //visible add it to the vector
      if (!doWallsObstructLineSegment(pBot->Pos(),
                              (*curBot)->Pos(),
                              m_pMap->GetWalls()))
      {
        VisibleBots.push_back(*curBot);
      }
    }
  }

  return VisibleBots;
}

//---------------------------- isSecondVisibleToFirst -------------------------

bool
Raven_Game::isSecondVisibleToFirst(const Raven_Bot* pFirst,
                                   const Raven_Bot* pSecond)const
{
  //if the two bots are equal or if one of them is not alive return false
  if ( !(pFirst == pSecond) && pSecond->isAlive())
  {
    //first of all test to see if this bot is within the FOV
    if (isSecondInFOVOfFirst(pFirst->Pos(),
                             pFirst->Facing(),
                             pSecond->Pos(),
                             pFirst->FieldOfView()))
    {
      //test the line segment connecting the bot's positions against the walls.
      //If the bot is visible add it to the vector
      if (!doWallsObstructLineSegment(pFirst->Pos(),
                                      pSecond->Pos(),
                                      m_pMap->GetWalls()))
      {
        return true;
      }
    }
  }

  return false;
}

//--------------------- GetPosOfClosestSwitch -----------------------------
//
//  returns the position of the closest visible switch that triggers the
//  door of the specified ID
//-----------------------------------------------------------------------------
Vector2D 
Raven_Game::GetPosOfClosestSwitch(Vector2D botPos, unsigned int doorID)const
{
  std::vector<unsigned int> SwitchIDs;
  
  //first we need to get the ids of the switches attached to this door
  std::vector<Raven_Door*>::const_iterator curDoor;
  for (curDoor = m_pMap->GetDoors().begin();
       curDoor != m_pMap->GetDoors().end();
       ++curDoor)
  {
    if ((*curDoor)->ID() == doorID)
    {
       SwitchIDs = (*curDoor)->GetSwitchIDs(); break;
    }
  }

  Vector2D closest;
  double ClosestDist = MaxDouble;
  
  //now test to see which one is closest and visible
  std::vector<unsigned int>::iterator it;
  for (it = SwitchIDs.begin(); it != SwitchIDs.end(); ++it)
  {
    BaseGameEntity* trig = EntityMgr->GetEntityFromID(*it);

    if (isLOSOkay(botPos, trig->Pos()))
    {
      double dist = Vec2DDistanceSq(botPos, trig->Pos());

      if ( dist < ClosestDist)
      {
        ClosestDist = dist;
        closest = trig->Pos();
      }
    }
  }

  return closest;
}




    
//--------------------------- Render ------------------------------------------
//-----------------------------------------------------------------------------
void Raven_Game::Render()
{
  m_pGraveMarkers->Render();
  
  //render the map
  m_pMap->Render();

  //render all the bots unless the user has selected the option to only 
  //render those bots that are in the fov of the selected bot
  if (m_pSelectedBot && UserOptions->m_bOnlyShowBotsInTargetsFOV)
  {
    std::vector<Raven_Bot*> 
    VisibleBots = GetAllBotsInFOV(m_pSelectedBot);

    std::vector<Raven_Bot*>::const_iterator it = VisibleBots.begin();
    for (it; it != VisibleBots.end(); ++it) (*it)->Render();

    if (m_pSelectedBot) m_pSelectedBot->Render();
  }

  else
  {
    //render all the entities
    std::list<Raven_Bot*>::const_iterator curBot = m_Bots.begin();
    for (curBot; curBot != m_Bots.end(); ++curBot)
    {
      if ((*curBot)->isAlive())
      {
        (*curBot)->Render();
      }
    }
  }
  
  //render any projectiles
  std::list<Raven_Projectile*>::const_iterator curW = m_Projectiles.begin();
  for (curW; curW != m_Projectiles.end(); ++curW)
  {
    (*curW)->Render();
  }

 // gdi->TextAtPos(300, WindowHeight - 70, "Num Current Searches: " + ttos(m_pPathManager->GetNumActiveSearches()));

  //render a red circle around the selected bot (blue if possessed)
  if (m_pSelectedBot)
  {
    if (m_pSelectedBot->isPossessed())
    {
      gdi->BluePen(); gdi->HollowBrush();
      gdi->Circle(m_pSelectedBot->Pos(), m_pSelectedBot->BRadius()+1);
    }
    else
    {
      gdi->RedPen(); gdi->HollowBrush();
      gdi->Circle(m_pSelectedBot->Pos(), m_pSelectedBot->BRadius()+1);
    }


    if (UserOptions->m_bShowOpponentsSensedBySelectedBot)
    {
      m_pSelectedBot->GetSensoryMem()->RenderBoxesAroundRecentlySensed();
    }

    //render a square around the bot's target
    if (UserOptions->m_bShowTargetOfSelectedBot && m_pSelectedBot->GetTargetBot())
    {  
      
      gdi->ThickRedPen();

      Vector2D p = m_pSelectedBot->GetTargetBot()->Pos();
      double   b = m_pSelectedBot->GetTargetBot()->BRadius();
      
      gdi->Line(p.x-b, p.y-b, p.x+b, p.y-b);
      gdi->Line(p.x+b, p.y-b, p.x+b, p.y+b);
      gdi->Line(p.x+b, p.y+b, p.x-b, p.y+b);
      gdi->Line(p.x-b, p.y+b, p.x-b, p.y-b);
    }



    //render the path of the bot
    if (UserOptions->m_bShowPathOfSelectedBot)
    {
      m_pSelectedBot->GetBrain()->Render();
    }  
    
    //display the bot's goal stack
    if (UserOptions->m_bShowGoalsOfSelectedBot)
    {
      Vector2D p(m_pSelectedBot->Pos().x -50, m_pSelectedBot->Pos().y);

      m_pSelectedBot->GetBrain()->RenderAtPos(p, GoalTypeToString::Instance());
    }

    if (UserOptions->m_bShowGoalAppraisals)
    {
      m_pSelectedBot->GetBrain()->RenderEvaluations(5, 415);
    } 
    
    if (UserOptions->m_bShowWeaponAppraisals)
    {
      m_pSelectedBot->GetWeaponSys()->RenderDesirabilities();
    }

   if (IS_KEY_PRESSED('Q') && m_pSelectedBot->isPossessed())
    {
      gdi->TextColor(255,0,0);
      gdi->TextAtPos(GetClientCursorPosition(), "Queuing");
    }
  }
}

bool Raven_Game::isTeamMatch()
{
	if (m_mode == TEAM_MATCH) {
		return true;
	}
	else {
		return false;
	}
	
}