#include "MatrixUtils.h"

matrix_exception::matrix_exception(const std::string& message) : std::runtime_error(message)
{
}

matrix_member multiply_matrix_diagonal(const CMatrix& matrix)
{
	if (matrix.get_row_count() == 0) // empty matrix
		return 0;

	matrix_member rtn = 1;
	const matrix_size matrix_row_count = matrix.get_row_count();

	for (matrix_size i = 0; i < matrix_row_count; i++)
	{
		rtn *= matrix.get_value(i, i);
		
		if (rtn == 0)
			return 0;
	}

	return rtn;
}

CMatrix parse_matrix(std::istream& stream, const char line_delimiter)
{
	matrix_size row_count = 0, column_count = 0, max_columns = 0;
	std::vector<matrix_member> row_values;
	std::vector<std::vector<matrix_member>> parsed_values;

	while (!stream.eof())
	{
		const char next_char = stream.peek();

		if (next_char == line_delimiter) // new row
		{
			stream.ignore();
			row_count++;

			if (column_count > max_columns)
				max_columns = column_count;

			column_count = 0;
			parsed_values.push_back(std::move(row_values));
			continue;
		}

		if (!isdigit(next_char) && next_char != '-') // ignore any unknown character 
		{
			stream.ignore();
			continue;
		}

		matrix_member value;
		stream >> value;
		row_values.push_back(value);
		column_count++;
	}

	// there's no line_delimiter after last row
	row_count++; 
	parsed_values.push_back(row_values);
	if (column_count > max_columns)
		max_columns = column_count;

	CMatrix parsed_matrix(row_count, max_columns); // all values are initially 0

	// create CMatrix from parsed values
	for (matrix_size row = 0; row < row_count; row++)
	{
		std::vector<matrix_member>& row_values = parsed_values[row];

		for (matrix_size column = 0; column < row_values.size(); column++)
			parsed_matrix.set_value(row, column, row_values[column]);
	}

	return parsed_matrix;
}

CMatrix singlethread_gem_matrix(CMatrix&& matrix)
{
	CMatrix gem_matrix(std::move(matrix));
	const matrix_size matrix_row_count = gem_matrix.get_row_count();

	// with each outer loop iteration i-th row is eliminated, inner loop does the elimination (subtracts i-th row from j-th row)
	for (matrix_size i = 0; i < matrix_row_count; i++)
	{
		for (matrix_size j = i + 1; j < matrix_row_count; j++)
		{
			eliminate_matrix_row(gem_matrix, i, j);
		}
	}

	return gem_matrix;
}

CMatrix singlethread_gem_matrix(const CMatrix& original_matrix)
{
	CMatrix gem_matrix = CMatrix(original_matrix);
	return singlethread_gem_matrix(std::move(gem_matrix));
}

void eliminate_matrix_row(CMatrix& matrix, matrix_size& previous_row_index, matrix_size& current_row_index)
{
	CMatrixRow* previous_row = matrix.get_row(previous_row_index);

	if (previous_row_index < previous_row->get_column_count() && previous_row->get_column(previous_row_index) == 0)
	{
		matrix.swap_rows(previous_row_index, current_row_index);
		previous_row = matrix.get_row(previous_row_index);
	}
	CMatrixRow* current_row = matrix.get_row(current_row_index);

	matrix_member coef = previous_row_index >= previous_row->get_column_count() || previous_row->get_column(previous_row_index) == 0 ? matrix_member(1) : current_row->get_column(previous_row_index) / previous_row->get_column(previous_row_index);
	*current_row -= *previous_row * coef;
}