/*
 * 2. PRL project, Odd-even transposition sort
 *
 * author: Bc. Matej Karas
 * email: xkaras34@stud.fit.vutbr.cz
 *
 */

#include "ots.h"
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std::chrono;
using clk = high_resolution_clock;

int main(int argc, char *argv[])
{
	Process process(argc, argv);

	#if TOPOLOGY
		return topology(process);
	#else
		return sendrecv(process);
	#endif
}

int sendrecv(Process& process)
{
	uint8_t myNumber;
	uint8_t neighbourNumber;
	
	// Set ranks for even/odd targets
	int oddTargetRank = process.rank() + (process.isOdd() & 1 ? 1 : -1);
	int evenTargetRank = process.rank() + (process.isOdd() & 1 ? -1 : 1);

	if (oddTargetRank < 0 || oddTargetRank == process.worldSize())
		oddTargetRank = MPI_PROC_NULL;
	if (evenTargetRank == process.worldSize())
		evenTargetRank = MPI_PROC_NULL;

	#if BENCHMARK
		// Start clock for benchmark
		auto start = clk::now();
		auto begin = start;
	#endif

	// Load numbers with root process and print them
	std::vector<uint8_t> numbers;
	if (process.isRoot())
	{
		numbers = loadNumbers("numbers");
		
		#if BENCHMARK
			std::cout << "Element count: " << process.worldSize() << std::endl;
			std::cout << "Load time: " << duration_cast<std::chrono::duration<float, std::milli>>(clk::now() - start).count() << "ms" << std::endl;
			start = clk::now();
		#else
			for (size_t i = 0; i < numbers.size() - 1; ++i)
				std::cout << static_cast<int>(numbers[i]) << " ";

			std::cout << static_cast<int>(numbers.back()) << std::endl;
		#endif
	}

	// Scatter numbers across processes
	MPI_Scatter(numbers.data(), 1, MPI_UINT8_T, &myNumber, 1, MPI_UINT8_T, Process::ROOT, MPI_COMM_WORLD);

	// Sort numbers
	for (size_t i = 0; i < process.worldSize() / 2; ++i)
	{
		neighbourNumber = myNumber;
		MPI_Sendrecv(&myNumber, 1, MPI_UINT8_T, oddTargetRank, 0, &neighbourNumber, 1, MPI_UINT8_T, oddTargetRank, 0, MPI_COMM_WORLD, nullptr);
		swap(process, myNumber, neighbourNumber, ComDir::ODD);

		neighbourNumber = myNumber;
		MPI_Sendrecv(&myNumber, 1, MPI_UINT8_T, evenTargetRank, 0, &neighbourNumber, 1, MPI_UINT8_T, evenTargetRank, 0, MPI_COMM_WORLD, nullptr);
		swap(process, myNumber, neighbourNumber, ComDir::EVEN);
	}

	// Gather sorted numbers 
	MPI_Gather(&myNumber, 1, MPI_UINT8_T, &numbers[0], 1, MPI_UINT8_T, Process::ROOT, MPI_COMM_WORLD);

	if (process.isRoot())
	{
	#if BENCHMARK
		// measure time, and print
		std::cout << "Algorithm: " << duration_cast<std::chrono::duration<float, std::milli>>(clk::now() - start).count() << "ms" << std::endl;
		std::cout << "Final Time: " << duration_cast<std::chrono::duration<float, std::milli>>(clk::now() - begin).count() << "ms" << std::endl;
	#else
		for (const auto& num : numbers)
			std::cout << static_cast<int>(num) << "\n";
	#endif
	}

	return EXIT_SUCCESS;
}

int topology(Process& process)
{
	uint8_t myNumber;
	uint8_t neighbourNumber;

	#if BENCHMARK
		// Start clock for benchmark
		auto start = clk::now();
		auto begin = start;
	#endif

	// Load numbers with root process and print them
	std::vector<uint8_t> numbers;
	if (process.isRoot())
	{
		numbers = loadNumbers("numbers");
		
		#if BENCHMARK
			std::cout << "Element count: " << process.worldSize() << std::endl;
			std::cout << "Load time: " << duration_cast<std::chrono::duration<float, std::milli>>(clk::now() - start).count() << "ms" << std::endl;
			start = clk::now();
		#else
			for (size_t i = 0; i < numbers.size() - 1; ++i)
				std::cout << static_cast<int>(numbers[i]) << " ";

			std::cout << static_cast<int>(numbers.back()) << std::endl;
		#endif

		#if BENCHMARK
			start = clk::now();
		#endif
	}

	// Create topologies
	auto oddCom = oddTopology(process);
	auto evenCom = evenTopology(process);

	#if BENCHMARK
		// topology creation time
		if (process.isRoot())
			std::cout << "Topology: " << duration_cast<std::chrono::duration<float, std::milli>>(clk::now() - start).count() << "ms" << std::endl;

		start = clk::now();
	#endif

	// Scatter numbers across processes
	MPI_Scatter(numbers.data(), 1, MPI_UINT8_T, &myNumber, 1, MPI_UINT8_T, Process::ROOT, MPI_COMM_WORLD);

	// Sort numbers
	for (size_t i = 0; i < process.worldSize() / 2; ++i)
	{
		neighbourNumber = myNumber;
		MPI_Neighbor_alltoall(&myNumber, 1, MPI_UINT8_T, &neighbourNumber, 1, MPI_UINT8_T, oddCom);
		swap(process, myNumber, neighbourNumber, ComDir::ODD);

		neighbourNumber = myNumber;
		MPI_Neighbor_alltoall(&myNumber, 1, MPI_UINT8_T, &neighbourNumber, 1, MPI_UINT8_T, evenCom);
		swap(process, myNumber, neighbourNumber, ComDir::EVEN);
	}

	// Gather sorted numbers 
	MPI_Gather(&myNumber, 1, MPI_UINT8_T, &numbers[0], 1, MPI_UINT8_T, Process::ROOT, MPI_COMM_WORLD);

	if (process.isRoot())
	{
	#if BENCHMARK
		// measure time, and print
		std::cout << "Algorithm: " << duration_cast<std::chrono::duration<float, std::milli>>(clk::now() - start).count() << "ms" << std::endl;
		std::cout << "Final Time: " << duration_cast<std::chrono::duration<float, std::milli>>(clk::now() - begin).count() << "ms" << std::endl;
	#else
		for (const auto& num : numbers)
			std::cout << static_cast<int>(num) << "\n";
	#endif
	}

	// todo RAII
	MPI_Comm_free(&oddCom);
	MPI_Comm_free(&evenCom);

	return EXIT_SUCCESS;
}

////////////////////////////////////////////////
/// Functions
////////////////////////////////////////////////
std::vector<uint8_t> loadNumbers(const std::string& path)
{
	std::ifstream source(path, std::ios::in | std::ios::binary);
	return std::vector<uint8_t>(std::istreambuf_iterator<char>(source), std::istreambuf_iterator<char>());
}

MPI_Comm oddTopology(Process& proc)
{
	MPI_Comm newCom;
	int target = proc.rank() + (proc.rank() & 1 ? 1 : -1);
	int degree = 1;

	if (proc.rank() == 0 || target == proc.worldSize())
		MPI_Dist_graph_create(MPI_COMM_WORLD, 0, nullptr, nullptr, nullptr, nullptr, MPI_INFO_NULL, false, &newCom);
	else
		MPI_Dist_graph_create(MPI_COMM_WORLD, 1, &proc.rank(), &degree, &target, &degree, MPI_INFO_NULL, false, &newCom);

	return newCom;
}

MPI_Comm evenTopology(Process& proc)
{
	MPI_Comm newCom;
	int target = proc.rank() + (proc.rank() & 1 ? -1 : 1);
	int degree = 1;

	if (target == proc.worldSize())
		MPI_Dist_graph_create(MPI_COMM_WORLD, 0, nullptr, nullptr, nullptr, nullptr, MPI_INFO_NULL, false, &newCom);
	else
		MPI_Dist_graph_create(MPI_COMM_WORLD, 1, &proc.rank(), &degree, &target, &degree, MPI_INFO_NULL, false, &newCom);

	return newCom;
}

void swap(Process& proc, uint8_t& myNum, uint8_t& neiNum, ComDir dir)
{
	// Swaps numbers if procesor at rank i, has number greater than procesor at rank i + 1
	// Some magic which works. Consider this table, which is function of XOR
	// 				DirE E 0
	// 				DirE O 1
	//				DirO E 1
	// 				DirO O 0
	if ((static_cast<int>(dir) ^ static_cast<int>(proc.isOdd())) == 0)
		myNum = (myNum > neiNum) ? neiNum : myNum;
	else
		myNum = (myNum > neiNum) ? myNum : neiNum;
}

////////////////////////////////////////////////
/// Process class
////////////////////////////////////////////////
Process::Process(int& argc, char**& argv)
{
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mWorldSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &mRank);
}

Process::~Process()
{
	MPI_Finalize();
}

int& Process::rank()
{
	return mRank;
}

int& Process::worldSize()
{
	return mWorldSize;
}

bool Process::isRoot()
{
	return mRank == ROOT;
}

bool Process::isOdd()
{
	return static_cast<bool>(mRank & 1);
}
