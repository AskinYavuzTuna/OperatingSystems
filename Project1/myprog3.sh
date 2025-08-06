#!/bin/bash

if [ ! -d "./writable" ]; then
  mkdir ./writable
fi

moved_file_number=0

for file in *; do
  if [ -f "$file" ] && [ -w "$file" ];
  then
    mv "$file" ./writable/
    moved_file_number=$((moved_file_number + 1))
  fi
done

echo "$moved_file_number files moved to writable directory."
