#!/bin/sh

echo "Executing bucket and ball.."

g++ mirage_bucket_and_ball_with_assert.cc -o mirage_with_assert.o
rm -f mirage_bucket_ball_data.txt

echo "Compilation successful. Running the code.."
times=6

for i in $(seq 0 $times); do
	echo "-------------------------------------------------------------"
	echo "For extra skew $i"
	./mirage_with_assert.o $i
	sleep 1
done


python3 plotter.py

