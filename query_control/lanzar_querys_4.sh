#!/bin/bash

echo "Lanzando 25 AGING_4..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_4 20 &
done
