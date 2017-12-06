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
#include <numeric>

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
		double angle = m_pWeaponSys->GetBotAim();
		if (GetTargetBot() != NULL) {
			m_pWeaponSys->TakeAimLearningBot(this, angle);
		}
	}
}

void Raven_BotApprenant::UseNetToShoot(Vector2D pos) {
	// Retrieve all the input of the game that the net need
	vector<double> gameValues;
	vector<double> result;

	// first : if the ennemy is visible
	gameValues.push_back(GetTargetSys()->isTargetWithinFOV());
	// second : ennemy life
	gameValues.push_back(GetTargetBot()->Health() < 100);
	// third : player life
	gameValues.push_back(Health() < 100);
	// fourth : the current weapon used
	gameValues.push_back(GetWeaponSys()->GetCurrentWeaponType());
	// fifth : the distance between the two bots
	gameValues.push_back((GetTargetBot()->Pos() - Pos()).Length() < 100);

	// Put inputs in the net
	GetNet()->FeedForward(vector<double>(gameValues.begin(), gameValues.begin() + m_neuralNet->GetNetInputNumber()));
	// Get the result with those inputs
	GetNet()->GetResult(result);

	if (result[0] > 0.9) {
		// Fire !!
		m_pWeaponSys->ShootAt(pos);
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

	int testNumber = 50;
	int numberDataTraining = 4000;

	// Load dataset to train the net
	READER_FICHIER.InitFile(READER_FICHIER.PATH + "TrainingData/" + inputFileName);
	vector<vector<double>> trainValues;
	READER_FICHIER.FillInputValues(trainValues);

	// Test on the topology
	//StartTrainingTopology(testNumber, numberDataTraining, inputFileName, trainValues);

	// Test on the Alpha
	// StartTrainingAlpha(testNumber, numberDataTraining, inputFileName, trainValues);

	// Test on the Eta
	//StartTrainingEta(testNumber, numberDataTraining, inputFileName, trainValues);

	// Regular training

	// Create the "right" topology of the net
	vector<unsigned> topology;
	topology.push_back(trainValues.at(0).size()-1);
	topology.push_back(3);
	topology.push_back(1);
	assert(topology.size() == 3);

	// Give the topology to the learning agent
	SetNetTopology(topology);

	this->m_neuralNet->SetEta(0.01);

	this->m_neuralNet->SetAlpha(0.00);

	//init another file for the error-analyzing script
	//ofstream pythonFile(READER_FICHIER.PATH + "Errors/TrainingsError" + ttos(i) + ".txt", ios::out);
	ofstream pythonFile(READER_FICHIER.PATH + "Errors/Regular_TrainingsError.txt", ios::out);

	//init another file for the error-analyzing script
	//ofstream pythonFile2(READER_FICHIER.PATH + "Errors/TestsError" + ttos(i) + ".txt", ios::out);
	ofstream pythonFile2(READER_FICHIER.PATH + "Errors/Regular_TestsError.txt", ios::out);

	for (int y = 0; y < testNumber; y++) {
		// Start Training (and create an output file to see the training output)
		// Training on the same dataset (first numberDataTraining lines of the file)
		double random = rand() % trainValues.size() - numberDataTraining + 1;
		double error = TrainingFunction(READER_FICHIER.PATH + "ResultsData/Regular_results_" + inputFileName,
			vector<vector<double>>(trainValues.begin() + random, trainValues.begin() + random + numberDataTraining));

		pythonFile << error << "\n";

		// Now test the net on some data
		double random2 = rand() % trainValues.size() - numberDataTraining + 1;
		double error2 = TestFunction(vector<vector<double>>(trainValues.begin() + random2, trainValues.begin() + random2 + numberDataTraining));

		pythonFile2 << error2 << "\n";
	}

	// Close the error file
	pythonFile.close();

	WriteData(READER_FICHIER.PATH + "WeightsData/Regular_weights_" + inputFileName, topology);

	topology.clear();
}

void Raven_BotApprenant::StartTrainingAlpha(int testsNumber, int numberDataTraining, string inputFileName, vector<vector<double>> trainValues) {

	// Create the topology of the net
	vector<unsigned> topology;
	topology.push_back(trainValues.at(0).size() - 1);
	topology.push_back(3);
	topology.push_back(1);
	assert(topology.size() == 3);

	// Give the topology to the learning agent
	SetNetTopology(topology);

	this->m_neuralNet->SetEta(0.01);

	// Usefull for testing topology influence
	for (float alpha = 0.00; alpha <= 1.0; alpha += 0.25) {

		this->m_neuralNet->SetAlpha(alpha);

		//init another file for the error-analyzing script
		ofstream pythonFile(READER_FICHIER.PATH + "Errors/Alpha" + ttos(alpha) + "_TrainingsError.txt", ios::out);

		//init another file for the error-analyzing script
		ofstream pythonFile2(READER_FICHIER.PATH + "Errors/Alpha" + ttos(alpha) + "_TestsError.txt", ios::out);

		for (int y = 0; y < testsNumber; y++) {
			// Start Training (and create an output file to see the training output)
			// Training on the same dataset (first numberDataTraining lines of the file)
			double random = rand() % trainValues.size() - numberDataTraining + 1;
			double error = TrainingFunction(READER_FICHIER.PATH + "ResultsData/Alpha" + ttos(alpha) + "results.txt",
				vector<vector<double>>(trainValues.begin() + random, trainValues.begin() + random + numberDataTraining));

			pythonFile << error << "\n";

			// Now test the net on some data
			double random2 = rand() % trainValues.size() - numberDataTraining + 1;
			double error2 = TestFunction(vector<vector<double>>(trainValues.begin() + random2, trainValues.begin() + random2 + numberDataTraining));

			pythonFile2 << error2 << "\n";
		}

		// Close the error file
		pythonFile.close();

		WriteData(READER_FICHIER.PATH + "WeightsData/Alpha" + ttos(alpha) + "_weights_" + inputFileName, topology);

	}
	topology.clear();
}

void Raven_BotApprenant::StartTrainingEta(int testsNumber, int numberDataTraining, string inputFileName, vector<vector<double>> trainValues) {
	
	// Create the topology of the net
	vector<unsigned> topology;
	topology.push_back(trainValues.at(0).size() - 1);
	topology.push_back(3);
	topology.push_back(1);
	assert(topology.size() == 3);

	// Give the topology to the learning agent
	SetNetTopology(topology);

	// Usefull for testing topology influence
	for (float eta = 0.01; eta <= 0.16; eta+=0.04) {

		this->m_neuralNet->SetEta(eta);

		//init another file for the error-analyzing script
		ofstream pythonFile(READER_FICHIER.PATH + "Errors/Eta" + ttos(eta) + "_TrainingsError.txt", ios::out);

		//init another file for the error-analyzing script
		ofstream pythonFile2(READER_FICHIER.PATH + "Errors/Eta" + ttos(eta) + "_TestsError.txt", ios::out);

		for (int y = 0; y < testsNumber; y++) {
			// Start Training (and create an output file to see the training output)
			// Training on the same dataset (first numberDataTraining lines of the file)
			double random = rand() % trainValues.size() - numberDataTraining + 1;
			double error = TrainingFunction(READER_FICHIER.PATH + "ResultsData/Eta" + ttos(eta) + "results.txt",
				vector<vector<double>>(trainValues.begin() + random, trainValues.begin() + random + numberDataTraining));

			pythonFile << error << "\n";

			// Now test the net on some data
			double random2 = rand() % trainValues.size() - numberDataTraining + 1;
			double error2 = TestFunction(vector<vector<double>>(trainValues.begin() + random2, trainValues.begin() + random2 + numberDataTraining));

			pythonFile2 << error2 << "\n";
		}

		// Close the error file
		pythonFile.close();

		WriteData(READER_FICHIER.PATH + "WeightsData/Eta" + ttos(eta) + "_weights_" + inputFileName, topology);

	}
	topology.clear();
}

void Raven_BotApprenant::StartTrainingTopology(int testsNumber, int numberDataTraining, string inputFileName, vector<vector<double>> trainValues) {

	vector<unsigned> topology;
	// Usefull for testing topology influence
	for (int i = 1; i < 5; i++) {

		// Create the topology of the net
		topology.push_back(trainValues.at(0).size() - 1);
		topology.push_back(i);		// Change the topology here
		topology.push_back(1);
		assert(topology.size() == 3);

		// Give the topology to the learning agent
		SetNetTopology(topology);

		//init another file for the error-analyzing script
		ofstream pythonFile(READER_FICHIER.PATH + "Errors/Topo" + ttos(i) + "_TrainingsError.txt", ios::out);

		//init another file for the error-analyzing script
		ofstream pythonFile2(READER_FICHIER.PATH + "Errors/Topo" + ttos(i) + "_TestsError.txt", ios::out);

		for (int y = 0; y < testsNumber; y++) {
			// Start Training (and create an output file to see the training output)
			// Training on the same dataset (first numberDataTraining lines of the file)
			double random = rand() % trainValues.size() - numberDataTraining + 1;
			double error = TrainingFunction(READER_FICHIER.PATH + "ResultsData/Topo" + ttos(i) + "results.txt",
				vector<vector<double>>(trainValues.begin() + random, trainValues.begin() + random + numberDataTraining));

			pythonFile << error << "\n";

			// Now test the net on some data
			double random2 = rand() % trainValues.size() - numberDataTraining + 1;
			double error2 = TestFunction(vector<vector<double>>(trainValues.begin() + random2, trainValues.begin() + random2 + numberDataTraining));

			pythonFile2 << error2 << "\n";
		}

		// Close the error file
		pythonFile.close();

		WriteData(READER_FICHIER.PATH + "WeightsData/Topo" + ttos(i) + "_weights_" + inputFileName, topology);

		topology.clear();
	}
}

double Raven_BotApprenant::TestFunction(vector<vector<double>> trainValues) {
	vector<double> resultVals, errorsVals;

	// Start a training pass
	for (int i = 1; i < trainValues.size(); i++) {

		// Put inputs in the net
		GetNet()->FeedForward(vector<double>(trainValues.at(i).begin(), trainValues.at(i).end() - 1));

		// Get the result with those inputs
		GetNet()->GetResult(resultVals);

		// Retrieve errors : Purpose of the test
		errorsVals.push_back(this->GetNet()->getError());

		// No backpropagation here

		resultVals.clear();
	}

	return accumulate(errorsVals.begin(), errorsVals.end(), 0.0) / errorsVals.size();
}

double Raven_BotApprenant::TrainingFunction(string filename, vector<vector<double>> trainValues) {
	vector<double> targetVals, errorsVals;
	vector<double> resultVals = vector<double>(trainValues.at(0).size() - 1);

	// Open a file to print results inside it
	std::ofstream resultFile;
	READER_FICHIER.OpenFile(resultFile, READER_FICHIER.PATH + filename);

	// Start a training pass
	for (int i = 0; i < trainValues.size(); i++) {

		// Put inputs in the net (all except the last one)
		GetNet()->FeedForward(vector<double>(trainValues.at(i).begin(), trainValues.at(i).end() - 1));
		// Get the result with those inputs
		GetNet()->GetResult(resultVals);
		
		// Get the error for graphs
		errorsVals.push_back(this->GetNet()->getError());

		// Use the backpropagation algorithm to adjust the weights of the net
		targetVals.push_back(trainValues.at(i).at(trainValues.at(i).size() - 1));
		GetNet()->BackProp(targetVals);

		targetVals.clear();
		resultVals.clear();
	}

	// Close the results file
	READER_FICHIER.CloseFile(resultFile);

	return accumulate(errorsVals.begin(), errorsVals.end(), 0.0) / errorsVals.size();
}

void Raven_BotApprenant::WriteData(string filename, vector<unsigned> topology) {
	// Change the pointer of the file
	std::ofstream resultFile;
	READER_FICHIER.OpenFile(resultFile, filename);

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

