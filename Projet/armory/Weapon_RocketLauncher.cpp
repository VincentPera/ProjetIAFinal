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

    //fuzzify distance and amount of ammo
    m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);
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
	FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");
	FzSet& Target_Close = DistToTarget.AddLeftShoulderSet("Target_Close", 0, 50, 150);
	FzSet& Target_Medium_P = DistToTarget.AddTriangularSet("Target_Medium_P", 50, 150, 200);
	FzSet& Target_Medium_M = DistToTarget.AddTriangularSet("Target_Medium_M", 150, 200, 300);
	FzSet& Target_Medium_L = DistToTarget.AddTriangularSet("Target_Medium_L", 200, 300, 500);
	FzSet& Target_Far = DistToTarget.AddRightShoulderSet("Target_Far", 300, 500, 1000);


	FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");
	FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 75, 80, 100);
	FzSet& Desirable_P = Desirability.AddTriangularSet("Desirable_P", 50, 60, 75);
	FzSet& Desirable_M = Desirability.AddTriangularSet("Desirable_M", 40, 50, 60);
	FzSet& Desirable_L = Desirability.AddTriangularSet("Desirable_L", 25, 40, 50);
	FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 40);


	FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
	FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 20, 30, 100);
	FzSet& Ammo_Okay_P = AmmoStatus.AddTriangularSet("Ammo_Okay_P", 15, 20, 30);
	FzSet& Ammo_Okay_M = AmmoStatus.AddTriangularSet("Ammo_Okay_M", 10, 15, 20);
	FzSet& Ammo_Okay_L = AmmoStatus.AddTriangularSet("Ammo_Okay_L", 0, 10, 15);
	FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 10);


	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), Desirable_L);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay_P), Desirable_L);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay_M), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay_L), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), Undesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Medium_P, Ammo_Loads), Desirable_P);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_P, Ammo_Okay_P), Desirable_P);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_P, Ammo_Okay_M), Desirable_M);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_P, Ammo_Okay_L), Desirable_M);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_P, Ammo_Low), Desirable_L);

	m_FuzzyModule.AddRule(FzAND(Target_Medium_M, Ammo_Loads), Desirable_P);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_M, Ammo_Okay_P), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_M, Ammo_Okay_M), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_M, Ammo_Okay_L), VeryDesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_M, Ammo_Low), Desirable_P);

	m_FuzzyModule.AddRule(FzAND(Target_Medium_L, Ammo_Loads), Desirable_M);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_L, Ammo_Okay_P), Desirable_M);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_L, Ammo_Okay_M), Desirable_M);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_L, Ammo_Okay_L), Desirable_L);
	m_FuzzyModule.AddRule(FzAND(Target_Medium_L, Ammo_Low), Desirable_L);

	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), Desirable_M);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay_P), Desirable_L);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay_M), Desirable_L);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay_L), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), Undesirable);
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
