/*	\file   SimulatedAnnealingSolver.cpp
	\date   Sunday August 11, 2013
	\author Gregory Diamos <solusstultus@gmail.com>
	\brief  The source file for the SimulatedAnnealingSolver class.
*/

// Minvera Includes
#include <minerva/optimizer/interface/SimulatedAnnealingSolver.h>
#include <minerva/optimizer/interface/CostFunction.h>

#include <minerva/matrix/interface/Matrix.h>

#include <minerva/util/interface/debug.h>
#include <minerva/util/interface/Knobs.h>

// Standard Library Includes
#include <cmath>
#include <random>

namespace minerva
{

namespace optimizer
{

typedef matrix::Matrix Matrix;

SimulatedAnnealingSolver::~SimulatedAnnealingSolver()
{

}

static float simulatedAnnealing(Matrix& inputs, const CostFunction& callback);

float SimulatedAnnealingSolver::solve(Matrix& inputs, const CostFunction& callBack)
{
	return simulatedAnnealing(inputs, callBack);
}

static float computeTempurature(float temperature)
{
	return temperature * 0.9f;
}

static float annealingProbability(float currentCost, float newCost,
	float tempurature)
{
	if(newCost < currentCost)
		return 1.0f;

	return std::exp(-(newCost - currentCost) / tempurature);
}

static Matrix pickNeighbouringState(const Matrix& inputs,
	std::default_random_engine& generator, unsigned int iterations)
{
	typedef std::uniform_int_distribution<unsigned int> IntDistribution;
	typedef std::uniform_real_distribution<float>       FloatDistribution;

	Matrix newInputs = inputs;

	IntDistribution   intDistribution(0, inputs.size() - 1);
	FloatDistribution floatDistribution(-2.0f / inputs.size(), 2.0f / inputs.size());

	unsigned changes = std::max(10UL, 2 * inputs.size() / iterations);

	for(unsigned change = 0; change != changes; ++change)
	{
		unsigned position = intDistribution(generator);
		
		newInputs(0, position) += floatDistribution(generator);
	}

	return newInputs;
}

static float simulatedAnnealing(Matrix& inputs, const CostFunction& callback)
{
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	std::default_random_engine generator(std::time(0));

	unsigned iterations = util::KnobDatabase::getKnobValue(
		"SimulatedAnnealing::Iterations", 50000);
	
	auto  currentInputs = inputs;
	float currentCost	= callback.computeCost(currentInputs);
		
	float bestCostSoFar = currentCost;
	auto  bestInputs    = currentInputs;

	util::log("SimulatedAnnealingSolver")
		<< "   Running simmulated annealing for "
		<< iterations << " iterations\n";

	float tempurature = util::KnobDatabase::getKnobValue(
		"SimulatedAnnealing::Tempurature", 50.0f);
	
	for(unsigned i = 0; i < iterations; ++i)
	{
		tempurature = computeTempurature(tempurature);
		
		auto newInputs = pickNeighbouringState(currentInputs, generator,
			iterations);
		
		float newCost = callback.computeCost(newInputs);
		
		if(annealingProbability(currentCost, newCost, tempurature) >
			distribution(generator))
		{
			currentInputs = newInputs;
			currentCost	  = newCost;
			
			util::log("SimulatedAnnealingSolver") << "	accepted new inputs, "
				"cost is now: " << currentCost << " (iteration " << i << ")\n";
		}

		if (newCost < bestCostSoFar)
		{
			bestInputs    = newInputs;
			bestCostSoFar = newCost;

			util::log("SimulatedAnnealingSolver") << "	 best cost is now: "
				<< bestCostSoFar << " (iteration " << i << ")\n";
		}
	}
	
	return bestCostSoFar;
}

}

}

