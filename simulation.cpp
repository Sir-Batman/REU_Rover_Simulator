/*********************************************************************
* simulation.cpp
*
* This class represents a single simulation, which can manipulate and 
* step a gridworld for a full epoch.
*
* Copyright (C) 2016 Abby Van Soest, Connor Yates

*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "simulation.h"
#include <iostream>

/* Calls the non-default constructors on the members with predetermined, 
 * almost arbitrary values */
Simulation::Simulation()
	: world(2, 1, 5, 5, 1, "")
{
	//std::cout << "call to default constructor" << std::endl;
	this->net = new FANN::neural_net(FANN::LAYER, 3, (const unsigned int[]) {13,9,4});
	this->reward = 0;
	this->timesteps = 250;
}

/* This non default constructor uses the information provided by the configuration structs 
 * to call the subsequent non default constructors for the members */
Simulation::Simulation(struct gridConfig GC, struct netConfig NC, int timesteps, std::string pickupNet)
	: world(GC.numAgents, GC.numPOI, GC.width, GC.height, GC.poiWeight, pickupNet)
{
	//std::cout << "Call to non default constructor" << std::endl;
	this->net = new FANN::neural_net(NC.net_type, NC.num_layers, NC.layers);

	if (NC.randWeights) { this->net->randomize_weights(NC.randMin, NC.randMax); }
	this->timesteps = timesteps; 
	this->reward = 0;

}

// destructor
Simulation::~Simulation()
{
	//std::cout << "Call to destructor" << std::endl;
	delete this->net;
	this->net = NULL;
}

//  copy constructor
Simulation::Simulation(const Simulation& that)
{
	//std::cout << "call to copy constructor" << std::endl;
	this->net = new FANN::neural_net(*that.net);
	this->reward = that.reward;
	this->world = that.world;
	this->timesteps = that.timesteps;
}

// copy assignment operator
Simulation& Simulation::operator=(const Simulation& that)
{
	//std::cout << "Call to equal operator" << std::endl;
	if (this->net != NULL) { delete this->net; }
    this->net = new FANN::neural_net(*that.net);
	this->reward = that.reward;
	this->world = that.world;
	this->timesteps = that.timesteps;
    return *this;
}

// override relational operator based on rewards received
bool Simulation::operator<(const Simulation &rhs) const
{
	return this->reward < rhs.reward;
}

//  print the results from the simulation's learning epoch
void Simulation::logResults()
{
	std::cout << "Reward: " << this->getReward() << 
	"    Returned: " << this->world.currentAmount() << "   Steps: " << this->world.stepsTaken() << std::endl;
	if (this->world.currentAmount() > 0)
	{
		std::cout << "SUCCESS OF SOME SORT!" << std::endl;
	}
}

/* Runs a single epoch, which runs for a given number of timesteps */
// runs the simulation until the time runs out or the simulation ends prematurely
int Simulation::runEpoch()
{
	int prev = 0;
	int steps = 0;
	double eps = 0.1;
	for (steps = 0; steps < this->timesteps; ++steps)
	{

		this->world.stepAgents(this->net, eps);

		int check = this->world.currentAmount();
		if (check > prev) {
			this->reward += 100;
			prev = check;
		}
	
		if (this->world.worldComplete())
		{
			this->world.printWorld();
			break;
		}

		this->reward += (-0.05);
	}

	return 0;
}

int Simulation::runEpochAndPrint()
{
	int prev = 0;
	int steps = 0;
	int count = 0;
	double eps = 0.1;
	for (steps = 0; steps < this->timesteps; ++steps)
	{
		//if (count < 20) {
			this->world.printWorld();
		//	count++;
		//}
		this->world.stepAgents(this->net, eps);

		int check = this->world.currentAmount();
		if (check > prev) {
			this->reward += 100;
			prev = check;
		}
	
		if (this->world.worldComplete())
		{
			this->world.printWorld();
			break;
		}

		this->reward += (-0.05);
	}

	return 0;
}

int Simulation::getSteps() {
	return this->world.stepsTaken();
}

//  return the reward the simulation earned in the epoch
double Simulation::getReward() 
{
	return this->reward;
}

/* Resets the gridworld and the statistics variables internal to the class */
void Simulation::reset()
{
	this->world.reset();
	this->reward = 0;
}

//  destroy the simulation's old net (for use during evolution)
void Simulation::destroyNet() {
	delete this->net;
	this->net = NULL;
}

//  give the simualtion a new neural net based on the net provided
//  (for use during evolution)
void Simulation::recreateNet(FANN::neural_net* net) {
	if (this->net == NULL) {
		this->net = new FANN::neural_net(*net);
	}
	else
	{
		std::cout << "Net not NULL in recreateNet()" << std::endl;
	}
}

// return a pointer to the simulation's current net
FANN::neural_net* Simulation::getNet() {
	return this->net;
}
// set the simulation's current net pointer to the one provided
void Simulation::setNet(FANN::neural_net* net)
{
	if (this->net == NULL)
	{
		this->net = net;
	}
	else
	{
		std::cout << "Net not NULL in setNet()" << std::endl;
	}
}

/* Performs an in place mutation of a single weight in the given network */
void Simulation::mutate(double percent)
{
	int length = this->net->get_total_connections();
	int numMutations = percent*length;

	FANN::connection connections[length];
	this->net->get_connection_array(connections);

	/* mutate a percentage of random weight */
	for (int i = 0; i < numMutations; i++) {
		int index = rand() % length;
		int sign = rand() % 2 ? 1 : -1;
		fann_type current = connections[index].weight;
		double random = ((double)rand()/((double)RAND_MAX+1.0))/10.0;

		connections[index].weight += (fann_type) sign*random*current;
	}

	this->net->set_weight_array(connections, length);
}

int Simulation::amountReturned() {
	return this->world.currentAmount();
}

