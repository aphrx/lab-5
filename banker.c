/*
 * Banker's Algorithm for SOFE 3950U: Operating Systems
 *
 * Copyright (C) 2019, Amalnnath Parameswaran, Zeerak Siddiqui, Sachin Teckchandani, Mohtasim Siddiqui
 * All rights aren't reserved.
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "banker.h"

// Put any other macros or constants here using #define
#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 3


// Available amount of each resource
int available[NUM_RESOURCES];

// Maximum demand of each customer
int maximum[NUM_CUSTOMERS][NUM_RESOURCES];

// Amount currently allocated to each customer
int allocation[NUM_CUSTOMERS][NUM_RESOURCES];

// Remaining need of each customer
int need[NUM_CUSTOMERS][NUM_RESOURCES];

//Order at which each customer completed their task
int sequence[NUM_CUSTOMERS];

//Keep track of remaining customers
int done = 0;

//Mutex Lock
pthread_mutex_t lock;

//Checks if the state will be safe after the request
bool is_safe(){
	int checkRes[NUM_RESOURCES];
	bool finish[NUM_CUSTOMERS];
	int count = 0;
	bool finishCust = true;
	int prev = -1;
	int index = -1;

	printf("Available:");
	for (int i = 0; i < NUM_RESOURCES; i++)
	{
		printf(" %d", available[i]);
	}
	printf("\n");
	for (int i = 0; i < NUM_RESOURCES; ++i)
	{
		checkRes[i] = available[i];
	}

	for (int i = 0; i < NUM_CUSTOMERS; ++i)
	{
		finish[i] = false;
	}

	while (count < NUM_CUSTOMERS){
		prev = index;
		for (int i = 0; i < NUM_CUSTOMERS; ++i)
		{
			if(!finish[i]){
				for (int j = 0; j < NUM_RESOURCES; ++j)
				{
					if(need[i][j] > checkRes[j])
					{
						finishCust = false;
					}
				
				}
				if (finishCust)
				{
					index = i;
					for (int j = 0; j < NUM_RESOURCES; ++j)
					{
						checkRes[j] += allocation[i][j];
					}
					finish[i] = true;
					count++;
					finishCust = true;
					break;
				}
			}
		}
		for (int i = 0; i < NUM_CUSTOMERS; ++i)
		{
			if(!finish[i]){
				break;
			}
		}
		if(prev == index){
			return false;
		}
	}
	return true;
}

// Define functions declared in banker.h here
bool request_res(int n_customer, int request[])
{
	printf("Customer %d requests %d %d %d\n",n_customer, request[0], request[1], request[2]);
	for (int i = 0; i < NUM_RESOURCES; ++i)
	{
		if(request[i] <= need[n_customer][i]){
			if(request[i] > available[i]){
				printf("%s\n", "Unsafe [1]");
				return false;
			}
			else{
				for (int j = 0; j < NUM_RESOURCES; j++)
				{
					allocation[n_customer][j] += request[j];
					available[j] -= request[j];
					need[n_customer][j] = maximum[n_customer][j] - allocation[n_customer][j];
				}
				if(is_safe()){
					printf("%s\n", "Safe!");
					return true;
				}
				else{
					printf("%s\n", "Unsafe [2]");
					for (int j = 0; j < NUM_RESOURCES; j++)
					{
						allocation[n_customer][j] -= request[j];
						available[j] += request[j];
						//need[n_customer][j] += request[j];
						need[n_customer][j] = maximum[n_customer][j] + allocation[n_customer][j];
					}
					return false;
				}
			}

		}
		else{
			printf("%s\n", "Unsafe [3]");
			return false;
		}	
	}
	return true;
}

// Release resources, returns true if successful
bool release_res(int n_customer, int release[])
{
	for (int i = 0; i < NUM_RESOURCES; ++i)
	{
		available[i] += release[i];
		need[n_customer][i] += release[i];
		allocation[n_customer][i] -= release[i];
	}
	printf("Available:");
	for (int i = 0; i < NUM_RESOURCES; i++)
	{
		printf(" %d", available[i]);
	}
	printf("\n");
	return true;
}

// Print maximum resources for each customer
void print_max(){
	printf("%s\n", " Maximum");
    printf("%s\n", " =======");
    for (int i = 0; i < NUM_CUSTOMERS; ++i)
    {
    	for (int j = 0; j < NUM_RESOURCES; ++j)
    	{
    		printf(" %d ", maximum[i][j]);
    	}
    	printf("\n");
    }
    printf("\n");
}

// Print needed resources for each customer
void print_need(){
	printf("%s\n", " Need");
    printf("%s\n", " =======");
    for (int i = 0; i < NUM_CUSTOMERS; ++i)
    {
    	for (int j = 0; j < NUM_RESOURCES; ++j)
    	{
    		printf(" %d ", need[i][j]);
    	}
    	printf("\n");
    }
    printf("\n");
}

// Print allocated resources for each customer
void print_alloc(){
	printf("%s\n", " Alloc");
    printf("%s\n", " =======");
    for (int i = 0; i < NUM_CUSTOMERS; i++)
    {
    	for (int j = 0; j < NUM_RESOURCES; j++)
    	{
    		printf(" %d ", allocation[i][j]);
    	}
    	printf("\n");
    }
    printf("\n");
}

//Loops until process is completed
void *cust_loop(int cust){
	//int cust = cust_number;
	bool rec;
	int request[NUM_RESOURCES];

	while(1){
		for (int i = 0; i < NUM_RESOURCES; ++i)
		{

			request[i] = rand() % (maximum[cust][i]+1);
		}
		pthread_mutex_lock(&lock);
		rec = request_res(cust, request);

		if (rec){
			rec = false;
			release_res(cust, request);

			for (int i = 0; i < NUM_RESOURCES; ++i)
			{
				need[cust][i] = 0;
				maximum[cust][i] = 0;
			}
			print_max();
			print_need();
			print_alloc();
			pthread_mutex_unlock(&lock);

			if(done != NUM_CUSTOMERS){
				sequence[done] = cust;
				done++;
			}
			return 1;

		}
		pthread_mutex_unlock(&lock);

	}
	return 0;
}

int main(int argc, char *argv[])
{

	pthread_t t[NUM_CUSTOMERS];
	pthread_mutex_init(&lock, NULL);

	// Make sure 3 arguments are provided
    if(argc == 4) {
    	for (int i = 0; i < NUM_RESOURCES; i++)
    	{
    		//Get parameters
	    	available[i] = atoi(argv[i+1]);

	    	for (int j = 0; j < NUM_CUSTOMERS; j++)
	    	{
	    		//Set Max, Need and Alloc arrays
	    		maximum[j][i] = rand() % (available[i]+1);
	    		allocation[j][i] = rand() % (maximum[j][i]+1);
	    		//printf("%d\n", maximum[j][i]);
	    		//printf("%d\n", allocation[j][i]);
	    		//need[j][i] = maximum[j][i];
	    		need[j][i] = maximum[j][i] - allocation[j][i];
	    	}
    	}
    	//calc_need();

    } else {
    	printf("%s\n", "Must provide 3 arguments");
    	return 0;
    }
	   
	//Print current state of arrays
    print_max();
    print_need();
    print_alloc();

    //Create threads
    for (int i = 0; i < NUM_CUSTOMERS; i++)
    {
    	int *a = i;
    	pthread_create(&t[i], NULL, &cust_loop, a);
    }

    //Join Threads
    for(int i = 0; i < NUM_CUSTOMERS; i++)
    {
    	pthread_join(t[i], NULL);
    }


    //End and print sequence
    printf("%s\n", "--THE END--");
    printf("Sequence:");
    for (int i = 0; i < NUM_CUSTOMERS; ++i)
    {
    	printf(" %d", sequence[i]);
    }
    printf("\n");
    pthread_mutex_destroy(&lock);
    return EXIT_SUCCESS;
}
