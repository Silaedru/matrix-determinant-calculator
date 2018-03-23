#ifndef _MATRIX_H_
#define _MATRIX_H_

#define MATRIX_INITIALIZATION_VALUE 0

#include <boost/multiprecision/cpp_dec_float.hpp>

typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<5>> matrix_member;
typedef unsigned int matrix_size;

class CMatrixRow
{
private:
	const matrix_size column_count;
	matrix_member* values;

public:
	CMatrixRow& operator/=(const matrix_member& val);
	CMatrixRow& operator*=(const matrix_member& val);
	CMatrixRow& operator-=(const CMatrixRow& other);
	CMatrixRow& operator+=(const CMatrixRow& other);

	void set_value(matrix_size column, const matrix_member& value);

	matrix_size get_column_count() const;
	const matrix_member& get_column(matrix_size column) const;

	CMatrixRow(const CMatrixRow& original);
	CMatrixRow(matrix_size column_count);
	~CMatrixRow();
};

class CMatrix
{
private:
	const matrix_size row_count;
	int swap_coefficient;
	CMatrixRow** rows;

public:
	int get_swap_coefficient() const;
	void set_value(matrix_size row, matrix_size column, const matrix_member& value);
	void set_row(matrix_size row_index, CMatrixRow* row);

	matrix_size get_column_count() const;
	matrix_size get_row_count() const;
	void swap_rows(matrix_size row_index_1, matrix_size row_index_2);

	CMatrixRow* get_row(matrix_size row) const;
	const matrix_member& get_value(matrix_size row, matrix_size column) const;

	CMatrix(const CMatrix& original);
	CMatrix(CMatrix&& matrix);
	CMatrix(matrix_size rows, matrix_size columns);

	~CMatrix();
};

CMatrixRow operator*(const CMatrixRow& matrix_row, const matrix_member& val);
CMatrixRow operator/(const CMatrixRow& matrix_row, const matrix_member& val);
CMatrixRow operator+(const CMatrixRow& matrix_row, const CMatrixRow& other);
CMatrixRow operator-(const CMatrixRow& matrix_row, const CMatrixRow& other);

#endif // !_MATRIX_H_
