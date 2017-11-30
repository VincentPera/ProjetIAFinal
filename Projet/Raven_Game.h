#ifndef RAVEN_ENV
#define RAVEN_ENV
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Game.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   this class creates and stores all the entities that make up the
//          Raven game environment. (walls, bots, health etc) and can read a
//          Raven map file and recreate the necessary geometry.
//
//          this class has methods for updating the game entities and for
//          rendering them.
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <list>

#include "graph/SparseGraph.h"
#include "Raven_ObjectEnumerations.h"
#include "2d/Wall2D.h"
#include "misc/utils.h"
#include "game/EntityFunctionTemplates.h"
#include "Raven_Bot.h"
#include "navigation/pathmanager.h"
#include "Raven_HumanPlayer.h"
#include "TeamSimple.h"


class BaseGameEntity;
class Raven_Projectile;
class Raven_Map;
class GraveMarkers;

enum GAME_MODE {DEATH_MATCH = 0, TEAM_MATCH = 1, SOLO = 2};

class Raven_Game
{
private:

  //the current game map
  Raven_Map*                       m_pMap;
 
  //a list of all the bots that are inhabiting the map
  std::list<Raven_Bot*>            m_Bots;

  //the user may select a bot to control manually. This is a pointer to that
  //bot
  Raven_Bot*                       m_pSelectedBot;
  
  //this list contains any active projectiles (slugs, rockets,
  //shotgun pellets, etc)
  std::list<Raven_Projectile*>     m_Projectiles;

  //this class manages all the path planning requests
  PathManager<Raven_PathPlanner>*  m_pPathManager;

  //We implement for this game a human player
  Raven_Bot*				       m_ThePlayer;


  //if true the game will be paused
  bool                             m_bPaused;

  //if true a bot is removed from the game
  bool                             m_bRemoveABot;

  //when a bot is killed a "grave" is displayed for a few seconds. This
  //class manages the graves
  GraveMarkers*                    m_pGraveMarkers;

  //current mode used in the game
  GAME_MODE                        m_mode;
  int                              m_human;			//if there are a human player
  int                              m_learning_bot;	//if there are a learning bot
  int                              m_strategy_j1;	  //strategie of the player1
  int                              m_strategy_j2;	  //strategie of the player2
  int                              m_strategy_t1;	  //strategie of the team1
  int                              m_strategy_t2;	  //strategie of the team2
  int							   m_isRecording;

  //teams
  std::vector<TeamSimple*> m_teams;

  //output for the neural network
  std::ofstream m_outputFile;
  bool			hasShot;

  //this iterates through each trigger, testing each one against each bot
  void  UpdateTriggers();

  //deletes all entities, empties all containers and creates a new navgraph 
  void  Clear();

  //attempts to position a spawning bot at a free spawn point. returns false
  //if unsuccessful 
  bool AttemptToAddBot(Raven_Bot* pBot);

  bool AttemptToAddBotTeam(Raven_Bot* pBot);

  //when a bot is removed from the game by a user all remaining bots
  //must be notified so that they can remove any references to that bot from
  //their memory
  void NotifyAllBotsOfRemoval(Raven_Bot* pRemovedBot)const;

  //used when we want to assign spawnpoints to teams
  void AddSpawnPointsTeams();

  int mod(int a, int b);
  
public:
  
  Raven_Game();
  Raven_Game(int mode, int human, int grenades, int learning_bot, int strategie_j1,
			 int strategie_j2, int strategie_t1, int strategie_t2, int isRecording);
  ~Raven_Game();

  //the usual suspects
  void Render();
  void Update();

  //loads an environment from a file
  bool LoadMap(const std::string& FileName); 

  //used for refactoring when introducing teams
  void AddBots(unsigned int NumBotsToAdd);
  void AddBotsTeam(unsigned int NumBotsToAdd);
  void AddBot(Raven_Bot* rb);

  //Neural network methods
  void OpenFile(std::string fileName);
  void CloseFile();
  // method used to write a line of the current game into a file
  void WriteLine();

  void AddHumanPlayer();
  void AddBotApprenant();
  void AddRocket(Raven_Bot* shooter, Vector2D target);
  void AddRailGunSlug(Raven_Bot* shooter, Vector2D target);
  void AddShotGunPellet(Raven_Bot* shooter, Vector2D target);
  void AddBolt(Raven_Bot* shooter, Vector2D target);
  void AddBomb(Raven_Bot* shooter, Vector2D target);

  //removes the last bot to be added
  void RemoveBot();

  //returns true if a bot of size BoundingRadius cannot move from A to B
  //without bumping into world geometry
  bool isPathObstructed(Vector2D A, Vector2D B, double BoundingRadius = 0)const;

  //returns a vector of pointers to bots in the FOV of the given bot
  std::vector<Raven_Bot*> GetAllBotsInFOV(const Raven_Bot* pBot)const;

  //returns true if the second bot is unobstructed by walls and in the field
  //of view of the first.
  bool        isSecondVisibleToFirst(const Raven_Bot* pFirst,
                                     const Raven_Bot* pSecond)const;

  //returns true if the ray between A and B is unobstructed.
  bool        isLOSOkay(Vector2D A, Vector2D B)const;

  //starting from the given origin and moving in the direction Heading this
  //method returns the distance to the closest wall
  double       GetDistanceToClosestWall(Vector2D Origin, Vector2D Heading)const;

  
  //returns the position of the closest visible switch that triggers the
  //door of the specified ID
  Vector2D GetPosOfClosestSwitch(Vector2D botPos, unsigned int doorID)const;

  //given a position on the map this method returns the bot found with its
  //bounding radius of that position.If there is no bot at the position the
  //method returns NULL
  Raven_Bot*  GetBotAtPosition(Vector2D CursorPos)const;


  void        TogglePause(){m_bPaused = !m_bPaused;}

  // Give the possibility to move with key instead of clicking with the mouse
  void Raven_Game::MoveToward(Vector2D dir);
  
  //this method is called when the user clicks the right mouse button.
  //The method checks to see if a bot is beneath the cursor. If so, the bot
  //is recorded as selected.If the cursor is not over a bot then any selected
  // bot/s will attempt to move to that position.
  void        ClickRightMouseButton(POINTS p);

  //this method is called when the user clicks the left mouse button. If there
  //is a possessed bot, this fires the weapon, else does nothing
  void        ClickLeftMouseButton(POINTS p);

  void		ScrollMouseButton(bool scrollUp);

  //when called will release any possessed bot from user control
  void        ExorciseAnyPossessedBot();
 
  //if a bot is possessed the keyboard is polled for user input and any 
  //relevant bot methods are called appropriately
  void        GetPlayerInput()const;
  Raven_Bot*  PossessedBot()const{return m_pSelectedBot;}
  void        ChangeWeaponOfPossessedBot(unsigned int weapon)const;

  
  const Raven_Map* const                   GetMap()const{return m_pMap;}
  Raven_Map* const                         GetMap(){return m_pMap;}
  const std::list<Raven_Bot*>&             GetAllBots()const{return m_Bots;}
  PathManager<Raven_PathPlanner>* const    GetPathManager(){return m_pPathManager;}
  int                                      GetNumBots()const{return m_Bots.size();}

  
  void  TagRaven_BotsWithinViewRange(BaseGameEntity* pRaven_Bot, double range)
              {TagNeighbors(pRaven_Bot, m_Bots, range);}  
};





#endif