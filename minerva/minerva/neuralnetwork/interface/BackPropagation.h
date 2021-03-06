/*! \file   BackPropagation.h
	\author Gregory Diamos
	\date   Sunday December 22, 2013
	\brief  The header file for the BackPropagation class.
*/

#pragma once

// Standard Library Includes
#include <vector>

// Forward Declarations
namespace minerva { namespace neuralnetwork { class NeuralNetwork; } }

namespace minerva { namespace matrix { class Matrix;                  } }
namespace minerva { namespace matrix { class BlockSparseMatrix;       } }
namespace minerva { namespace matrix { class BlockSparseMatrixVector; } }

namespace minerva { namespace optimizer { class SparseMatrixFormat; } }

namespace minerva
{
namespace neuralnetwork
{

class BackPropagation
{
public:
	typedef minerva::neuralnetwork::NeuralNetwork NeuralNetwork;
	typedef minerva::matrix::Matrix Matrix;
	typedef minerva::matrix::BlockSparseMatrix BlockSparseMatrix;
	typedef minerva::matrix::BlockSparseMatrixVector BlockSparseMatrixVector;
	typedef std::vector<optimizer::SparseMatrixFormat> DataStructureFormat;

public:
	BackPropagation(NeuralNetwork* ann = nullptr,
		BlockSparseMatrix* input = nullptr,
		BlockSparseMatrix* ref = nullptr);
	virtual ~BackPropagation();

public:
	BlockSparseMatrixVector computeCostDerivative() const;
	BlockSparseMatrix computeInputDerivative() const;
	float computeCost() const;
	float computeInputCost() const;
	float computeAccuracy() const;

public:
	NeuralNetwork*     getNeuralNetwork();
	BlockSparseMatrix* getInput();
	BlockSparseMatrix* getReferenceOutput();

public:
	const NeuralNetwork*     getNeuralNetwork() const;
	const BlockSparseMatrix* getInput() const;
	const BlockSparseMatrix* getReferenceOutput() const;

public:
	void setNeuralNetwork(NeuralNetwork* );
	void setInput(BlockSparseMatrix* );
	void setReferenceOutput(BlockSparseMatrix* );

public:
	Matrix getFlattenedWeights() const;
	Matrix getFlattenedCostDerivative() const;
	void   setFlattenedWeights(const Matrix& weights);

public:
	DataStructureFormat getWeightFormat() const;
	DataStructureFormat getInputFormat() const;

public:
	BlockSparseMatrixVector getWeights() const;
	void setWeights(const BlockSparseMatrixVector& weights);
	
	float computeCostForNewWeights(const BlockSparseMatrixVector& weights) const;
	float computeCostForNewInputs(const BlockSparseMatrixVector& inputs) const;
	float computeAccuracyForNewWeights(const BlockSparseMatrixVector& weights) const;
	BlockSparseMatrixVector computePartialDerivativesForNewWeights(
		const BlockSparseMatrixVector& weights) const;
	BlockSparseMatrixVector computePartialDerivativesForNewInputs(
		const BlockSparseMatrixVector& inputs) const;
	
public:
	float  computeCostForNewFlattenedWeights(const Matrix& weights) const;
	float  computeCostForNewFlattenedInputs(const Matrix& inputs) const;
	float  computeAccuracyForNewFlattenedWeights(const Matrix& weights) const;
	Matrix computePartialDerivativesForNewFlattenedWeights(const Matrix& weights) const;
	Matrix computePartialDerivativesForNewFlattenedInputs(const Matrix& inputs) const;	

public:
	static Matrix flatten(const BlockSparseMatrixVector& matrices);
	static Matrix flatten(const BlockSparseMatrix& blockedMatrix);

protected:
	virtual BlockSparseMatrixVector getCostDerivative(const NeuralNetwork&,
		const BlockSparseMatrix&, const BlockSparseMatrix& ) const = 0;
	virtual BlockSparseMatrix getInputDerivative(const NeuralNetwork&,
		const BlockSparseMatrix&, const BlockSparseMatrix&) const = 0;
	virtual float getCost(const NeuralNetwork&, const BlockSparseMatrix&,
		const BlockSparseMatrix&) const = 0;
	virtual float getInputCost(const NeuralNetwork&, const BlockSparseMatrix&,
		const BlockSparseMatrix&) const = 0;

protected:
	NeuralNetwork*     _neuralNetworkPtr;
	BlockSparseMatrix* _input;
	BlockSparseMatrix* _referenceOutput;

};

}

}

