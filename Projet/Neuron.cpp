#include "Neuron.h"
#include <cmath>

using namespace std;

double Neuron::eta = 0.15; // overall net training rate
double Neuron::alpha = 0.5; // multiplier of last weight change (momentum)



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

void Neuron::UpdateInputWeights(Layer &prevLayer)
{
	// The weights to be updated are in the connection container
	// in the neurons in the preceding layer

	for (unsigned n = 0; n < prevLayer.size(); ++n)
	{
		Neuron &neuron = prevLayer[n];
		double oldDeltaWeight = neuron.m_outputWeights[m_myIndex].deltaWeight;

		double newDeltaWeight =
			//Individual input, magnified by the gradient and train rate
			eta // learning rate (0.0 - slow learner, 0.2 - medium learner, 1.0 - reckless learner
			* neuron.GetOutputVal()
			* m_gradient
			// Also add momentum = a fraction of the previous delta weight
			+ alpha
			* oldDeltaWeight;

		neuron.m_outputWeights[m_myIndex].deltaWeight = newDeltaWeight;
		neuron.m_outputWeights[m_myIndex].weight += newDeltaWeight;
	}
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

void Neuron::CalcOutputGradients(double targetVal)
{
	double delta = targetVal - m_outputVal;
	m_gradient = delta * Neuron::TransferFunctionDerivative(m_outputVal);
}

void Neuron::CalcHiddenGradients(const Layer &nextLayer)
{
	double dow = SumDOW(nextLayer);
	m_gradient = dow * Neuron::TransferFunctionDerivative(m_outputVal);
}

double Neuron::SumDOW(const Layer &nextLayer) const
{
	double sum = 0.0;

	//Sum our contributions of the error at the nodes we feed

	for (unsigned n = 0; n < nextLayer.size() - 1; ++n)
	{
		sum += m_outputWeights[n].weight * nextLayer[n].m_gradient;
	}

	return sum;
}