/*	\file   BlockSparseMatrix.cpp
	\date   Sunday August 11, 2013
	\author Gregory Diamos <solusstultus@gmail.com>
	\brief  The source file for the BlockSparseMatrix class.
*/

// Minerva Includes
#include <minerva/matrix/interface/BlockSparseMatrix.h>
#include <minerva/matrix/interface/Matrix.h>

#include <minerva/util/interface/debug.h>

// Standard Library Includes
#include <cassert>

namespace minerva
{

namespace matrix
{

BlockSparseMatrix::BlockSparseMatrix(size_t blocks, size_t rows,
	size_t columns, bool isRowSparse)
: _matrices(blocks), _isRowSparse(isRowSparse)
{
	for(auto& matrix : *this)
	{
		matrix.resize(rows, columns);
	}
}

BlockSparseMatrix::BlockSparseMatrix(bool isRowSparse)
: _isRowSparse(isRowSparse)
{

}

BlockSparseMatrix::iterator BlockSparseMatrix::begin()
{
	return _matrices.begin();
}

BlockSparseMatrix::const_iterator BlockSparseMatrix::begin() const
{
	return _matrices.begin();
}

BlockSparseMatrix::iterator BlockSparseMatrix::end()
{
	return _matrices.end();
}

BlockSparseMatrix::const_iterator BlockSparseMatrix::end() const
{
	return _matrices.end();
}

Matrix& BlockSparseMatrix::front()
{
	return _matrices.front();
}

const Matrix& BlockSparseMatrix::front() const
{
	return _matrices.front();
}

Matrix& BlockSparseMatrix::back()
{
	return _matrices.back();
}

const Matrix& BlockSparseMatrix::back() const
{
	return _matrices.back();
}

const Matrix& BlockSparseMatrix::operator[](size_t position) const
{
	return _matrices[position];
}

Matrix& BlockSparseMatrix::operator[](size_t position)
{
	return _matrices[position];
}

void BlockSparseMatrix::pop_back()
{
	return _matrices.pop_back();
}

void BlockSparseMatrix::push_back(const Matrix& m)
{
	return _matrices.push_back(m);
}

size_t BlockSparseMatrix::size() const
{
	size_t s = 0;

	for(auto& m : *this)
	{
		s += m.size();
	}

	return s;
}

size_t BlockSparseMatrix::blocks() const
{
	return _matrices.size();
}

bool BlockSparseMatrix::empty() const
{
	return _matrices.empty();
}

size_t BlockSparseMatrix::columns() const
{
	if(isColumnSparse())
	{
		size_t c = 0;

		for(auto& matrix : *this)
		{
			c += matrix.columns();
		}

		return c;
	}
	
	return front().columns();
}

size_t BlockSparseMatrix::rows() const
{
	if(isRowSparse())
	{
		size_t r = 0;

		for(auto& matrix : *this)
		{
			r += matrix.rows();
		}

		return r;
	}


	return front().rows();
}

bool BlockSparseMatrix::isRowSparse() const
{
	return _isRowSparse;
}

bool BlockSparseMatrix::isColumnSparse() const
{
	return not isRowSparse();
}

void BlockSparseMatrix::resize(size_t blocks, size_t rowsPerBlock,
	size_t columnsPerBlock)
{
	_matrices.resize(blocks);

	for(auto& matrix : *this)
	{
		matrix.resize(rowsPerBlock, columnsPerBlock);
	}
}

void BlockSparseMatrix::setColumnSparse()
{
	_isRowSparse = false;
}

void BlockSparseMatrix::setRowSparse()
{
	_isRowSparse = true;
}

BlockSparseMatrix BlockSparseMatrix::multiply(
	const BlockSparseMatrix& m) const
{
	// TODO: in parallel
	BlockSparseMatrix result(isRowSparse());

	assert(m.blocks() == blocks());
	assertM(columns() == m.rows(), "Left columns " << columns() << " does not match right rows " << m.rows());

	for(auto left = begin(), right = m.begin(); left != end(); ++left, ++right)
	{
		result.push_back(left->multiply(*right));
	}

	return result;
}

BlockSparseMatrix BlockSparseMatrix::multiply(float f) const
{
	BlockSparseMatrix result(isRowSparse());

	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.multiply(f));
	}
	
	return result;
}

BlockSparseMatrix BlockSparseMatrix::elementMultiply(const BlockSparseMatrix& m) const
{
	// TODO: in parallel
	BlockSparseMatrix result(isRowSparse());

	assert(m.blocks() == blocks());

	for(auto left = begin(), right = m.begin(); left != end(); ++left, ++right)
	{
		result.push_back(left->elementMultiply(*right));
	}

	return result;
}

BlockSparseMatrix BlockSparseMatrix::add(const BlockSparseMatrix& m) const
{
	// TODO: in parallel
	BlockSparseMatrix result(isRowSparse());

	assert(m.size() == size());

	for(auto left = begin(), right = m.begin(); left != end(); ++left, ++right)
	{
		result.push_back(left->add(*right));
	}

	return result;

}

BlockSparseMatrix BlockSparseMatrix::addBroadcastRow(const BlockSparseMatrix& m) const
{
	// TODO: in parallel
	BlockSparseMatrix result(isRowSparse());

	assert(m.columns() == columns());

	for(auto left = begin(), right = m.begin(); left != end(); ++left, ++right)
	{
		result.push_back(left->addBroadcastRow(*right));
	}

	return result;

}

BlockSparseMatrix BlockSparseMatrix::add(float f) const
{
	BlockSparseMatrix result(isRowSparse());

	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.add(f));
	}
	
	return result;
}

BlockSparseMatrix BlockSparseMatrix::subtract(const BlockSparseMatrix& m) const
{
	// TODO: in parallel
	BlockSparseMatrix result(isRowSparse());

	assert(m.size() == size());

	for(auto left = begin(), right = m.begin(); left != end(); ++left, ++right)
	{
		result.push_back(left->subtract(*right));
	}

	return result;
}

BlockSparseMatrix BlockSparseMatrix::subtract(float f) const
{
	BlockSparseMatrix result(isRowSparse());

	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.subtract(f));
	}
	
	return result;
}

BlockSparseMatrix BlockSparseMatrix::log() const
{
	BlockSparseMatrix result(isRowSparse());

	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.log());
	}
	
	return result;
}

BlockSparseMatrix BlockSparseMatrix::negate() const
{
	BlockSparseMatrix result(isRowSparse());

	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.negate());
	}
	
	return result;

}

BlockSparseMatrix BlockSparseMatrix::sigmoidDerivative() const
{
	BlockSparseMatrix result(isRowSparse());
	
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.sigmoidDerivative());
	}
	
	return result;
}

BlockSparseMatrix BlockSparseMatrix::sigmoid() const
{
	BlockSparseMatrix result(isRowSparse());

	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.sigmoid());
	}
	
	return result;
}

BlockSparseMatrix BlockSparseMatrix::transpose() const
{
	BlockSparseMatrix result(isRowSparse());
	
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		result.push_back(matrix.transpose());
	}
	
	return result;
}

void BlockSparseMatrix::negateSelf()
{
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		matrix.negateSelf();
	}
}

void BlockSparseMatrix::logSelf()
{
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		matrix.logSelf();
	}
}

void BlockSparseMatrix::sigmoidSelf()
{
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		matrix.sigmoidSelf();
	}
}

void BlockSparseMatrix::sigmoidDerivativeSelf()
{
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		matrix.sigmoidDerivativeSelf();
	}
}

void BlockSparseMatrix::transposeSelf()
{
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		matrix.transposeSelf();
	}
}

void BlockSparseMatrix::assignUniformRandomValues(
	std::default_random_engine& engine, float min, float max)
{
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		matrix.assignUniformRandomValues(engine, min, max);
	}
}

BlockSparseMatrix BlockSparseMatrix::greaterThanOrEqual(float f) const
{
	BlockSparseMatrix result(isRowSparse());
	
	for(auto& matrix : *this)
	{
		result.push_back(matrix.greaterThanOrEqual(f));
	}
	
	return result;
}

BlockSparseMatrix BlockSparseMatrix::equals(const BlockSparseMatrix& m) const
{
	BlockSparseMatrix result(isRowSparse());
	
	for(auto matrix = begin(), block = m.begin(); matrix != end(); ++matrix, ++block)
	{
		result.push_back(matrix->equals(*block));
	}

	return result;
}

Matrix BlockSparseMatrix::toMatrix() const
{
	Matrix result;

	// TODO: faster	
	if(isColumnSparse())
	{
		for(auto& matrix : *this)
		{
			result = result.appendColumns(matrix);
		}
	}
	else
	{
		for(auto& matrix : *this)
		{
			result = result.appendRows(matrix);
		}
	}
	
	return result;
}

float BlockSparseMatrix::reduceSum() const
{
	float sum = 0.0f;
	
	// TODO: in parallel
	for(auto& matrix : *this)
	{
		sum += matrix.reduceSum();
	}
	
	return sum;
}

std::string BlockSparseMatrix::toString() const
{
	if(empty()) return "(0 rows, 0 columns) []";
	
	return front().toString();
}

}

}

