#! /bin/bash

ARGS=2

if [ $# -ne "$ARGS" ]
    then
    echo "Error: Two arguments are needed"
    exit 1
fi

COUNTER=0;

while [ $COUNTER -lt $2 ]
do
    (( COUNTER++ ))
    echo "Execution n°:" $COUNTER
    $1
done

exit 0

