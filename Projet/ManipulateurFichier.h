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
	ManipulateurFichier();
	~ManipulateurFichier();
	//[numOfFileLine][indexOfInput/output]
	// format [[<inputValue>,<inputValue>,.......,<OutputValue>],.....]
	void FillInputValues(vector<vector<double>> &trainValues);
	void InitFile(string _filepath);
};

