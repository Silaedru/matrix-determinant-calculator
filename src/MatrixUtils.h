#ifndef _MATRIX_UTILS_H_
#define _MATRIX_UTILS_H_

#include <exception>
#include <stdexcept>
#include <ios>

#include "Matrix.h"

class matrix_exception : public std::runtime_error
{
public:
	matrix_exception(const std::string& message);
};

inline bool is_matrix_square(const CMatrix& matrix)
{
	return matrix.get_row_count() == matrix.get_column_count();
}

matrix_member multiply_matrix_diagonal(const CMatrix& matrix);
CMatrix parse_matrix(std::istream& stream, const char line_delimiter);

CMatrix singlethread_gem_matrix(CMatrix&& matrix);
CMatrix singlethread_gem_matrix(const CMatrix& original_matrix);
void eliminate_matrix_row(CMatrix& matrix, matrix_size& previous_row_index, matrix_size& current_row_index);

#endif // !_MATRIX_UTILS_H_
