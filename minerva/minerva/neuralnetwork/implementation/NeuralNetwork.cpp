/* Author: Sudnya Padalikar
 * Date  : 08/09/2013
 * The implementation of the Neural Network class 
 */

// Minerva Includes
#include <minerva/neuralnetwork/interface/NeuralNetwork.h>
#include <minerva/neuralnetwork/interface/NeuralNetworkSubgraphExtractor.h>
#include <minerva/neuralnetwork/interface/BackPropagation.h>
#include <minerva/neuralnetwork/interface/BackPropagationFactory.h>

#include <minerva/optimizer/interface/NeuralNetworkSolver.h>

#include <minerva/matrix/interface/Matrix.h>
#include <minerva/matrix/interface/BlockSparseMatrixVector.h>

#include <minerva/util/interface/Knobs.h>
#include <minerva/util/interface/debug.h>

// Standard Library Includes
#include <cassert>
#include <vector>
#include <ctime>

typedef minerva::optimizer::NeuralNetworkSolver NeuralNetworkSolver;

namespace minerva
{
namespace neuralnetwork
{

NeuralNetwork::NeuralNetwork()
: _useSparseCostFunction(false)
{

}

void NeuralNetwork::initializeRandomly(std::default_random_engine& engine, float epsilon)
{
	util::log("NeuralNetwork") << "Initializing neural network randomly.\n";

	for (auto i = _layers.begin(); i != _layers.end(); ++i)
	{
		(*i).initializeRandomly(engine, epsilon);
	}
}

void NeuralNetwork::initializeRandomly(float epsilon)
{
	std::default_random_engine engine(std::time(0));
	
	initializeRandomly(engine, epsilon);
}

void NeuralNetwork::train(const Matrix& inputMatrix, const Matrix& referenceOutput)
{
	auto inputData     = convertToBlockSparseForLayerInput(front(), inputMatrix);
	auto referenceData = convertToBlockSparseForLayerOutput(back(), referenceOutput);
	
	train(inputData, referenceData);
}

void NeuralNetwork::train(Matrix&& inputMatrix, Matrix&& referenceOutput)
{
	auto inputData     = convertToBlockSparseForLayerInput(front(), inputMatrix);
	auto referenceData = convertToBlockSparseForLayerOutput(back(), referenceOutput);
	
	inputMatrix.clear();
	referenceOutput.clear();
	
	train(inputData, referenceData);
}

void NeuralNetwork::train(BlockSparseMatrix& input, BlockSparseMatrix& reference)
{
	//create a backpropagate-data class
	//given the neural network, inputs & reference outputs 
	util::log("NeuralNetwork") << "Running back propagation on input matrix (" << input.rows() << ") rows, ("
	   << input.columns() << ") columns. Using reference output of (" << reference.rows() << ") rows, ("
	   << reference.columns() << ") columns. \n";

	auto backPropagation = createBackPropagation(); 

	backPropagation->setNeuralNetwork(this);
	backPropagation->setInput(&input);
	backPropagation->setReferenceOutput(&reference);

	auto solver = NeuralNetworkSolver::create(backPropagation);
	
	try
	{
		solver->solve();
	}
	catch(...)
	{
		delete solver;
		delete backPropagation;

		throw;
	}

	delete solver;
	delete backPropagation;
}

NeuralNetwork::Matrix NeuralNetwork::runInputs(const Matrix& m) const
{
	//util::log("NeuralNetwork") << "Running forward propagation on matrix (" << m.rows()
	//		<< " rows, " << m.columns() << " columns).\n";
	
	auto temp = convertToBlockSparseForLayerInput(front(), m);

	return runInputs(temp).toMatrix();
}

NeuralNetwork::BlockSparseMatrix NeuralNetwork::runInputs(const BlockSparseMatrix& m) const
{
	auto temp = m;

	for (auto i = _layers.begin(); i != _layers.end(); ++i)
	{
		util::log("NeuralNetwork") << " Running forward propagation through layer "
			<< std::distance(_layers.begin(), i) << "\n";
		//formatInputForLayer(*i, temp);
		temp = (*i).runInputs(temp);
	}

	return temp;
}

float NeuralNetwork::computeAccuracy(const Matrix& input, const Matrix& reference) const
{
	return computeAccuracy(convertToBlockSparseForLayerInput(front(), input),
		convertToBlockSparseForLayerOutput(back(), reference));
}

float NeuralNetwork::computeAccuracy(const BlockSparseMatrix& input,
	const BlockSparseMatrix& reference) const
{
	assert(input.rows() == reference.rows());
	assert(reference.columns() == getOutputCount());

	auto result = runInputs(input);

	float threshold = 0.5f;

	auto resultActivations	  = result.greaterThanOrEqual(threshold);
	auto referenceActivations = reference.greaterThanOrEqual(threshold);

	util::log("NeuralNetwork") << "Result activations " << resultActivations.toString();
	util::log("NeuralNetwork") << "Reference activations " << referenceActivations.toString();

	auto matchingActivations = resultActivations.equals(referenceActivations);

	float matches = matchingActivations.reduceSum();

	return matches / result.size();
}

std::string NeuralNetwork::getLabelForOutputNeuron(unsigned int i) const
{
	auto label = _labels.find(i);
	
	if(label == _labels.end())
	{
		std::stringstream stream;
		
		stream << "output-neuron-" << i;
		
		return stream.str();
	}
	
	util::log("NeuralNetwork") << "Getting output label for output neuron "
		<< i << ", " << label->second << "\n";
	
	return label->second;
}

void NeuralNetwork::setLabelForOutputNeuron(unsigned int idx, const std::string& label)
{
	assert(idx < getOutputCount());

	util::log("NeuralNetwork") << "Setting label for output neuron "
		<< idx << " to " << label << "\n";

	_labels[idx] = label;
}

void NeuralNetwork::setLabelsForOutputNeurons(const NeuralNetwork& network)
{
	_labels = network._labels;
}

void NeuralNetwork::addLayer(Layer&& L)
{
	_layers.push_back(std::move(L));
}

void NeuralNetwork::addLayer(const Layer& L)
{
	_layers.push_back(L);
}

unsigned NeuralNetwork::getTotalLayerSize() const
{
	return _layers.size();
}

const NeuralNetwork::LayerVector* NeuralNetwork::getLayers() const
{
	return &_layers;
}

NeuralNetwork::LayerVector* NeuralNetwork::getLayers()
{
	return &_layers;
}

void NeuralNetwork::resize(size_t layers)
{
	_layers.resize(layers);
}

void NeuralNetwork::clear()
{
	_layers.clear();
}

static size_t getGreatestCommonDivisor(size_t a, size_t b)
{
	// Euclid's method
	if(b == 0)
	{
		return a;
	}

	return getGreatestCommonDivisor(b, a % b);
}

void NeuralNetwork::mirror()
{
	mirror(front());
}

void NeuralNetwork::mirror(const Layer& layer)
{
	size_t blocks = getGreatestCommonDivisor(layer.blocks(),
		getGreatestCommonDivisor(getOutputCount(), layer.getInputCount()));

	assertM(getOutputCount() % blocks == 0, "Input count " << getOutputCount()
		<< " not divisible by " << blocks << ".");
	assertM(layer.getInputCount() % blocks == 0, "Output count " << layer.getInputCount()
		<< " not divisivle by " << blocks << ".");

	size_t newInputs  = getOutputCount() / blocks;
	size_t newOutputs = layer.getInputCount()  / blocks;
	
	assert(newInputs  > 0);
	assert(newOutputs > 0);
	
	util::log("NeuralNetwork") << "Mirroring neural network output layer ("
		<< back().blocks() << " blocks, " << back().getInputBlockingFactor()
		<< " inputs, " << back().getOutputBlockingFactor()
		<< " outputs) to (" << blocks << " blocks, " << newInputs
		<< " inputs, " << newOutputs << " outputs)\n";

	addLayer(Layer(blocks, newInputs, newOutputs));
	
	std::default_random_engine engine;

	// TODO: should wire this out of the neural network	
	bool shouldSeedWithTime = util::KnobDatabase::getKnobValue(
		"NeuralNetwork::SeedWithTime", false);
	
	if(shouldSeedWithTime)
	{
		engine.seed(std::time(0));
	}
	else
	{
		engine.seed(0);
	}

	// should be pseudo inverse
	back().initializeRandomly(engine);
}

void NeuralNetwork::cutOffSecondHalf()
{
	assert(size() > 1);

	resize(size() - 1);
}

size_t NeuralNetwork::getInputCount() const
{
	if(empty())
		return 0;

	return front().getInputCount();
}

size_t NeuralNetwork::getOutputCount() const
{
	return getOutputCountForInputCount(getInputCount());
}

size_t NeuralNetwork::getOutputNeurons() const
{
	if(empty())
		return 0;

	return back().getOutputCount();
}

size_t NeuralNetwork::getOutputCountForInputCount(
	size_t inputCount) const
{
	if(empty())
		return 0;
	
	// TODO: memoize
	size_t outputCount = inputCount;
	
	for(auto& layer : *this)
	{
		outputCount = layer.getOutputCountForInputCount(outputCount);
	}
	
	return outputCount;
}

size_t NeuralNetwork::getInputBlockingFactor() const
{
	if(empty())
		return 0;

	return front().getInputBlockingFactor();
}

size_t NeuralNetwork::getOutputBlockingFactor() const
{
	if(empty())
		return 0;

	return back().getOutputBlockingFactor();
}

size_t NeuralNetwork::totalConnections() const
{
	return totalWeights();
}

size_t NeuralNetwork::getFloatingPointOperationCount() const
{
	size_t flops = 0;

	for(auto& layer : *this)
	{
		flops += layer.getFloatingPointOperationCount();
	}
	
	return flops;
}

size_t NeuralNetwork::totalNeurons() const
{
	return totalActivations();
}

size_t NeuralNetwork::totalWeights() const
{
	size_t weights = 0;
	
	for(auto& layer : *this)
	{
		weights += layer.totalWeights();
	}
	
	return weights;
}

size_t NeuralNetwork::totalActivations() const
{
	size_t activations = 0;
	
	for(auto& layer : *this)
	{
		activations += layer.getOutputCount();
	}
	
	return activations;
}

NeuralNetwork::Matrix NeuralNetwork::getFlattenedWeights() const
{
	// TODO: avoid the copy
	Matrix weights(1, totalWeights());
	
	size_t position = 0;
	
	for(auto& layer : *this)
	{
		auto flattenedWeights = layer.getFlattenedWeights();

		std::memcpy(&weights.data()[position], &flattenedWeights.data()[0],
			flattenedWeights.size() * sizeof(float));

		position += flattenedWeights.size();	
	}
	
	return weights;
}

void NeuralNetwork::setFlattenedWeights(const Matrix& m)
{
	size_t weights = 0;

	for(auto& layer : *this)
	{
		auto sliced = m.slice(0, weights, 1, layer.totalWeights());
		
		layer.setFlattenedWeights(sliced);
		
		weights += layer.totalWeights();
	}
}

NeuralNetwork::BlockSparseMatrix NeuralNetwork::convertToBlockSparseForLayerInput(
	const Layer& layer, const Matrix& m) const
{
	assert(m.columns() % layer.getInputCount() == 0);
	
	BlockSparseMatrix result;

	size_t blockingFactor = layer.getInputBlockingFactor();

	for(size_t column = 0; column < m.columns(); column += blockingFactor)
	{
		size_t columns = std::min(m.columns() - column, blockingFactor);
		
		result.push_back(m.slice(0, column, m.rows(), columns));
	}
	
	result.setColumnSparse();

	return result;
}

void NeuralNetwork::formatInputForLayer(const Layer& layer, BlockSparseMatrix& m) const
{
	//assertM(layer.getInputCount() == m.columns(), "Layer input count "
	//	<< layer.getInputCount() << " does not match the input count "
	//	<< m.columns());
	assert(m.columns() % layer.getInputCount() == 0);
	
	if(layer.blocks() == m.blocks()) return;

	assert(m.isColumnSparse());

	// TODO: faster
	m = convertToBlockSparseForLayerInput(layer, m.toMatrix());
}

void NeuralNetwork::formatOutputForLayer(const Layer& layer, BlockSparseMatrix& m) const
{
	assert(m.columns() % layer.getOutputCount() == 0);
	
	if(layer.blocks() == m.blocks()) return;

	assert(m.isColumnSparse());

	// TODO: faster
	m = convertToBlockSparseForLayerOutput(layer, m.toMatrix());
}

NeuralNetwork::BlockSparseMatrix NeuralNetwork::convertToBlockSparseForLayerOutput(
	const Layer& layer, const Matrix& m) const
{
	assert(m.columns() % layer.getOutputCount() == 0);
	
	BlockSparseMatrix result;

	size_t blockingFactor = layer.getOutputBlockingFactor();

	for(size_t column = 0; column < m.columns(); column += blockingFactor)
	{
		size_t columns = std::min(m.columns() - column, blockingFactor);
		
		result.push_back(m.slice(0, column, m.rows(), columns));
	}
	
	result.setColumnSparse();

	return result;
}

NeuralNetwork::iterator NeuralNetwork::begin()
{
	return _layers.begin();
}

NeuralNetwork::const_iterator NeuralNetwork::begin() const
{
	return _layers.begin();
}

NeuralNetwork::iterator NeuralNetwork::end()
{
	return _layers.end();
}

NeuralNetwork::const_iterator NeuralNetwork::end() const
{
	return _layers.end();
}

NeuralNetwork::reverse_iterator NeuralNetwork::rbegin()
{
	return _layers.rbegin();
}

NeuralNetwork::const_reverse_iterator NeuralNetwork::rbegin() const
{
	return _layers.rbegin();
}

NeuralNetwork::reverse_iterator NeuralNetwork::rend()
{
	return _layers.rend();
}

NeuralNetwork::const_reverse_iterator NeuralNetwork::rend() const
{
	return _layers.rend();
}

NeuralNetwork::Layer& NeuralNetwork::operator[](size_t index)
{
	return _layers[index];
}

const NeuralNetwork::Layer& NeuralNetwork::operator[](size_t index) const
{
	return _layers[index];
}

Layer& NeuralNetwork::back()
{
	return _layers.back();
}

const Layer& NeuralNetwork::back() const
{
	return _layers.back();
}

Layer& NeuralNetwork::front()
{
	return _layers.front();
}

const Layer& NeuralNetwork::front() const
{
	return _layers.front();
}

size_t NeuralNetwork::size() const
{
	return _layers.size();
}

bool NeuralNetwork::empty() const
{
	return _layers.empty();
}

void NeuralNetwork::setParameters(const NeuralNetwork& network)
{
	setUseSparseCostFunction(network.isUsingSparseCostFunction());
	
	setLabelsForOutputNeurons(network);
}

void NeuralNetwork::setUseSparseCostFunction(bool shouldUse)
{
	_useSparseCostFunction = shouldUse;
}

bool NeuralNetwork::isUsingSparseCostFunction() const
{
	return _useSparseCostFunction;
}

BackPropagation* NeuralNetwork::createBackPropagation() const
{
	std::string backPropagationType;

	if(_useSparseCostFunction)
	{
		backPropagationType = "SparseBackPropagation";
	}
	else
	{
		backPropagationType = "DenseBackPropagation";
	}
	
	backPropagationType = util::KnobDatabase::getKnobValue(
		"BackPropagation::ForceType", backPropagationType);
	
	auto backPropagation = BackPropagationFactory::create(backPropagationType);
	
	if(backPropagation == nullptr)
	{
		throw std::runtime_error("Failed to create back propagation "
			"structure with type: " + backPropagationType);
	}
	
	backPropagation->setNeuralNetwork(const_cast<NeuralNetwork*>(this));
	
	return backPropagation;
}

float NeuralNetwork::getCostAndGradient(BlockSparseMatrixVector& gradient, BlockSparseMatrix& input, BlockSparseMatrix& reference) const
{
	//create a backpropagate-data class
	//given the neural network, inputs & reference outputs 
	std::unique_ptr<BackPropagation> backPropagation(createBackPropagation()); 

	backPropagation->setNeuralNetwork(const_cast<NeuralNetwork*>(this));
	backPropagation->setInput(&input);
	backPropagation->setReferenceOutput(&reference);
	
	// TODO: merge these
	gradient = backPropagation->computeCostDerivative();
	
	return backPropagation->computeCost();
}

float NeuralNetwork::getCost(BlockSparseMatrix& input, BlockSparseMatrix& reference) const
{
	//create a backpropagate-data class
	//given the neural network, inputs & reference outputs 
	std::unique_ptr<BackPropagation> backPropagation(createBackPropagation()); 

	backPropagation->setNeuralNetwork(const_cast<NeuralNetwork*>(this));
	backPropagation->setInput(&input);
	backPropagation->setReferenceOutput(&reference);
	
	return backPropagation->computeCost();
}
		
float NeuralNetwork::getCostAndGradient(BlockSparseMatrixVector& gradient, const Matrix& input, const Matrix& reference) const
{
	auto sparseInput     = convertToBlockSparseForLayerInput(front(), input);
	auto sparseReference = convertToBlockSparseForLayerOutput(back(), reference);
	
	return getCostAndGradient(gradient, sparseInput, sparseReference);
}

float NeuralNetwork::getCost(const Matrix& input, const Matrix& reference) const
{
	auto sparseInput     = convertToBlockSparseForLayerInput(front(), input);
	auto sparseReference = convertToBlockSparseForLayerOutput(back(), reference);
	
	return getCost(sparseInput, sparseReference);
}

bool NeuralNetwork::areConnectionsValid() const
{
	// TODO
	
	util::log("NeuralNetwork") << "Verified network with " << getInputCount()
		<< " inputs and " << getOutputCount() << " outputs\n";
	
	return true;
}

NeuralNetwork::NeuronSet NeuralNetwork::getInputNeuronsConnectedToThisOutput(
	unsigned neuron) const
{
	NeuronSet inputs;
	
	NeuronSet outputs;

	inputs.insert(neuron);
	
	for(auto layer = rbegin(); layer != rend(); ++layer)
	{
		std::swap(inputs, outputs);
		
		inputs = layer->getInputNeuronsConnectedToTheseOutputs(outputs);
	}
	
	return inputs;
}

NeuralNetwork NeuralNetwork::getSubgraphConnectedToThisOutput(
	unsigned neuron) const
{
	NeuralNetworkSubgraphExtractor extractor(const_cast<NeuralNetwork*>(this));
	
	return extractor.copySubgraphConnectedToThisOutput(neuron);
}

std::string NeuralNetwork::shapeString() const
{
	std::stringstream stream;
	
	stream << "Neural Network [" << size() << " layers, " << getInputCount()
		<< " inputs (" << getInputBlockingFactor() << " way blocked), "
		<< getOutputNeurons() << " outputs (" << getOutputBlockingFactor()
		<< " way blocked)]\n";

	for(auto& layer : *this)
	{
		size_t index = &layer - &*begin();
		
		stream << " Layer " << index << ": [" << layer.blocks() << " blocks, "
			<< layer.getInputCount() << " inputs ("
			<< layer.getInputBlockingFactor() << " way blocked), "
			<< layer.getOutputCount() << " outputs (" << layer.getOutputBlockingFactor()
			<< " way blocked), " << layer.blockStep() << " block step]\n";
	}
	
	return stream.str();
}

}

}

