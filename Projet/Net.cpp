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

void Net::BackProp(const vector<double> &targetVals) {}

void Net::GetResult(vector<double> &resultVals) const {}

Net::~Net()
{
}
