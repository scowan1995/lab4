#include "mpi.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <vector>

// Don't CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------

void
CreateElectionArray(int numberOfProcesses,int electionArray[][2])
{

    std::vector<int> permutation;
    for(int i = 0; i < numberOfProcesses; ++i)
        permutation.push_back(i);
    std::random_shuffle(permutation.begin(), permutation.end());

    for(int i = 0; i < numberOfProcesses; ++i)
    {
        electionArray[i][0] = permutation[i];
        int chance = std::rand() % 4; // 25 % chance of inactive
        electionArray[i][1] = chance != 0; // 50% chance,
    }

    //Check that there is at least one active
    bool atLeastOneActive = false;
    for(int i = 0; i < numberOfProcesses; ++i)
    {
        if(electionArray[i][1] == 1)
            atLeastOneActive = true;
    }
    if(!atLeastOneActive)
    {
        electionArray[std::rand() % numberOfProcesses][1] = 1;
    }
}

void
PrintElectionArray(int numberOfProcesses, int electionArray[][2])
{
    for(int i = 0; i < numberOfProcesses; ++i)
    {
        std::printf("%-3d ", electionArray[i][0]);
    }
    std::cout << std::endl;
    for(int i = 0; i < numberOfProcesses; ++i)
    {
        std::printf("%-3d ", electionArray[i][1]);
    }
    std::cout << std::endl;
}
void
PrintElectionResult(int winnerMPIRank, int round, int numberOfProcesses, int electionArray[][2])
{
    std::cout << "Round " << round << std::endl;
    std::cout << "ELECTION WINNER IS " << winnerMPIRank << "(" << electionArray[winnerMPIRank][0] << ")  !!!!!\n";
    std::cout << "Active nodes where: ";
    for(int i = 0; i < numberOfProcesses; ++i)
    {
        if(electionArray[i][1] == 1)
            std::cout << i << "(" << electionArray[i][0] << "), ";
    }
    std::cout << std::endl;
    PrintElectionArray(numberOfProcesses, electionArray);
    for(int i = 0; i < numberOfProcesses*4-2; ++i)
        std::cout << "_";
    std::cout << std::endl;
}

// CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------

int
main(int argc, char* argv[])
{
    int processId;
    int numberOfProcesses;

    // Setup MPI
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &processId);
    MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses);

    // Two arguments, the program name, the number of rounds, and the random seed
    if(argc != 3)
    {
        if(processId == 0)
        {
            std::cout << "ERROR: Incorrect number of arguments. Format is: <numberOfRounds> <seed>" << std::endl;
        }
        MPI_Finalize();
        return 0;
    }

    const int numberOfRounds = std::atoi(argv[1]);
    const int seed           = std::atoi(argv[2]);
    std::srand(seed); // Set the seed

    auto electionArray = new int[numberOfProcesses][2]; // Bcast with &electionArray[0][0]...

    for(int round = 0; round < numberOfRounds; ++round)
    {
        if(processId == 0)
            CreateElectionArray(numberOfProcesses, electionArray);

        // ....... Your SPMD program goes here ............
        MPI_Bcast(&electionArray[0][0], numberOfProcesses*2, MPI_INT, 0, MPI_COMM_WORLD);
        auto sendArray = new int[numberOfProcesses];
        auto recvArray = new int[numberOfProcesses];
        for (int i = 0; i <numberOfProcesses; i++){
            sendArray[i] = -1;
            recvArray[i] = -1;
        }
        sendArray[processId] = electionArray[processId][0];
        auto candidateArray = new int[numberOfProcesses+1];
        candidateArray[numberOfProcesses] = processId;
        do {
            if (processId == 0) {
                MPI_Send(&candidateArray, numberOfProcesses + 1, MPI_INT, processId + 1, 0, MPI_COMM_WORLD);
            }
            else {
                MPI_Recv(&candidateArray, numberOfProcesses + 1, MPI_INT, processId - 1, 0, MPI_COMM_WORLD);
                candidateArray[processId] = 999;
                if (processId < numberOfProcesses-1){
                    MPI_Send(&candidateArray, numberOfProcesses+1);
                }
            }
        }while (processId<numberOfProcesses);


            if (processId == 0) {
                for (int i = 0; i< numberOfProcesses; i++){
                    std::cout<<candidateArray[i]<<std::endl;
                }
                int winner = 0; // CHANGE THIS
                PrintElectionResult(winner, round, numberOfProcesses, electionArray);
            }
    }

    delete[] electionArray;

    MPI_Finalize();

    return 0;
}
