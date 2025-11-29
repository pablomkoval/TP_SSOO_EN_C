#!/bin/bash

echo "Lanzando 25 AGING_3..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_3 20 &
done
