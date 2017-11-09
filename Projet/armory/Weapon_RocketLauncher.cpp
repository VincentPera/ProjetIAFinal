#include "Weapon_RocketLauncher.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
RocketLauncher::RocketLauncher(Raven_Bot*   owner):

                      Raven_Weapon(type_rocket_launcher,
                                   script->GetInt("RocketLauncher_DefaultRounds"),
                                   script->GetInt("RocketLauncher_MaxRoundsCarried"),
                                   script->GetDouble("RocketLauncher_FiringFreq"),
                                   script->GetDouble("RocketLauncher_IdealRange"),
                                   script->GetDouble("Rocket_MaxSpeed"),
                                   owner)
{
    //setup the vertex buffer
  const int NumWeaponVerts = 8;
  const Vector2D weapon[NumWeaponVerts] = {Vector2D(0, -3),
                                           Vector2D(6, -3),
                                           Vector2D(6, -1),
                                           Vector2D(15, -1),
                                           Vector2D(15, 1),
                                           Vector2D(6, 1),
                                           Vector2D(6, 3),
                                           Vector2D(0, 3)
                                           };
  for (int vtx=0; vtx<NumWeaponVerts; ++vtx)
  {
    m_vecWeaponVB.push_back(weapon[vtx]);
  }

  //setup the fuzzy module
  InitializeFuzzyModule();

}


//------------------------------ ShootAt --------------------------------------
//-----------------------------------------------------------------------------
inline void RocketLauncher::ShootAt(Vector2D pos)
{ 
  if (NumRoundsRemaining() > 0 && isReadyForNextShot())
  {
    //fire off a rocket!
    m_pOwner->GetWorld()->AddRocket(m_pOwner, pos);

    m_iNumRoundsLeft--;

    UpdateTimeWeaponIsNextAvailable();

    //add a trigger to the game so that the other bots can hear this shot
    //(provided they are within range)
    m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("RocketLauncher_SoundRange"));
  }
}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
double RocketLauncher::GetDesirability(double DistToTarget)
{
  if (m_iNumRoundsLeft == 0)
  {
    m_dLastDesirabilityScore = 0;
  }
  else
  {
	int m_HealthBot = m_pOwner->Health();

    //fuzzify distance and amount of ammo
    m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
	m_FuzzyModule.Fuzzify("HealthPoints", (double)m_HealthBot);
    m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);
    m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
  }

  return m_dLastDesirabilityScore;
}

//-------------------------  InitializeFuzzyModule ----------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void RocketLauncher::InitializeFuzzyModule()
{
  int max_health = script->GetInt("Bot_MaxHealth");
  int min_health = 0;
  int mid_health = intpart((double)max_health/2.0);
  int mid_up_health = intpart(((double)max_health + (double)mid_health) / 2.0);
  int mid_down_health = intpart((double)mid_health / 2.0);

  FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");
  FzSet& Target_Close = DistToTarget.AddLeftShoulderSet("Target_Close",0,25,150);
  FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium",25,150,300);
  FzSet& Target_Far = DistToTarget.AddRightShoulderSet("Target_Far",150,300,1000);

  FuzzyVariable& HealthPoints = m_FuzzyModule.CreateFLV("HealthPoints");
  FzSet& Health_Low = HealthPoints.AddLeftShoulderSet("Health_Low", min_health, mid_down_health, mid_health);
  FzSet& Health_Medium = HealthPoints.AddTriangularSet("Health_Medium", mid_down_health, mid_health, mid_up_health);
  FzSet& Health_High = HealthPoints.AddRightShoulderSet("Health_High", mid_health, mid_up_health, max_health);

  FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability"); 
  FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
  FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
  FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

  FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
  FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 10, 30, 100);
  FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 0, 10, 30);
  FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 10);


  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads, Health_Low), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads, Health_Medium), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads, Health_High), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay, Health_Low), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay, Health_Medium), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay, Health_High), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low, Health_Low), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low, Health_Medium), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low, Health_High), Undesirable);

  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads, Health_Low), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads, Health_Medium), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads, Health_High), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay, Health_Low), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay, Health_Medium), VeryDesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay, Health_High), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low, Health_Low), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low, Health_Medium), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low, Health_High), Desirable);

  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads, Health_Low), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads, Health_Medium), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads, Health_High), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay, Health_Low), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay, Health_Medium), Desirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay, Health_High), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low, Health_Low), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low, Health_Medium), Undesirable);
  m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low, Health_High), Undesirable);
}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void RocketLauncher::Render()
{
    m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
                                   m_pOwner->Pos(),
                                   m_pOwner->Facing(),
                                   m_pOwner->Facing().Perp(),
                                   m_pOwner->Scale());

  gdi->RedPen();

  gdi->ClosedShape(m_vecWeaponVBTrans);
}

int intpart(double x)
{
	int part_int = static_cast<int>(x);
	return part_int;
}