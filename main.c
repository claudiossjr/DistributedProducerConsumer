/*
  * Problem: Producer and Consumer distributed Problem using openMPI
  * Author: Claudio Santos
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <semaphore.h>

#define MAXBUFFER 10
#define COORDINATORLABEL 0
#define PRODUCERLABEL 1
#define CONSUMERLABEL 2

void * ProducerListener();
void * ConsumerListener();
int producedItens(int buffer[MAXBUFFER]);
void SendData();

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
sem_t mutex_produtor; // Up - Down
sem_t mutex_consumidor; // True - False
sem_t mutex_buffer; // True - False

//pthread_mutex_init (&mutex_produtor, MAXBUFFER);
//pthread_mutex_init (&mutex_consumidor, 0);
//pthread_mutex_init (&mutex_buffer, 1);

int main(int argc, char **argv)
{
  if (sem_init(&mutex_produtor, 0, 1) != 0)
	{
    exit(1);
	}

  if (sem_init(&mutex_consumidor, 0, 1) != 0)
	{
    exit(1);
	}

  if (sem_init(&mutex_buffer, 0, 1) != 0)
	{
    exit(1);
	}

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

// =============================
// FUNCTIONS
// =============================

void * ProducerListener()
{
  MPI_Status stats;
  printf("ProducerListener\n");
  while (1 != 0)
  {
    int message;
    MPI_Recv (&message, 1, MPI_INT, MPI_ANY_SOURCE, PRODUCERLABEL, MPI_COMM_WORLD, &stats);
    printf("%d\n",message );

	// Down no semáforo do produtor	
	//pthread_mutex_lock(&mutex_produtor);
	
	if(producedItens(buffer) < MAXBUFFER)
	{
		int answer = -400;
		SendData(answer, stats);
				
		int message;
		MPI_Recv (&message, 1, MPI_INT, MPI_ANY_SOURCE, PRODUCERLABEL, MPI_COMM_WORLD, &stats);
		printf("%d\n",message );
		
		// Lock! Semáforo do buffer (utilização)
		//pthread_mutex_lock(&mutex_buffer);
		
		buffer[producerIndex] = message;
		producerIndex = producerIndex < MAXBUFFER ? producerIndex + 1 : 0;	
		
		// Unlock! Semáforo do buffer (utilização)
		sem_post(&mutex_buffer);
		
		// Up semáforo do produtor	
		sem_post(&mutex_produtor);				
	}
  }
}

void * ConsumerListener()
{
  MPI_Status stats;
  printf("ConsumerListener\n");
  while (1 != 0)
  {
    int message;
    MPI_Recv (&message, 1, MPI_INT, MPI_ANY_SOURCE, CONSUMERLABEL, MPI_COMM_WORLD, &stats);
    printf("%d\n",message );
    
    // Down semáforo do consumidor
    sem_wait(&mutex_consumidor);	
    
    if(message == -102)
    {
		if(buffer != NULL && producedItens(buffer))
		{			
			// Lock! Semáforo do buffer (utilização)
			sem_wait(&mutex_buffer);	
			
			int message = buffer[consumerIndex];	
			SendData(message, stats);	
			
			buffer[consumerIndex] = 0;			
			consumerIndex = consumerIndex < MAXBUFFER ? consumerIndex + 1 : 0;	
				
			// Unlock! Semáforo do buffer (utilização)
			sem_post(&mutex_buffer);						
		}
		else
		{
			int answer = -401;
			MPI_Send (&answer, 1, MPI_INT, stats.MPI_SOURCE, COORDINATORLABEL, MPI_COMM_WORLD);	
			printf("%d\n",answer );			
			
			int message = buffer[consumerIndex];	
			SendData(message, stats);	
			
			buffer[consumerIndex] = 0;			
			consumerIndex = consumerIndex < MAXBUFFER ? consumerIndex + 1 : 0;
		}
		
		// Up semáforo do produtor
		sem_post(&mutex_consumidor);
	}    
  }
}

void SendData(int message, MPI_Status stats)
{	
	MPI_Send (&message, 1, MPI_INT, stats.MPI_SOURCE, COORDINATORLABEL, MPI_COMM_WORLD);	
	printf("%d\n", message);			
}

int producedItens(int buffer[MAXBUFFER])
{
	int i, total = 0;
	
	for(i=0; i<MAXBUFFER; i++)
	{
		if(buffer[i])
		{
			total++;
		}
	}
	
	return total;
}
