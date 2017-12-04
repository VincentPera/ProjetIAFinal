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

#include "Debug/DebugConsole.h"

Raven_BotApprenant::Raven_BotApprenant(Raven_Game* world, Vector2D v) :Raven_Bot(world, v)
{
}

Raven_BotApprenant::~Raven_BotApprenant()
{
}

void Raven_BotApprenant::SetNetTopology(vector<unsigned int> topology) {
	this->m_neuralNet = new Net(topology);
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
		// Instead call the neuron network to see if he should shoot or not
		if (GetTargetBot() != NULL) {
			UseNetToShoot();
		}
	}
}

void Raven_BotApprenant::UseNetToShoot() {
	// Retrieve all the input of the game that the net need
	vector<double> gameValues;
	vector<double> result;

	// first : if the ennemy is visible
	gameValues.push_back(GetTargetSys()->isTargetWithinFOV());
	// second : ennemy life
	gameValues.push_back(GetTargetBot()->Health() < 50);
	// third : player life
	gameValues.push_back(Health() < 50);
	// fourth : the current weapon used
	gameValues.push_back(GetWeaponSys()->GetCurrentWeaponType());
	// fifth : the distance between the two bots
	gameValues.push_back((GetTargetBot()->Pos() - Pos()).Length());

	// Put inputs in the net
	GetNet()->FeedForward(vector<double>(gameValues.begin(), gameValues.begin() + m_neuralNet->GetNetInputNumber() - 1));
	// Get the result with those inputs
	GetNet()->GetResult(result);

	if (result[0] > 0.9) {
		// Fire !!
		m_pWeaponSys->ShootAt(GetTargetBot()->Pos());
	}
	else {
		// Not the right situation => don't shoot.
	}
}

Net* Raven_BotApprenant::GetNet()
{
	return m_neuralNet;
}

void Raven_BotApprenant::StartTraining(string inputFileName) {
	debug_con << "Bot " << this->ID() << " is learning how to shoot!" << "";

	// Load dataset from the human player
	READER_FICHIER.InitFile(READER_FICHIER.PATH + "TrainingData/" + inputFileName);
	//init another file for the python-analyzing data script (error)
	ofstream pythonFile("Data/TrainingData/Error.txt", ios::out);


	vector<vector<double>> trainValues;
	READER_FICHIER.FillInputValues(trainValues);

	// Create the topology of the net
	vector<unsigned> topology;
	topology.push_back(trainValues.at(0).size()-1);
	topology.push_back(2);
	topology.push_back(1);

	// Give the topology to the learning agent
	SetNetTopology(topology);

	// Open a file to print results inside it
	std::ofstream resultFile;
	READER_FICHIER.OpenFile(resultFile, READER_FICHIER.PATH + "ResultsData/results_" + inputFileName);
	int trainingPass = 0;
	vector<double> resultVals, targetVals;

	// Start Training (and create an output file to see the training output)
	for (int i = 1; i < trainValues.size(); i++) {
		++trainingPass;
		resultFile << endl << "Pass " << trainingPass << "\nInputs: ";
		for (int y = 0; y < trainValues.at(i).size() - 1; y++) {
			resultFile << trainValues.at(i).at(y) << ", ";
		}
		resultFile << "\nTarget: " << trainValues.at(i).at(trainValues.at(i).size() - 1) << "\n";
		targetVals.push_back(trainValues.at(i).at(trainValues.at(i).size() - 1));
		trainValues.at(i).pop_back();

		// Put inputs in the net
		GetNet()->FeedForward(trainValues.at(i));
		// Get the result with those inputs
		GetNet()->GetResult(resultVals);

		resultFile << "Output: " << resultVals.at(0) << "\n";
		double error = trainValues.at(i).at(trainValues.at(i).size() - 1) - resultVals.at(0);
		resultFile << "Erreur: " << error << "\n";
		pythonFile << error << "\n";
		// Use the backpropagation algorithm to adjust the weights of the net
		GetNet()->BackProp(targetVals);
		targetVals.clear();
	}
	pythonFile.close();

	// Close the results file
	READER_FICHIER.CloseFile(resultFile);
	
	// Change the pointer of the file
	READER_FICHIER.OpenFile(resultFile, READER_FICHIER.PATH + "WeightsData/weights_" + inputFileName);

	// Write the topology of the net first
	for (int i = 0; i < topology.size(); i++) {
		resultFile << topology.at(i);
		if (i != topology.size() - 1)
			resultFile << ";";
	}
	resultFile << "\n";

	// Write all the weight of the net
	vector<vector<vector<double>>> weights = GetNet()->GetWeights();
	for (int i = 0; i < weights.size(); ++i) {
		for (int j = 0; j < weights.at(i).size(); ++j) {
			for (int z = 0; z < weights.at(i).at(j).size(); ++z) {
				resultFile << weights.at(i).at(j).at(z);
				if (z != weights.at(i).at(j).size() - 1)
					resultFile << ";";
			}
			resultFile << "/";
		}
		resultFile << "\n";
	}
	READER_FICHIER.CloseFile(resultFile);
}

void Raven_BotApprenant::LoadTraining(string inputFileName) {
	debug_con << "Bot " << this->ID() << " is using prepared weights to shoot!" << "";
	// Change the file pointer
	READER_FICHIER.InitFile(READER_FICHIER.PATH + "WeightsData/" + inputFileName);

	// Retrieve the topology of the net
	vector<unsigned> topology;

	// Load weights and the topology from a previous training
	vector<vector< vector<double>>> weightsValues;
	READER_FICHIER.FillWeightsValues(weightsValues, topology);

	// Give the topology to the learning agent
	SetNetTopology(topology);

	// instanciate weigths
	GetNet()->SetWeights(weightsValues);
}

