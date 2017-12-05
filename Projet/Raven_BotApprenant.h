#pragma once
#include "Raven_Bot.h"
#include <vector>
#include <iosfwd>
#include <map>

#include "game/MovingEntity.h"
#include "misc/utils.h"
#include "Raven_TargetingSystem.h"

#include "Net.h"
#include "ManipulateurFichier.h"

#include <iostream>
#include <fstream>


class Raven_PathPlanner;
class Raven_Steering;
class Raven_Game;
class Regulator;
class Raven_Weapon;
struct Telegram;
class Raven_Bot;
class Goal_Think;
class Raven_WeaponSystem;
class Raven_SensoryMemory;
class Net;

class Raven_BotApprenant :
	public Raven_Bot
{
public:
	Raven_BotApprenant(Raven_Game* world, Vector2D v);
	~Raven_BotApprenant();

	ManipulateurFichier READER_FICHIER;

	void		SetNetTopology(vector<unsigned int> topology);
	void		Update();
	Net*		GetNet();
	void		UseNetToShoot();

	void		StartTraining(string inputFileName);
	void		LoadTraining(string inputFileName);
	double		TrainingFunction(int currentTestNumber, string filename, vector<vector<double>> trainValues);
	double		TestFunction(vector<vector<double>> trainValues);
	void		WriteData(string fileName, vector<unsigned> topology);
private:
	//Net neuralNet;
	Net* m_neuralNet;

};

