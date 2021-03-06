#include "controller.h"

//remove comparator

void evolve_population(std::vector<Simulation> &simulations, int X, int Y)
{

	//  sort to find the top X performers
	std::sort(simulations.begin(), simulations.end());

	//  delete the nets of the lowest performing half of the population
	int halfPop = 0.5*(simulations.size());
	for (auto it = simulations.begin(); it != simulations.end() - halfPop; ++it) {
		it->destroyNet();
		int random = rand() % X;
		++random;
		it->recreateNet((simulations.end() - random)->getNet());
	}

	//   mutate up to Y randomly chosen nets
	int i = 0;
	while (i < Y) {
		int chosenOne = rand() % simulations.size();
		(simulations.begin() + chosenOne)->mutate();
		i++;
	}

}

/* run simulations for the full number of epochs, performing neuro-evolutionary
   techniques between each epoch */ 
int main(void) {
	//  control experiment data collection
	int MAX_STEPS = 50;
	int NUM_SIMULATIONS = 100;
	int NUM_EPOCHS = 10000;
	int X_TOP_PERFORMERS = 5;
	int Y_MUTATIONS = 100;

	//  control gridworld
	int NUMBER_OF_AGENTS = 2;
	int NUMBER_OF_POI = 1;

	int WORLD_WIDTH = 3;
	int WORLD_HEIGHT = 3;

	int POI_WEIGHT = 2;

	//  control neural nets
	int NUMBER_OF_LAYERS = 3;
	bool RANDOM_WEIGHTS = true;
	double RANDOM_NET_MIN = -0.5;
	double RANDOM_NET_MAX =  0.5;

	//  set up gridworld configuration
	struct gridConfig GC;
	GC.numAgents = NUMBER_OF_AGENTS;
	GC.numPOI = NUMBER_OF_POI;
	GC.width = WORLD_WIDTH;
	GC.height = WORLD_HEIGHT;
	GC.poiWeight = POI_WEIGHT;

	//  set up initial net configuration
	struct netConfig NC;
	NC.net_type = FANN::LAYER;
	NC.num_layers = NUMBER_OF_LAYERS;
	NC.layers = new unsigned int[3];
	NC.layers[0] = 13;
	NC.layers[1] = 8;
	NC.layers[2] = 6;
	NC.randWeights = RANDOM_WEIGHTS;
	NC.randMin = RANDOM_NET_MIN;
	NC.randMax = RANDOM_NET_MAX;

	srand(time(NULL));

	std::vector<Simulation> simulations(NUM_SIMULATIONS, Simulation(GC, NC, MAX_STEPS));

	//  for each learning epoch, we run the set of simulations and 
	//  then evolve the population based on basic neuroevolutionary 
	//  algorithms.
	std::cout << std::endl;
	for (int i = 0; i < NUM_EPOCHS; i++) {

		double avg = 0.0;

		std::cout << "EPOCH " << i << std::endl;
		std::cout << "**********************************" << std::endl;

		//  run each simulation
		for (int j = 0; j < NUM_SIMULATIONS; j++) {
			std::cout << "------------------------------------" << std::endl;
			simulations[j].runEpoch();
			std::cout << "simulation " << j << "   ";
			simulations[j].logResults();
			avg += simulations[j].getReward();
		}

		avg /= NUM_SIMULATIONS;
		std::cout << "EPOCH AVERAGE " << avg << std::endl;
		std::cout << std::endl;

		//printAvgReward(simulations, NUM_SIMULATIONS, i);
		evolve_population(simulations, X_TOP_PERFORMERS, Y_MUTATIONS);
		for (auto it = simulations.begin(); it != simulations.end(); ++it)
		{
			it->reset();
		}
		if (Y_MUTATIONS != 0 )
			Y_MUTATIONS -= 1;
	}

	/* Cleanup configuration memory */
	delete [] NC.layers;

	return 0;
}




