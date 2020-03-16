/*
 * 2. PRL project, Odd-even transposition sort
 *
 * author: Bc. Matej Karas
 * email: xkaras34@stud.fit.vutbr.cz
 *
 */

#pragma once 
#include <mpi.h>
#include <vector>

enum class ComDir
{
    EVEN,
    ODD
};

// Forward declarations
class Process;

// Algorithm variant with sends and recieves
int sendrecv(Process& process);

// Algorithm variant with virtual topology
int topology(Process& process);

// Random number loader
std::vector<uint8_t> loadNumbers(const std::string& path);

// Creates graph topology like
// 0 1-2 3-4 5-6 
MPI_Comm oddTopology(Process& proc);

// Creates graph topology like
// 0-1 2-3 4-5 6
MPI_Comm evenTopology(Process& proc);

// Swaps current process number and neighbour number 
// if conditions are met -- actuall sorting
void swap(Process& proc, uint8_t& myNum, uint8_t& neiNum, ComDir dir);

// Helper class for current MPI process
class Process
{
public:
    static const int ROOT = 0;

public:
    Process(int& argc, char**& argv);
    ~Process();

    int& rank();
    int& worldSize();
    bool isRoot();
    bool isOdd();

private:
    int mRank;
    int mWorldSize;

};
