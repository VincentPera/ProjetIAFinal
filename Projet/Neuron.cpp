#include "Neuron.h"
#include <cmath>

using namespace std;

Neuron::Neuron(unsigned numOutputs, unsigned myIndex)
{
	for (unsigned c = 0; c < numOutputs; ++c)
	{
		m_outputWeights.push_back(Connection());
		m_outputWeights.back().weight = RandomWeight();
	}

	m_myIndex = myIndex;
}


Neuron::~Neuron()
{
}

//make the sum of the product of all the weight by their input value
void Neuron::FeedForward(const Layer &prevLayer)
{
	double sum = 0.0;
	//sum the previous layer's output (which become inputs of the next layer)
	//include the bias node from the previous layer
	for (unsigned n = 0; n < prevLayer.size(); ++n)
	{
		sum += prevLayer[n].GetOutputVal() *
			prevLayer[n].m_outputWeights[m_myIndex].weight;
	}

	m_outputVal = TransferFunction(sum);
}

double Neuron::TransferFunction(double x)
{
	//hyperbolic transfer function 
	// tanh - output range [-1.0..1.0]
	return tanh(x);
}

double Neuron::TransferFunctionDerivative(double x) {
	//tanh approximated derivative
	return 1.0 - x * x;
}