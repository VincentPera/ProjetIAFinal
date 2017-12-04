#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
using namespace std;
class ManipulateurFichier
{
private:
	string m_filepath;
	void split(const string &s, const char* delim, vector<string> &v);
public:
	const string PATH = "Data/";

	ManipulateurFichier();
	~ManipulateurFichier();
	//[numOfFileLine][indexOfInput/output]
	// format [[<inputValue>,<inputValue>,.......,<OutputValue>],.....]
	void FillInputValues(vector<vector<double>> &trainValues);
	void FillWeightsValues(vector<vector<vector<double>>> &weightsValues, vector<unsigned> &topologyValues);
	void InitFile(string _filepath);

	static void ManipulateurFichier::OpenFile(std::ofstream& file, std::string fileName);
	static void ManipulateurFichier::CloseFile(std::ofstream& file);
};

