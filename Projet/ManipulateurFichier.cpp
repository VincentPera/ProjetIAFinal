#include "ManipulateurFichier.h"

ManipulateurFichier::ManipulateurFichier(const string _filepath)
{
	m_filepath = _filepath;
	ifstream file(_filepath, ios::in);
	assert(file);
}


ManipulateurFichier::~ManipulateurFichier()
{
}

void ManipulateurFichier::FillInputValues(vector<vector<double>> &trainValues)
{
	// clear of the container before inserting the new values or reread the training file 
	trainValues.clear();
	string contenu;
	ifstream file(m_filepath, ios::in);
	if (file)
	{
		string ligne = contenu;

		while (getline(file, contenu))
		{
			//split the actual line into a array of string (not double yet)
			vector<string> parsedLine;
			vector<double> strToDoubleArray;
			split(contenu, ";", parsedLine);
			//need to convert a vector of string into a vector of double (x.xx) (4 chars)
			for each (string s in parsedLine)
			{
				strToDoubleArray.push_back(stod(s));
			}
			// push the converted array to the training value
			trainValues.push_back(strToDoubleArray);
		}
	}
}

void ManipulateurFichier::split(const string &s, const char* delim, vector<string> &v)
{
	// to avoid modifying original string
	// first duplicate the original string and return a char pointer then free the memory
	char * dup = strdup(s.c_str());
	char * token = strtok(dup, delim);
	while (token != NULL) {
		v.push_back(string(token));
		// the call is treated as a subsequent calls to strtok:
		// the function continues from where it left in previous invocation
		token = strtok(NULL, delim);
	}
	free(dup);
}