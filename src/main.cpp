#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "Matrix.h"
#include "MatrixUtils.h"
#include "MultithreadedMatrixGem.h"

#define PARSING_LINE_DELIMITER '/'

bool use_singlethread_impl = false;
bool print_perf_info = false;
int num_threads = 0;

millisecond_time_difference parsing_time = 0;

void print_format_info()
{
	std::cout << std::endl <<
		"Expected matrix format: " << std::endl <<
		"'/' as row separator, ' ' (space) as column separator, '.' as decimal point. Any unknown character is ignored. Number of columns is based on a row with the most specified columns. Missing values are replaced with 0." << std::endl << std::endl <<
		
		"Example 2x2 identity matrix:    1 0/" << std::endl <<
		"                                0 1" << std::endl << std::endl <<

		"Example of a more complex matrix:    2.5  3/" << std::endl <<
		"                                     0  0  1/" << std::endl << std::endl <<
		"is parsed as   2.5   3   0" << std::endl <<
		"               0     0   1" << std::endl <<
		"               0     0   0" << std::endl <<
		"because second row has 3 columns, first row does not have third column specified and third row does not have any column specified." << std::endl <<
		"";
}

void print_help()
{
	std::cout << std::endl <<
		"MatrixDeterminant: computes determinant of a square matrix" << std::endl << std::endl <<

		"Usage: " << " MatrixDeterminant [OPTIONS] INPUT" << std::endl << std::endl <<

		"Expected INPUT is a file containing the matrix to be processed, unless the -m option is used." << std::endl << std::endl <<

		"Available OPTIONS: " << "-s         Singlethread implementation will be used for computing the result." << std::endl <<
		"                   " << "-t NUMBER  NUMBER of threads will be used for computing the result. This option is ignored if used with the -s option." << std::endl <<
		"                   " << "-p         Prints performance statistics." << std::endl <<
		"                   " << "-m         INPUT will be processed directly (as a matrix)." << std::endl <<
		"                   " << "-h, -help  Prints this message. No input is required and any provided input will be ignored." << std::endl <<
		"                   " << "-f         Prints information about expected matrix format. No input is required and any provided input will be ignored." << std::endl <<
		"";
}

CMatrix parse_matrix_from_file(const std::string& filename)
{
	std::ifstream file(filename);

	if (!file.is_open())
		throw std::runtime_error("cannot open file \"" + filename + "\"");

	CMatrix matrix = parse_matrix(file, PARSING_LINE_DELIMITER);
	file.close();
	return matrix;
}

CMatrix parse_matrix_from_string(const std::string& str)
{
	std::stringstream stream(str);
	return parse_matrix(stream, PARSING_LINE_DELIMITER);
}

CMatrix get_gemed_matrix(CMatrix&& source_matrix)
{
	if (use_singlethread_impl)
	{
		time_value st_start = get_current_time();
		CMatrix gemed_matrix = singlethread_gem_matrix(std::move(source_matrix));
		time_value st_end = get_current_time();

		if (print_perf_info)
		{
			millisecond_time_difference computation_time = time_diff(st_end, st_start);
			std::cout << "Parsing time: " << parsing_time << "ms" << std::endl;
			std::cout << "Singlethread gauss elimination time: " << computation_time << "ms" << std::endl;
			std::cout << "=== TOTAL TIME: " << parsing_time + computation_time << "ms ===" << std::endl;
		}

		return gemed_matrix;
	}
	else
	{
		CMultithreadedMatrixGem solver(std::move(source_matrix));

		if (num_threads > 0)
			solver.set_num_threads(num_threads);

		solver.compute_result();

		if (print_perf_info)
		{
			std::cout << "Parsing time: " << parsing_time << "ms" << std::endl;
			std::cout << std::endl;
			std::cout << "Multithreaded gauss elimination performance statistics:" << std::endl;
			std::cout << "Setup time: " << solver.get_setup_time() << "ms" << std::endl;
			std::cout << "Computation time: " << solver.get_computation_time() << "ms" << std::endl;
			std::cout << "Time spent synchronizing: " << solver.get_sync_wait_time() << "ms" << std::endl;
			std::cout << "Cleanup time: " << solver.get_cleanup_time() << "ms" << std::endl;
			std::cout << "TOTAL GEM TIME: " << solver.get_setup_time() + solver.get_computation_time() + solver.get_sync_wait_time() + solver.get_cleanup_time() << "ms" << std::endl;
			std::cout << "=== TOTAL TIME: " << solver.get_setup_time() + solver.get_computation_time() + solver.get_sync_wait_time() + solver.get_cleanup_time() + parsing_time << "ms ===" << std::endl;
			std::cout << "Threads used: " << solver.get_num_threads() << std::endl;
		}

		return solver.get_result();
	}
}

CMatrix get_gemed_matrix(const CMatrix& source_matrix)
{
	return get_gemed_matrix(std::move(CMatrix(source_matrix)));
}

int calculate_matrix_determinant(CMatrix&& source_matrix)
{
	if (!is_matrix_square(source_matrix))
	{	
		std::cerr << "Cannot compute determinant of a matrix that is not square (input matrix has " << source_matrix.get_column_count() << " columns and " << source_matrix.get_row_count() << " rows)" << std::endl;
		return -1;
	}

	CMatrix gemed_matrix = get_gemed_matrix(std::move(source_matrix));

	matrix_member determinant = multiply_matrix_diagonal(gemed_matrix) * gemed_matrix.get_swap_coefficient();

	if (print_perf_info)
		std::cout << std::endl << "Determinant: ";

	std::cout << std::setprecision(5) << determinant << std::endl;

	return 0;
}

int calculate_matrix_determinant(const CMatrix& source_matrix)
{
	return calculate_matrix_determinant(std::move(CMatrix(source_matrix)));
}

int exit_on_invalid_args()
{
	std::cerr << "Error: invalid usage. Use -h to print help." << std::endl;
	return -2;
}

int main(int argc, char** argv)
{
	/*
	 * args:
	 *  -s use singlethread impl
	 *  -t [#] use # threads
	 *  -p show perf info
	 *  -h, -help show help
	 *  -m direct input
	 *  -f show format info
	 *
	 * retval:
	 *   0: ok
	 *  -1: error while calculating
	 *  -2: invalid usage
	 *  -3: exception while processing
	 */

	int i = 1;
	bool direct_input = false;
	std::string user_arg;

	if (argc < 2)
		return exit_on_invalid_args();

	// parse options
	for ( ; i < argc; i++)
	{
		char* curr_arg = argv[i];

		if (strcmp("-help", curr_arg) == 0)
		{
			print_help();
			return 0;
		}

		if (*curr_arg == '-')
		{
			curr_arg++;

			if (*(curr_arg + 1))
				return exit_on_invalid_args();
			
			if (isdigit(*curr_arg))
				break;

			switch (*curr_arg)
			{
			case 's':
				use_singlethread_impl = true;
				break;

			case 'p':
				print_perf_info = true;
				break;

			case 'h':
				print_help();
				return 0;

			case 't':
				i++;

				if (argv[i] == nullptr)
					return exit_on_invalid_args();

				num_threads = atoi(argv[i]);

				if (num_threads <= 0)
				{
					num_threads = 0;
					std::cerr << "invalid number of threads specified, using default instead" << std::endl;
				}
				break;

			case 'm':
				direct_input = true;
				break;

			case 'f':
				print_format_info();
				return 0;

			default:
				return exit_on_invalid_args();
			}
		}
		else
			break;
	}

	// parse input
	for ( ; i < argc; i++)
	{
		user_arg.append(argv[i]);

		if (i != argc - 1)
			user_arg.append(" ");
	}

	if (user_arg.empty())
		return exit_on_invalid_args();

	CMatrix (*parsing_fun)(const std::string&) = &(direct_input ? parse_matrix_from_string : parse_matrix_from_file);

	try
	{
		time_value parsing_start = get_current_time();
		CMatrix parsed_matrix = parsing_fun(user_arg);
		parsing_time = time_diff(get_current_time(), parsing_start);
		return calculate_matrix_determinant(std::move(parsed_matrix));
	}
	catch (std::runtime_error& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return -3;
	}
}