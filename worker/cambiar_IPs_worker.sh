#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Uso: $0 <nueva_ip_master> <nueva_ip_storage>"
    exit 1
fi

IP_M="$1"
IP_S="$2"

for file in *.config; do
    [ -e "$file" ] || continue  # si no hay .config, evita error del for

    # Reemplaza IP_MASTER
    sed -i -E "s/^([[:space:]]*IP_MASTER[[:space:]]*=[[:space:]]*).*/\1${IP_M}/" "$file"

    # Reemplaza IP_STORAGE
    sed -i -E "s/^([[:space:]]*IP_STORAGE[[:space:]]*=[[:space:]]*).*/\1${IP_S}/" "$file"

    echo "Actualizado: $file"
done