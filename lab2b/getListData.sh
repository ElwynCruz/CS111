#!/bin/bash

rm -f lab2b_list.csv

for s in m s 
do
    for t in 1 2 4 8 12 16 24 
    do
    ./lab2_list --threads=$t --iterations=1000 --sync=$s >> lab2b_list.csv
    done
done


for s in m s
do
    for t in 1 4 8 12 16
    do
	for i in 10 20 40 80
	do
	    ./lab2_list --yield=id --lists=4 --threads=$t --iterations=$i --sync=$s >> lab2b_list.csv
	done
    done
done

for s in m s
do
    for l in 4 8 16
    do
	for t in 1 4 8 12
	do
	    ./lab2_list --lists=$l --threads=$t --iterations=1000 --sync=$s >> lab2b_list.csv
	done
    done
done

for t in 1 4 8 12 16
do
    for i in 1 2 4 8 16
    do
	./lab2_list --yield=id --lists=4 --threads=$t --iterations=$i >> lab2b_list.csv
    done
done
