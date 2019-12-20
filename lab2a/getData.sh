#!/bin/bash
#add-none
for i in 100 1000 10000 100000
do
    for t in 2 4 8 12 
    do
	./lab2_add --iterations=$i --threads=$t >> lab2_add.csv
    done
done
#add-yield
for i in 10 20 40 80 100 1000 10000 100000
do
    for t in 2 4 8 12
    do
	./lab2_add --yield --iterations=$i --threads=$t >> lab2_add.csv
    done
done
#add-none 1 thread
for i in 100 1000 10000 100000 1000000
do
    ./lab2_add --iterations=$i >> lab2_add.csv
done

#test for correctness in each type of lock 
for c in m s c
do
    for t in 2 4 8 12
    do
	./lab2_add --iterations=10000 --threads=$t --sync=$c --yield >> lab2_add.csv
    done
done

for s in n m s c
do
    for t in 1 2 4 8 12
    do
	./lab2_add --iterations=10000, --threads=$t --sync=$s >> lab2_add.csv
    done
done
