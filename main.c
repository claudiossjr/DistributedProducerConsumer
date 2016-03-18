/*
  * Problem: Producer and Consumer distributed Problem using openMPI
  * Author: Claudio Santos
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>

#define MAXBUFFER 10
#define COORDINATORLABEL 0
#define PRODUCERLABEL 1
#define CONSUMERLABEL 2

void * ProducerListener();
void * ConsumerListener();


// =============================
// FUNCTIONS
// =============================

void * ProducerListener()
{
  MPI_Status stats;
  printf("ProducerListener\n");
  int message;
  MPI_Recv (&message, 1, MPI_INT, MPI_ANY_SOURCE, PRODUCERLABEL, MPI_COMM_WORLD, &stats);
  printf("%d\n",message );

  int answer = -401;
  MPI_Send (&answer, 1, MPI_INT, stats.MPI_SOURCE, COORDINATORLABEL, MPI_COMM_WORLD);
}

void * ConsumerListener()
{
  MPI_Status stats;
  printf("ConsumerListener\n");
  int message;
  MPI_Recv (&message, 1, MPI_INT, MPI_ANY_SOURCE, CONSUMERLABEL, MPI_COMM_WORLD, &stats);
  printf("%d\n",message );

  int answer = -400;
  MPI_Send (&answer, 1, MPI_INT, stats.MPI_SOURCE, COORDINATORLABEL, MPI_COMM_WORLD);
}

// =============================
// Initialize structures
// =============================

int producerIndex;
int consumerIndex;
int buffer[MAXBUFFER];

// =============================
// Thread structures
// =============================
pthread_t coordinatorProducerListener;
pthread_t coordinatorConsumerListener;

int main(int argc, char **argv)
{
  // =============================
  // Initialize MPI structures
  // =============================
  int myRank;
  int np;
  MPI_Status stats;
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  // =============================
  // Coordinator
  // =============================
  if (myRank == 0)
  {
    printf("Coordinator\n");
    if(pthread_create(&coordinatorProducerListener, NULL, ProducerListener,NULL) != 0)
    {
      exit(1);
    }

    if (pthread_create(&coordinatorConsumerListener , NULL, ConsumerListener,NULL) != 0)
    {
      exit(1);
    }

    if (pthread_join(coordinatorProducerListener, NULL) != 0)
    {
      exit(1);
    }

    if (pthread_join(coordinatorConsumerListener, NULL) != 0)
    {
      exit(1);
    }

  }
  // =============================
  // Producer
  // =============================
  else if (myRank % 2 == 0)
  {
    while (1 != 0)
    {
      int message = -101;
      MPI_Send (&message, 1, MPI_INT, COORDINATORLABEL, PRODUCERLABEL, MPI_COMM_WORLD );

      int answer;
      MPI_Recv (&answer, 1, MPI_INT, COORDINATORLABEL, COORDINATORLABEL, MPI_COMM_WORLD, &stats);
      printf("%d\n",answer );
    }
  }
  // =============================
  // Consumer
  // =============================
  else if (myRank % 2 != 0)
  {
    while (1 != 0)
    {
      int message = -102;
      MPI_Send (&message, 1, MPI_INT, COORDINATORLABEL, CONSUMERLABEL, MPI_COMM_WORLD );

      int answer;
      MPI_Recv (&answer, 1, MPI_INT, COORDINATORLABEL, COORDINATORLABEL, MPI_COMM_WORLD, &stats);
      printf("%d\n",answer );
    }
  }


  MPI_Finalize();
  return 0;
}
