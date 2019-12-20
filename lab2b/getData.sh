#!/bin/bash

for i in 10 20 40 80
do
    for t in 1 4 8 12 16
    do
	./lab2_list --threads=$t --iterations=$i --lists=4 --yield=id --sync=s
    done
done
