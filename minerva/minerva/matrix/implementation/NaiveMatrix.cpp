/*	\file   NaiveMatrix.cpp
	\date   Sunday August 11, 2013
	\author Gregory Diamos <solusstultus@gmail.com>
	\brief  The source file for the NaiveMatrix class.
*/

// Minerva Includes
#include <minerva/matrix/interface/NaiveMatrix.h>

#include <minerva/util/interface/debug.h>

// Standard Library Includes
#include <cmath>
#include <random>

namespace minerva
{

namespace matrix
{

typedef NaiveMatrix::Value Value;

NaiveMatrix::NaiveMatrix(size_t r, size_t c, const FloatVector& d)
: MatrixImplementation(r, c, d)
{
    resize(rows(), columns());
}

void NaiveMatrix::resize(size_t rows, size_t columns)
{
	_data.resize(rows * columns);
	
	_rows	 = rows;
	_columns = columns;
}

Value* NaiveMatrix::appendColumns(const Value* matrix) const
{
	auto m = dynamic_cast<const NaiveMatrix*>(matrix);	
	assert(m != nullptr);

	assert(empty() || (rows() == m->rows()));

    size_t resultRows = rows();

    if(empty())
    {
        resultRows = m->rows();
    }

	auto result = new NaiveMatrix(resultRows, columns() + m->columns());
	
	// Copy rows from the original and appended matrices
	for(size_t row = 0; row != resultRows; ++row)
	{
		size_t originalPosition = getPosition(row, 0);
		size_t newPosition = result->getPosition(row, 0);
	
		std::memcpy(&result->_data[newPosition], &_data[originalPosition],
			columns() * sizeof(float));

		size_t appendedPosition = m->getPosition(row, 0);
		newPosition += columns();
		
		std::memcpy(&(*result)._data[newPosition], &m->_data[appendedPosition],
			m->columns() * sizeof(float));
	}
	
	return result;
}

Value* NaiveMatrix::appendRows(const Value* matrix) const
{
	auto m = dynamic_cast<const NaiveMatrix*>(matrix);	
	assert(m != nullptr);

	assert(empty() || (columns() == m->columns()));

    size_t resultColumns = columns();

    if(empty())
    {
        resultColumns = m->columns();
    }

	NaiveMatrix* result = new NaiveMatrix(rows() + m->rows(), resultColumns);
	
	// Copy rows from the original and appended matrices
	std::memcpy(&result->_data[0], &_data[0], size() * sizeof(float));
	std::memcpy(&result->_data[size()], &m->_data[0],
		m->size() * sizeof(float));
	
	return result;
}

Value* NaiveMatrix::transpose() const
{
	NaiveMatrix* result = new NaiveMatrix(columns(), rows());
	
	// TODO: faster
	for(size_t row = 0; row != rows(); ++row)
	{
		for(size_t column = 0; column != columns(); ++column)
		{
			result->data()[result->getPosition(column, row)] =
				data()[getPosition(row, column)];
		}
	}
	
	return result;
}

Value* NaiveMatrix::multiply(const Value* matrix) const
{
	auto m = dynamic_cast<const NaiveMatrix*>(matrix);	
	assert(m != nullptr);

	assert(columns() == m->rows());

	NaiveMatrix* result = new NaiveMatrix(rows(), m->columns());
	
	// TODO: much faster
	for(size_t row = 0; row != result->rows(); ++row)
	{
		for(size_t column = 0; column != result->columns(); ++column)
		{
			float value = 0.0f;
			
			for(size_t inner = 0; inner != columns(); ++inner)
			{
				value += data()[getPosition(row, inner)] *
					m->data()[m->getPosition(inner, column)];
			}

            result->data()[result->getPosition(row, column)] = value;
		}
	}
	
	return result;
}

Value* NaiveMatrix::multiply(float f) const
{
	NaiveMatrix* result = new NaiveMatrix(*this);
	
	// TODO: faster
	for(auto& value : result->_data)
	{
		value *= f;
	}
	
	return result;
}

Value* NaiveMatrix::elementMultiply(const Value* matrix) const
{
	auto m = dynamic_cast<const NaiveMatrix*>(matrix);	
	assert(m != nullptr);

	assert(m->rows()    == rows()   );
	assert(m->columns() == columns());

    NaiveMatrix* result = new NaiveMatrix(*this);

	// TODO: faster
	auto rValue = result->_data.begin();
	for(auto value = m->_data.begin(); value != m->_data.end();
		++value, ++rValue)
	{
		*rValue *= *value;
	}

    return result;
}

Value* NaiveMatrix::add(float f) const
{
	NaiveMatrix* result = new NaiveMatrix(*this);
	
	// TODO: faster
	for(auto& value : result->_data)
	{
		value += f;
	}
	
	return result;
}

Value* NaiveMatrix::add(const Value* matrix) const
{
	auto m = dynamic_cast<const NaiveMatrix*>(matrix);	
	assert(m != nullptr);
	
	assert(m->rows()    == rows());
	assert(m->columns() == columns());

	NaiveMatrix* result = new NaiveMatrix(*this);
	
	// TODO: faster
	auto rValue = result->_data.begin();
	for(auto value = m->_data.begin(); value != m->_data.end();
		++value, ++rValue)
	{
		*rValue += *value;
	}
	
	return result;
}

Value* NaiveMatrix::subtract(const Value* matrix) const
{
	auto m = dynamic_cast<const NaiveMatrix*>(matrix);	
	assert(m != nullptr);
	
	assert(m->rows()    == rows());
	assert(m->columns() == columns());

	NaiveMatrix* result = new NaiveMatrix(*this);
	
	// TODO: faster
	auto rValue = result->_data.begin();
	for(auto value = m->_data.begin(); value != m->_data.end();
		++value, ++rValue)
	{
		*rValue -= *value;
	}
	
	return result;
}

Value* NaiveMatrix::subtract(float f) const
{
	NaiveMatrix* result = new NaiveMatrix(*this);
	
	// TODO: faster
	for(auto& value : result->_data)
	{
		value -= f;
	}
	
	return result;
}

Value* NaiveMatrix::slice(size_t startRow, size_t startColumn,
	size_t rows, size_t columns) const
{
	NaiveMatrix* result = new NaiveMatrix(rows, columns);
	
	assert(startRow    + rows    <= this->rows()   );
	assert(startColumn + columns <= this->columns());
	
	for(size_t row = 0; row != rows; ++row)
	{
		std::memcpy(&result->data()[result->getPosition(row, 0)],
			&data()[getPosition(row + startRow, startColumn)],
			columns * sizeof(float));
	}
	
	return result;
}

Value* NaiveMatrix::log() const
{
    NaiveMatrix* result = new NaiveMatrix(*this);
	
	result->logSelf();

    return result;
}

Value* NaiveMatrix::abs() const
{
    NaiveMatrix* result = new NaiveMatrix(*this);
	
	result->absSelf();

    return result;
}

Value* NaiveMatrix::negate() const
{
    NaiveMatrix* result = new NaiveMatrix(*this);
	
	result->negateSelf();

    return result;
}

Value* NaiveMatrix::sigmoid() const
{
    NaiveMatrix* result = new NaiveMatrix(*this);
	
	result->sigmoidSelf();

    return result;
}

Value* NaiveMatrix::sigmoidDerivative() const
{
    NaiveMatrix* result = new NaiveMatrix(*this);
	
	result->sigmoidDerivativeSelf();

    return result;
}

void NaiveMatrix::negateSelf()
{
	for(auto& f : _data)
	{
		f = -f;
	}
}

void NaiveMatrix::logSelf()
{
	for(auto& f : _data)
	{
		f = std::log(f);
	}
}

void NaiveMatrix::absSelf()
{
	for(auto& f : _data)
	{
		f = std::abs(f);
	}
}

static float sigmoid(float v)
{
    if(v < -50.0f) return 0.0f;
    if(v >  50.0f) return 1.0f;
    
    return 1.0f / (1.0f + std::exp(-v)); 
}

static float sigmoidDerivative(float v)
{
    // f(x) = 1/(1+e^-x)
    // dy/dx = f(x)' = f(x) * (1 - f(x))
	float element = sigmoid(v) * (1.0f - sigmoid(v));
	
	element = element * (1.0f - element);
	
	return element;
}

void NaiveMatrix::sigmoidSelf()
{
	for(auto& f : _data)
	{
		f = matrix::sigmoid(f);
	}
}

void NaiveMatrix::sigmoidDerivativeSelf()
{
	for(auto& f : _data)
	{
		f = matrix::sigmoidDerivative(f);
	}
}

void NaiveMatrix::assignUniformRandomValues(float min, float max)
{
	std::default_random_engine generator(std::time(0));
	std::uniform_real_distribution<float> distribution(min, max);

	for(auto& f : _data)
	{
		f = distribution(generator);
	}
}

void NaiveMatrix::transposeSelf()
{
    // TODO: in place
	auto matrix = transpose();
	
	auto naiveMatrix = dynamic_cast<NaiveMatrix*>(matrix);
	assert(naiveMatrix != nullptr);
	
	*this = *naiveMatrix;
	
	delete naiveMatrix;
}

Value* NaiveMatrix::greaterThanOrEqual(float f) const
{
	NaiveMatrix* result = new NaiveMatrix(*this);
	
	// TODO: faster
	for(auto& value : result->_data)
	{
		value = (value >= f) ? 1.0f : 0.0f;
	}
	
	return result;
}

Value* NaiveMatrix::equals(const Value* m) const
{
	assert(m->size() == size());

	NaiveMatrix* result = new NaiveMatrix(*this);
	
	// TODO: faster
	auto value = m->data().begin();
	for(auto resultValue = result->data().begin(); resultValue != result->data().end();
		++resultValue, ++value)
	{
		*resultValue = (*resultValue == *value) ? 1.0f : 0.0f;
	}
	
	return result;
}

float NaiveMatrix::reduceSum() const
{
    float sum = 0.0f;

    for(auto& f : _data)
    {
        sum += f;
    }

    return sum;
}

const NaiveMatrix::FloatVector& NaiveMatrix::data() const
{
	return _data;
}

NaiveMatrix::FloatVector& NaiveMatrix::data()
{
	return _data;
}

Value* NaiveMatrix::clone() const
{
	return new NaiveMatrix(*this);
}

}

}


