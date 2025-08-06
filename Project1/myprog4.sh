#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Please give an input."
    exit 1
fi

number="$1"
# Check the input is a positive number
if  [[ "$number" =~ ^[0-9]+$ ]]
then
# Check all the numbers whether prime or not
for ((i=2; i<number; i++)); do
  is_prime=1 

    for ((j=2; j*j<=i; j++)); do
    if ((i % j == 0)); then
      is_prime=0 
      break
    fi
done
if ((is_prime == 1)); then
    num=$i
    hex_value=""
hex_chars="0123456789ABCDEF"
    while [ "$num" -gt 0 ]; do
      remainder=$(( num % 16 ))
      hex_digit="${hex_chars:$remainder:1}"
      hex_value="$hex_digit$hex_value"
      num=$(( num / 16 ))
    done

    echo "Hexadecimal of $i is $hex_value"
  fi
done
else
echo "Please give a positive number!"
exit 1
fi