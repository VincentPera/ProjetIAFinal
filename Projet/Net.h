#pragma once
#include "Neuron.h"
#include <vector>

using namespace std;

class Neuron;

typedef vector<Neuron> Layer;

class Net
{
public:
	Net(const vector<unsigned> &topology);
	void FeedForward(const vector<double> &inputVals);
	void BackProp(const vector<double> &targetVals);
	void GetResult(vector<double> &resultVals) const;
	~Net();
private:
	vector<Layer> m_Layers; // m_layers[layersNum][neuronNum]
};

