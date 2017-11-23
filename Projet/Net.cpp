#include "Net.h"



Net::Net(const vector<unsigned> &topology)
{
	unsigned numLayers = topology.size();
	for (unsigned layerNum = 0; layerNum < numLayers; ++layerNum)
	{
		m_Layers.push_back(Layer());
		//we made a new layer, now fill it with neurons, and add 
		//a bias neuron to the layer
		for (unsigned neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum)
		{
			m_Layers.back().push_back(Neuron());
		}

	}

}

void Net::FeedForward(const vector<double> &inputVals) {}

void Net::BackProp(const vector<double> &targetVals) {}

void Net::GetResult(vector<double> &resultVals) const {}

Net::~Net()
{
}
