#ifndef _MULTITHREADED_MATRIX_GEM_H_
#define _MULTITHREADED_MATRIX_GEM_H_

#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "Util.h"
#include "Matrix.h"

class CSolver
{
private:
	CMatrixRow* current_row;
	CMatrixRow* previous_row;
	matrix_member current_coef;

	const matrix_size column_count;

	std::atomic_bool finished;
	const unsigned int thread_num;
	const unsigned int thread_count;

	std::atomic_bool run;
	std::mutex guard;
	std::condition_variable work_sync;
	std::thread worker_thread;

	void work();

public:
	bool is_finished() const;
	void stop();
	void set_work(CMatrixRow* current_row, CMatrixRow* previous_row, const matrix_member& current_coef);

	CSolver(unsigned int this_thread_num, unsigned int total_thread_count, matrix_size matrix_column_count);
	~CSolver();
};

class CMultithreadedMatrixGem
{
private:
	bool result_computed;
	unsigned int num_threads;

	millisecond_time_difference setup_time;
	millisecond_time_difference computation_time;
	millisecond_time_difference cleanup_time;
	millisecond_time_difference sync_wait_time;

	CMatrix matrix;

public:
	unsigned int get_num_threads() const;

	millisecond_time_difference get_setup_time() const;
	millisecond_time_difference get_computation_time() const;
	millisecond_time_difference get_cleanup_time() const;
	millisecond_time_difference get_sync_wait_time() const;

	void set_num_threads(unsigned int num_threads);

	const CMatrix& get_result();
	void compute_result();

	CMultithreadedMatrixGem(CMatrix&& source_matrix);
	CMultithreadedMatrixGem(const CMatrix& source_matrix);
};
#endif // !_MULTITHREADED_MATRIX_GEM_H_
