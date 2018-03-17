#include "Matrix.h"
#include "MatrixUtils.h"

CMatrixRow::CMatrixRow(matrix_size column_count) : values(new matrix_member[column_count]), column_count(column_count)
{
#ifdef MATRIX_INITIALIZATION_VALUE
	for (matrix_size i = 0; i < column_count; i++)
		values[i] = MATRIX_INITIALIZATION_VALUE;
#endif
}

CMatrixRow::CMatrixRow(const CMatrixRow& original) : values(new matrix_member[original.get_column_count()]), column_count(original.get_column_count())
{
	for (matrix_size i = 0; i < original.get_column_count(); i++)
		values[i] = original.get_column(i);
}

CMatrixRow::~CMatrixRow()
{
	delete[] values;
}

CMatrixRow& CMatrixRow::operator+=(const CMatrixRow& other)
{
	if (other.get_column_count() != this->get_column_count())
		throw matrix_exception("adding rows of different sizes");

	for (matrix_size i = 0; i < this->get_column_count(); i++)
		this->values[i] += other.get_column(i);

	return (*this);
}

CMatrixRow& CMatrixRow::operator-=(const CMatrixRow& other)
{
	if (other.get_column_count() != this->get_column_count())
		throw matrix_exception("subtracting rows of different sizes");

	for (matrix_size i = 0; i < this->get_column_count(); i++)
		this->values[i] -= other.get_column(i);

	return (*this);
}

CMatrixRow& CMatrixRow::operator*=(const matrix_member& val)
{
	for (matrix_size i = 0; i < this->get_column_count(); i++)
		this->values[i] *= val;

	return (*this);
}

CMatrixRow& CMatrixRow::operator/=(const matrix_member& val)
{
	if (val == 0)
		throw std::runtime_error("dividing matrix row by zero");

	for (matrix_size i = 0; i < this->get_column_count(); i++)
		this->values[i] /= val;

	return (*this);
}

const matrix_member CMatrixRow::get_column(matrix_size column) const
{
	if (column >= column_count)
		throw std::out_of_range("column index out of range");

	return values[column];
}

const matrix_size CMatrixRow::get_column_count() const
{
	return column_count;
}

void CMatrixRow::set_value(matrix_size column, matrix_member value)
{
	if (column >= column_count)
		throw std::out_of_range("column index out of range");

	values[column] = value;
}

CMatrix::CMatrix(matrix_size rows, matrix_size columns) : row_count(rows), swap_coefficient(1), rows(new CMatrixRow*[rows])
{
	for (matrix_size i = 0; i < rows; i++)
		this->rows[i] = new CMatrixRow(columns);
}

CMatrix::CMatrix(const CMatrix& original) : row_count(original.get_row_count()), swap_coefficient(original.swap_coefficient), rows(new CMatrixRow*[row_count])
{
	for (matrix_size i = 0; i < row_count; i++)
		this->rows[i] = new CMatrixRow(*original.get_row(i));
}

CMatrix::CMatrix(CMatrix&& matrix) : row_count(std::move(matrix.row_count)), swap_coefficient(std::move(matrix.swap_coefficient)), rows(matrix.rows)
{
	matrix.rows = nullptr;
}

CMatrix::~CMatrix()
{
	if (rows)
	{
		for (matrix_size i = 0; i < row_count; i++)
			if (rows[i])
				delete(rows[i]);

		delete[](rows);
	}
}

void CMatrix::swap_rows(matrix_size row_index_1, matrix_size row_index_2)
{
	if (row_index_1 >= row_count || row_index_2 >= row_count)
		throw std::out_of_range("row index out of range");
	
	swap_coefficient *= -1;

	std::swap(rows[row_index_1], rows[row_index_2]);
}

int CMatrix::get_swap_coefficient() const
{
	return swap_coefficient;
}

CMatrixRow* CMatrix::get_row(matrix_size row) const
{
	if (row >= row_count)
		throw std::out_of_range("row index out of range");

	return rows[row];
}


const matrix_member CMatrix::get_value(matrix_size row, matrix_size column) const
{
	return get_row(row)->get_column(column);
}

const matrix_size CMatrix::get_row_count() const
{
	return row_count;
}

const matrix_size CMatrix::get_column_count() const
{
	if (this->get_row_count() == 0)
		return 0;

	return rows[0]->get_column_count();
}

void CMatrix::set_value(matrix_size row, matrix_size column, matrix_member value)
{
	this->get_row(row)->set_value(column, value);
}

void CMatrix::set_row(matrix_size row_index, CMatrixRow* row)
{
	if (row_index >= row_count)
		throw std::out_of_range("row index out of range");

	if (row != rows[row_index])
	{
		delete(rows[row_index]);
		rows[row_index] = row;
	}
}

CMatrixRow operator*(const CMatrixRow& matrix_row, const matrix_member& val)
{
	CMatrixRow result(matrix_row);
	result *= val;
	return result;
}

CMatrixRow operator+(const CMatrixRow& matrix_row, const CMatrixRow& other)
{
	CMatrixRow result(matrix_row);
	result += other;
	return result;
}

CMatrixRow operator-(const CMatrixRow& matrix_row, const CMatrixRow& other)
{
	CMatrixRow result(matrix_row);
	result -= other;
	return other;
}

CMatrixRow operator/(const CMatrixRow& matrix_row, const matrix_member& val)
{
	CMatrixRow result(matrix_row);
	result /= val;
	return result;
}