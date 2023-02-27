// Original Code by Anirban Chakraborty et al. [HPCA'23]
// Gururaj Saileshwar - Added Assert

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "mtrand.h"

int EXTRA_BUCKET_CAPACITY = 0;
int NUM_TRIALS = 1 * 1000 * 1000;

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

MTRand *mtrand=new MTRand();

// GS: Count number of balls in buckets
uint64_t balls_in_buckets()   {
	uns64 ii;
	uns64 balls_in_buckets = 0; 
	for (ii = 0; ii < NUM_BUCKETS; ii++)
	  balls_in_buckets += buckets[ii] ;
	return balls_in_buckets;
}

// GS: Add assert
#define ASSERT_WITH_MESSAGE(condition, message)\
   (!(condition)) ?\
      (std::cerr << "\033[0;31mAssertion failed: \033[0m(" << #condition << "), "\
      << "function " << __FUNCTION__\
      << ", file " << __FILE__\
      << ", line " << __LINE__ << "."\
      << std::endl << message << std::endl, abort(), 0) : 1



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
		return 0;
	}
	
	return 0;

}

void init_buckets()	{
	uns64 ii;
	assert(NUM_SKEWS * NUM_BUCKETS_PER_SKEW == NUM_BUCKETS);

	for (ii = 0; ii < NUM_BUCKETS; ii++)
		buckets[ii] = 0;
}


int main(int argc, char* argv[])	{

	init_buckets();

	assert(argc == 2);
	EXTRA_BUCKET_CAPACITY = atoi(argv[1]);
	SPILL_THRESHOLD = BASE_WAYS_PER_SKEW + EXTRA_BUCKET_CAPACITY;

	FILE *fp = fopen("mirage_bucket_ball_data.txt", "a");

	printf("%d, %d\n", BASE_WAYS_PER_SKEW, EXTRA_BUCKET_CAPACITY);

	printf("Cache configurations: %d MB, %d skews, %d ways (%d ways/skew)\n", CACHE_SZ_BYTES/1024/1024,NUM_SKEWS,NUM_SKEWS*BASE_WAYS_PER_SKEW,BASE_WAYS_PER_SKEW);
	printf("Number of buckets: %d\n", NUM_BUCKETS);

	for (uns64 i = 0; i < NUM_TRIALS; i++)	{
		if (insert_ball(i) != 0)	{
		  uint64_t cache_capacity = CACHE_SZ_BYTES/LINE_SZ_BYTES;

// #ifndef NO_ASSERT_CAPACITY
// 		  // GS: Sanity Check
// 		  if( balls_in_buckets() > cache_capacity)
// 		    printf("Balls in Buckets: %llu. Cache Capacity in Lines: %llu \n", balls_in_buckets(), cache_capacity);
// #endif
		  printf("Found collision after %lld trials\n", i);
// #ifndef NO_ASSERT_CAPACITY
// 		  // GS: Sanity Check
// 		  assert(balls_in_buckets() <= cache_capacity && "Lines in Cache Exceeds Cache Capacity\n");
//#endif
		  fprintf(fp, "%lld\n", i);
		  // GS: Sanity check
		  ASSERT_WITH_MESSAGE(balls_in_buckets() <= CACHE_CAPACITY_IN_LINES, "Cache Capacity Exceeded - Balls in Buckets : "<< balls_in_buckets() << ", Cache Capacity (lines) : " << CACHE_CAPACITY_IN_LINES << " at trial : "<<i); 

		  break;
		}
	}
	
	return 0;
}
