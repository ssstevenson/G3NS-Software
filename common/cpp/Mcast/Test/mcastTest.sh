#!/bin/sh

# Default nbBoosters=2 if no argument
rm -f test.txt
killall mcast
if [ $# -eq 0 ]; then
    nbBoosters=2
else
    nbBoosters=$1
fi

echo "Number of boosters = $nbBoosters"

# Loop from 1 to nbBoosters and start the MCAST application 
i=1
while [ $i -le $nbBoosters ]; do
    echo "Starting MCAST for Booster $i"
    ./mcast booster 0 &> test$i.txt  &
    sleep 1
    i=$((i+1))
done

sleep 2

# Start The Controller Side and send commands
./mcast SOS_CONTROLLER $nbBoosters 5
sleep 1



