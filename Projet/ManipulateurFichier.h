#pragma once
#include <string>
#include <vector>
using namespace std;
class ManipulateurFichier
{
private:
	string m_filepath;
	void split(const string &s, const char* delim, vector<string> &v);
public:
	ManipulateurFichier(const string _filepath);
	~ManipulateurFichier();
	void FillInputValues(vector<double> &inputValues);
};

