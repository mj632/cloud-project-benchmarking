#!/bin/bash

name=$(echo $1 | cut -d '-' -f1 -)
node1=$(echo $1 | cut -d '-' -f2 - | tr -d '[')
node2=$(echo $1 | cut -d '-' -f3 - | tr -d ']')

if [ "$(hostname)" == "$name-$node1" ]
then
	./MyNETBench-UDP server network-UDP-1-1thread.dat
else
	sleep 10
	./MyNETBench-UDP client network-UDP-1-1thread.dat $name-$node1
fi

