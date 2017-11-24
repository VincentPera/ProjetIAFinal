#include "Net.h"
#include <assert.h>


Net::Net(const vector<unsigned> &topology)
{
	unsigned numLayers = topology.size();
	for (unsigned layerNum = 0; layerNum < numLayers; ++layerNum)
	{
		m_Layers.push_back(Layer());

		//the number of output connection is equal to the number of neuron of the next layer 
		unsigned numOutputs = layerNum == topology.size() - 1 ? 0 : topology[layerNum + 1];
		//we made a new layer, now fill it with neurons, and add 
		//a bias neuron to the layer
		for (unsigned neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum)
		{
			m_Layers.back().push_back(Neuron(numOutputs, neuronNum));
		}

		// Force the bias node's output value to 1.0 , it is the last neuron created above
		m_Layers.back().back().SetOutputVal(1.0);
	}
}

void Net::FeedForward(const vector<double> &inputVals)
{
	//check if the number of input values fit the number of input neurons
	//(counting the bias neuron)(m_Layers.size()-1)

	assert(inputVals.size() == m_Layers[0].size() - 1);

	for (unsigned i = 0; i < inputVals.size(); ++i)
	{
		m_Layers[0][i].SetOutputVal(inputVals[i]);
	}

	//forward propagate
	//the aim of this step is to pass to each neuron of all the layer his input
	//and transform the outpu in new input for the next layer of neurons
	for (unsigned layerNum = 1; layerNum < m_Layers.size(); ++layerNum)
	{
		Layer &prevLayer = m_Layers[layerNum - 1];
		for (unsigned n = 0; n < m_Layers[layerNum].size() - 1; ++n)
		{
			m_Layers[layerNum][n].FeedForward(prevLayer);
		}
	}
}

void Net::BackProp(const vector<double> &targetVals) 
{
	// Calculate overall net error (RMS (root mean square) of output neuron error)
	//rms = sqrt((1/n(sum(i->1->n)(target(i)-actual(i))²)
	Layer &outputLayer = m_Layers.back();
	m_error = 0.0;

	for (unsigned n = 0; n < outputLayer.size() - 1; ++n)
	{
		double delta = targetVals[n] - outputLayer[n].GetOutputVal();
		m_error += delta * delta;
	}
	m_error /= outputLayer.size() - 1; // get the average error squared
	m_error = sqrt(m_error); // RMS

	// Implement a recent average measurement of the precedent error
	m_recentAverageError = (m_recentAverageError * m_recentAverageSmoothingFactor + m_error)
		/ (m_recentAverageSmoothingFactor + 1.0);


	// Calculate output layer gradients

	for (unsigned n = 0; n < outputLayer.size() - 1; ++n)
	{
		outputLayer[n].CalcOutputGradients(targetVals[n]);
	}

	// Calculate gradient on hidden layers

	for (unsigned layerNum = m_Layers.size() -2; layerNum > 0; --layerNum)
	{
		Layer &hiddenLayer = m_Layers[layerNum];
		Layer &nextLayer = m_Layers[layerNum + 1];

		for(unsigned n = 0; n < hiddenLayer.size(); ++n)
		{
			hiddenLayer[n].CalcHiddenGradients(nextLayer);
		}
	}

	// For all layers from ouputs to first hidden layer
	// Update the connection weights

	for (unsigned layerNum = m_Layers.size() - 1; layerNum > 0; --layerNum)
	{
		Layer &layer = m_Layers[layerNum];
		Layer &prevLayer = m_Layers[layerNum - 1];

		for (unsigned n = 0; n < layer.size() - 1; ++n)
		{
			layer[n].UpdateInputWeights(prevLayer);
		}
	}
}

void Net::GetResult(vector<double> &resultVals) const 
{
	resultVals.clear();
	for (unsigned n = 0; n < m_Layers.back().size() - 1; ++n) 
	{
		resultVals.push_back(m_Layers.back()[n].GetOutputVal());
	}
}

Net::~Net()
{
}
