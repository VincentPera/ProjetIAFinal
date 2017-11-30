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
	ManipulateurFichier(const string _filepath);
	~ManipulateurFichier();
	//[numOfFileLine][indexOfInput/output]
	void FillInputValues(vector<vector<double>> &inputValues);
};

