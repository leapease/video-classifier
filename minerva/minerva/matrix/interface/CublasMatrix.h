/*	\file   CublasMatrix.h
	\date   Monday September 2, 2013
	\author Gregory Diamos <solusstultus@gmail.com>
	\brief  The header file for the CublasMatrix class.
*/

#pragma once

// Minerva Includes
#include <minerva/matrix/interface/MatrixImplementation.h>

namespace minerva
{

namespace matrix
{

class CublasMatrix : public MatrixImplementation
{
public:
	CublasMatrix(size_t rows, size_t columns,
		const FloatVector& data = FloatVector());
	
public:
	virtual void resize(size_t rows, size_t columns);

public:
	virtual Value* appendColumns(const Value* m) const;
	virtual Value* appendRows(const Value* m) const;
	virtual Value* transpose() const;
 
public: 
	virtual Value* multiply(const Value* m) const;
	virtual Value* multiply(float f) const;
	virtual Value* elementMultiply(const Value* m) const;

	virtual Value* add(const Value* m) const;
	virtual Value* addBroadcastRow(const Value* m) const;
	virtual Value* add(float f) const;

	virtual Value* subtract(const Value* m) const;
	virtual Value* subtract(float f) const;

	virtual Value* log() const;
	virtual Value* abs() const;
	virtual Value* negate() const;
	virtual Value* sigmoid() const;
	virtual Value* sigmoidDerivative() const;
	virtual Value* klDivergence(float sparsity) const;
	virtual Value* klDivergenceDerivative(float sparsity) const;

public:
	virtual Value* slice(size_t startRow, size_t startColumn,
		size_t rows, size_t columns) const;

public:
	virtual void negateSelf();
	virtual void logSelf();
	virtual void absSelf();
    virtual void sigmoidSelf();
    virtual void sigmoidDerivativeSelf();
    virtual void klDivergenceSelf(float sparsity);
    virtual void klDivergenceDerivativeSelf(float sparsity);
    virtual void minSelf(float f);
    virtual void maxSelf(float f);
    virtual void assignSelf(float f);

	virtual void assignUniformRandomValues(
		std::default_random_engine& engine, float min, float max);
	virtual void transposeSelf();

public:
	virtual Value* greaterThanOrEqual(float f) const;
	virtual Value* equals(const Value* m) const;
	virtual Value* lessThanOrEqual(float f) const;

public:
    virtual float reduceSum() const;
	virtual Value* reduceSumAlongColumns() const;

public:
	virtual FloatVector& data();
	virtual const FloatVector& data() const;

public:
	virtual Value* clone() const;

public:
	static bool isSupported();

};

}

}




