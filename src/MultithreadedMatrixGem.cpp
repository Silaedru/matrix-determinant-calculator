#include <algorithm>

#include "MultithreadedMatrixGem.h"
#include "MatrixUtils.h"
#include "Util.h"

// performs elimination between current_row and previous_row, starts at start column and skips column_step-1 columns during elimination
inline void solve_row(
	CMatrixRow*& current_row,
	CMatrixRow*& previous_row,
	const matrix_member& coef,
	const matrix_size& matrix_column_count,
	const matrix_size& start,
	const matrix_size& column_step)
{
	for (matrix_size column = start; column < matrix_column_count; column += column_step)
		current_row->set_value(column, current_row->get_column(column) - previous_row->get_column(column) * coef);
}

CSolver::CSolver(
	const unsigned int this_thread_num,
	const unsigned int total_thread_count, 
	const matrix_size matrix_column_count)
	: 
	column_count(matrix_column_count),
	finished(true), // solver starts with no work
	thread_num(this_thread_num),
	thread_count(total_thread_count),
	run(true),
	worker_thread(std::thread(&CSolver::work, this))
{
}

CSolver::~CSolver()
{
	// stop worker thread if it is still running
	if (run)
		stop();

	// wait for it to finish in case it's still doing something
	if (worker_thread.joinable())
		worker_thread.join();
}

void CSolver::set_work(
	CMatrixRow* current_row,
	CMatrixRow* previous_row,
	const matrix_member& current_coef)
{
	if (!this->finished)
		throw matrix_exception("assigning new CSolver work when CSolver isn't finished");

	std::lock_guard<std::mutex> lock(guard); // lock condition variable mutex 
	this->current_coef = current_coef;
	this->current_row = current_row;
	this->previous_row = previous_row;
	this->finished = false;
	work_sync.notify_all(); // notify worker thread
}

void CSolver::stop()
{
	std::lock_guard<std::mutex> lock(guard); // lock condition variable mutex 
	this->run = false;
	work_sync.notify_all();
}

bool CSolver::is_finished() const
{
	return finished;
}

void CSolver::work()
{
	std::unique_lock<std::mutex> lock(guard); // lock for condition variable mutex

	while (run)
	{
		if (!finished)
		{
			solve_row(current_row, previous_row, current_coef, column_count, thread_num, thread_count);
			finished = true;
		}

		work_sync.wait(lock); // wait for more work
	}
}

CMultithreadedMatrixGem::CMultithreadedMatrixGem(CMatrix&& source_matrix) : result_computed(false), sync_wait_time(0), matrix(std::move(source_matrix))
{
	// don't make more threads than there is matrix columns
	num_threads = (unsigned int)std::min(std::max((int)std::thread::hardware_concurrency(), 1), (int)matrix.get_column_count());
}

CMultithreadedMatrixGem::CMultithreadedMatrixGem(const CMatrix& source_matrix) : CMultithreadedMatrixGem(CMatrix(source_matrix))
{
}

void CMultithreadedMatrixGem::set_num_threads(unsigned int num_threads)
{
	// no point in updating num_threads if the result is already computed
	if (!this->result_computed)
		this->num_threads = num_threads;
}

unsigned int CMultithreadedMatrixGem::get_num_threads() const
{
	return num_threads;
}

millisecond_time_difference CMultithreadedMatrixGem::get_setup_time() const
{
	return setup_time;
}

millisecond_time_difference CMultithreadedMatrixGem::get_computation_time() const
{
	return computation_time;
}

millisecond_time_difference CMultithreadedMatrixGem::get_sync_wait_time() const
{
	return sync_wait_time;
}

millisecond_time_difference CMultithreadedMatrixGem::get_cleanup_time() const
{
	return cleanup_time;
}

const CMatrix& CMultithreadedMatrixGem::get_result()
{
	if (!result_computed)
		compute_result(); 

	return matrix;
}

void CMultithreadedMatrixGem::compute_result()
{
	if (result_computed)
		return;

	time_value setup_start = get_current_time();

	// create workers
	std::vector<std::unique_ptr<CSolver>> solvers;
	for (unsigned int i = 1; i < num_threads; i++)
		solvers.push_back(std::make_unique<CSolver>(i, num_threads, matrix.get_column_count()));

	const matrix_size matrix_row_count = matrix.get_row_count();
	const matrix_size matrix_column_count = matrix.get_column_count();

	time_value computation_start = get_current_time();

	// start at second matrix row
	for (matrix_size current_row = 1; current_row < matrix_row_count; current_row++)
	{
		CMatrixRow* current_row_ptr = matrix.get_row(current_row); 

		// for all preceding rows
		for (matrix_size previous_row = 0; previous_row < current_row; previous_row++)
		{
			CMatrixRow* previous_row_ptr = matrix.get_row(previous_row);

			matrix_member div = 0; 
			
			if (previous_row < matrix_column_count)
				div = previous_row_ptr->get_column(previous_row);
		
			if (div == 0)
			{
				matrix.swap_rows(previous_row, current_row);
				
				current_row_ptr = matrix.get_row(current_row);
				previous_row_ptr = matrix.get_row(previous_row);

				if (previous_row < matrix_column_count)
					div = previous_row_ptr->get_column(previous_row);
			}

			matrix_member coef = previous_row >= matrix_column_count || div == 0 ? matrix_member(1) : (current_row_ptr->get_column(previous_row) / div);
			
			// assign work to workers
			for (auto& solver : solvers)
				solver->set_work(current_row_ptr, previous_row_ptr, coef);
			
			// no point in just waiting - main thread will compute as well (as thread "0")
			solve_row(current_row_ptr, previous_row_ptr, coef, matrix_column_count, 0, num_threads);

			time_value sync_wait_start = get_current_time();
	
			// wait for all workers to finish with their current assigment before continuing further
			// main thread has already finished computing - that's why this is waiting for num_threads-1
			for (unsigned int thread_num = 0; thread_num < num_threads-1; )
			{
				if (solvers[thread_num]->is_finished())
					thread_num++;
			}

			time_value sync_wait_end = get_current_time();
			sync_wait_time += time_diff(sync_wait_end, sync_wait_start);
		}
	}

	time_value cleanup_start = get_current_time();
	
	// stop all workers
	solvers.clear(); // this will clear the std::vector of workers, which will result in unique ptrs being destroyed, which will result in CSolver destructor call, which does the actual thread stopping and cleanup

	result_computed = true;
	time_value cleanup_finished_time = get_current_time();

	setup_time = time_diff(computation_start, setup_start);
	computation_time = time_diff(cleanup_start, computation_start);
	cleanup_time = time_diff(cleanup_finished_time, cleanup_start);
}
