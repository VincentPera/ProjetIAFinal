#include "ManipulateurFichier.h"

ManipulateurFichier::ManipulateurFichier(const string _filepath)
{
	m_filepath = _filepath;
}


ManipulateurFichier::~ManipulateurFichier()
{
}

void ManipulateurFichier::FillInputValues(vector<double> &inputValues)
{

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