#!/bin/sh

# Pass amount of launches as first argument and amount of points as second

for i in 1 2 3 4 5 6 7 8 9 10 16

do
    python3 scripts/script.py "$1" $i "$2"
done