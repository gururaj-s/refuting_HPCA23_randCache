#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "mtrand.h"

int EXTRA_BUCKET_CAPACITY = 6;
int NUM_BILLION_TRIES = 1 * 1000 * 1000 * 1000;


unsigned int seed = 1;

#define BASE_WAYS_PER_SKEW        (8)

#define NUM_SKEWS                 (2)

//16MB LLC
#define CACHE_SZ_BYTES            (16*1024*1024) 
#define LINE_SZ_BYTES             (64)
#define NUM_BUCKETS               ((CACHE_SZ_BYTES/LINE_SZ_BYTES)/(BASE_WAYS_PER_SKEW))
#define NUM_BUCKETS_PER_SKEW      (NUM_BUCKETS/NUM_SKEWS)
#define CACHE_CAPACITY_IN_LINES   (CACHE_SZ_BYTES/LINE_SZ_BYTES)

//Bucket Capacities
#define BALLS_PER_BUCKET      (BASE_WAYS_PER_SKEW)
#define MAX_FILL              (16)

#define MAXN					  NUM_BUCKETS_PER_SKEW

int SPILL_THRESHOLD = BALLS_PER_BUCKET + EXTRA_BUCKET_CAPACITY;

typedef unsigned int uns;
typedef unsigned long long uns64;
typedef double dbl;

uns64 buckets[NUM_BUCKETS];
uns64 balls[NUM_BUCKETS * BALLS_PER_BUCKET];

MTRand *mtrand=new MTRand();

uint64_t balls_in_buckets()   {
	uns64 ii;
	uns64 balls_in_buckets = 0; 
	for (ii = 0; ii < NUM_BUCKETS; ii++)
	  balls_in_buckets += buckets[ii] ;
	return balls_in_buckets;
}

#define ASSERT_WITH_MESSAGE(condition, message)\
   (!(condition)) ?\
      (std::cerr << "Assertion failed: (" << #condition << "), "\
      << "function " << __FUNCTION__\
      << ", file " << __FILE__\
      << ", line " << __LINE__ << "."\
      << std::endl << message << std::endl, abort(), 0) : 1

int hash(int function, uns64 key)	{
    switch (function)	{
        case 1: return key%MAXN;
        case 2: return (key/MAXN)%MAXN;
        default: return 0;
    }
}


int insert_ball(uns64 ballID)	{

	uns64 index1 = mtrand->randInt(NUM_BUCKETS_PER_SKEW  - 1);
	uns64 index2 = NUM_BUCKETS_PER_SKEW + mtrand->randInt(NUM_BUCKETS_PER_SKEW - 1);

	uns64 index;
	uns retval;

	if (buckets[index2] < buckets[index1])	
		index = index2;
	else if (buckets[index1] < buckets[index2])
		index = index1;
	else	{
		if (mtrand->randInt(1) == 0)	
			index = index1;
		else
			index = index2;
	}

	retval = buckets[index];
	if (retval >= SPILL_THRESHOLD)	{
	 	return -1;
	}
	else	{
		buckets[index]++;
		// GS: Fixed a bug in array indexing, to make sure balls get replaced in entire data store
		balls[ballID % (NUM_BUCKETS * BALLS_PER_BUCKET)] = index;
		return 0;
	}

	return 0;

}

uns remove_ball(void){
  uns64 ballID = mtrand->randInt(NUM_BUCKETS*BALLS_PER_BUCKET -1);
  if (balls[ballID] != (uns64)-1)	{
  	uns64 bucket_index = balls[ballID];
  	assert(buckets[bucket_index] != 0 ); 
  	buckets[bucket_index]--;
  	balls[ballID] = -1;
  }
  return ballID;
}

void init_buckets()	{
	uns64 ii;
	assert(NUM_SKEWS * NUM_BUCKETS_PER_SKEW == NUM_BUCKETS);

	for (ii = 0; ii < NUM_BUCKETS; ii++)
		buckets[ii] = 0;
	for (ii = 0; ii < NUM_BUCKETS * BALLS_PER_BUCKET; ii++)
		balls[ii] = -1;
}

int main(int argc, char *argv[])	{
  	//Get params
	if(argc != 2)
	  EXTRA_BUCKET_CAPACITY = 6;
	else {
	  EXTRA_BUCKET_CAPACITY = atoi(argv[1]);
	  SPILL_THRESHOLD = BALLS_PER_BUCKET + EXTRA_BUCKET_CAPACITY;
	}

	//Output the results
	FILE *fp = fopen("mirage_bucket_ball_data.txt", "a");
	
	init_buckets();
	bool found_collision = false;
	
	for (uns64 i = 0; i < NUM_BILLION_TRIES; i++)	{
		uns res = remove_ball();
		// GS: Fixed a bug to make sure inserted ball goes in place of the removed ball.
		if (insert_ball(res) != 0)	{
			printf("Found collision after %lld trials\n", i);
			fprintf(fp, "%lld\n", i);
			
			//GS: Sanity Check
			ASSERT_WITH_MESSAGE(balls_in_buckets() <= CACHE_CAPACITY_IN_LINES, "Cache Capacity Exceeded - Balls in Buckets : "<< balls_in_buckets() << ", Cache Capacity (lines) : " << CACHE_CAPACITY_IN_LINES << " at trial : "<<i); 
			found_collision = true;
			break;
		}
	}
	
	printf("End of experiment with  %llu ball throws\n", NUM_BILLION_TRIES);
	if(found_collision == false)
	  fprintf(fp, "%d\n", 0);
	
	return 0;

}
