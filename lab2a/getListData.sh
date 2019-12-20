#!/bin/bash

for i in 10 100 1000 10000 20000
do
    ./lab2_list --threads=1 --iterations=$i >> lab2_list.csv
done

for t in 2 4 8 12
do
    for i in 1 10 100 1000
    do
	./lab2_list --threads=$t --iterations=$i >> lab2_list.csv
    done
done

for t in 2 4 8 12
do
    for i in 1 2 4 8 16 32
    do
	./lab2_list --threads=$t --iterations=$i --yield=i >> lab2_list.csv
    done
done

for t in 2 4 8 12
do
    for i in 1 2 4 8 16 32
    do
	./lab2_list --threads=$t --iterations=$i --yield=d >> lab2_list.csv
    done
done

for t in 2 4 8 12
do
    for i in 1 2 4 8 16 32
    do
	./lab2_list --threads=$t --iterations=$i --yield=il >> lab2_list.csv
    done
done

for t in 2 4 8 12
do
    for i in 1 2 4 8 16 32
    do
	./lab2_list --threads=$t --iterations=$i --yield=dl >> lab2_list.csv
    done
done


for s in m s
do
    ./lab2_list --threads=12 --iterations=32 --sync=$s >> lab2_list.csv
done

for s in m s
do
    for y in i d l id dl il
    do
    ./lab2_list --threads=12 --iterations=32 --sync=$s --yield=$y >> lab2_list.csv
    done
done


for t in 1 2 4 8 12 16 24
do
    for s in n m s
    do
	./lab2_list --sync=$s --iterations=1000 --threads=$t >> lab2_list.csv
    done
done
