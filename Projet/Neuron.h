#pragma once
#include <vector>
#include <cstdlib>
#include "main.h"


using namespace std;

struct Connection
{
	double weight;
	double deltaWeight;
};

class Neuron
{
public:
	Neuron(unsigned numOutputs, unsigned myIndex);
	~Neuron();
	
	void SetOutputVal(double val) { m_outputVal = val; }
	double GetOutputVal() const { return m_outputVal; }
    void FeedForward(const Layer &prevLayer);

private:
	static double RandomWeight(void) { return rand() / double(RAND_MAX); }
	
	double m_outputVal;
	vector<Connection> m_outputWeights;
	unsigned m_myIndex;

	static double TransferFunction(double x);
	static double TransferFunctionDerivative(double x);
};

