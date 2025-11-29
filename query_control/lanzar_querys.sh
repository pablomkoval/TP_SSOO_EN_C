#!/bin/bash

echo "Lanzando 25 AGING_1..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_1 20 &
done

echo "Lanzando 25 AGING_2..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_2 20 &
done

echo "Lanzando 25 AGING_3..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_3 20 &
done

echo "Lanzando 25 AGING_4..."
for i in {1..25}; do
    ./bin/query_control query.config AGING_4 20 &
done