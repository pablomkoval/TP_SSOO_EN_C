#!/bin/bash

echo "Lanzando 25 AGING_2..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_2 20 &
done
