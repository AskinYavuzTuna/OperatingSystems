#!/bin/bash

if [ -z "$1" ]; then
  echo "Please write a wildcard."
  exit 1
fi

wildcard="$1"

if [ -n "$2" ]; then
  directory="$2"
else
  directory="." 
fi
echo "$directory"

files=$(find "$directory" -type f -name "$wildcard")

if [ -z "$files" ]; then
  echo "No files matching the wildcard '$wildcard' found in directory '$directory'."
  exit 0
fi

for file in $files; do
  read -p "Do you want to delete '$file'? (y/n): " answer

  if [[ "$answer" =~ ^[Yy] ]]; then
    rm "$file"
    echo "'$file' deleted."
  elif [[ "$answer" =~ ^[Nn] ]]; then
    echo "'$file' not deleted."
  else
    echo "Invalid input. Skipping '$file'."
fi

done

