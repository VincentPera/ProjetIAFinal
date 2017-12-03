#include "ManipulateurFichier.h"

ManipulateurFichier::ManipulateurFichier()
{
}


ManipulateurFichier::~ManipulateurFichier()
{
}

void ManipulateurFichier::FillWeightsValues(vector<vector<vector<double>>> &weightsValues)
{
	vector<vector<vector<double>>> result;
	// TODO
	weightsValues.clear();
	string contenu;
	ifstream file(m_filepath, ios::in);
	if (file)
	{
		string ligne = contenu;

		while (getline(file, contenu))
		{
			vector<string> rawLine;
			vector<string> parsedSlashLine;
			vector<vector<double>> parsedSlashAndWeightDouble;
			split(contenu, "/", parsedSlashLine);
			for each (string s in parsedSlashLine)
			{
				vector<string> parsedWeight;
				vector<double> parsedWeightDouble;
				split(s, ";", parsedWeight);
				for each (string s in parsedWeight)
				{
					parsedWeightDouble.push_back(stod(s));
				}
				parsedSlashAndWeightDouble.push_back(parsedWeightDouble);
			}
			result.push_back(parsedSlashAndWeightDouble);
		}
		weightsValues = result;
	}


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
		file.close();
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

void ManipulateurFichier::InitFile(string _filepath)
{
	m_filepath = _filepath;
	ifstream file(_filepath, ios::in);
	assert(file);
}

void ManipulateurFichier::OpenFile(std::ofstream& file, std::string fileName) {
	file.open(fileName);
}

void ManipulateurFichier::CloseFile(std::ofstream& file) {
	file.close();
}