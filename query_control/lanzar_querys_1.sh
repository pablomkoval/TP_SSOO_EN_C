#!/bin/bash

echo "Lanzando 25 AGING_1..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_1 20 &
done
