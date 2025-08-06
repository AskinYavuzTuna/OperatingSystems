#!/bin/bash

input1_file="giris.txt"
input2_file="gelisme.txt"
input3_file="sonuc.txt"
output_file="$1"

if [ $# -ne 1 ]; then
    echo "Please give an input."
    exit 1
fi

if [ ! -e "giris.txt" ]; then
    echo "giris.txt doesn't exist"
    exit 1
fi

if [ ! -e "gelisme.txt" ]; then
    echo "gelisme.txt doesn't exist"
    exit 1
fi

if [ ! -e "sonuc.txt" ]; then
    echo "sonuc.txt doesn't exist"
    exit 1
fi

if [ -f "$output_file" ]; then
    echo "$output_file exists. Do you want it to be modified? (y/n): "
    read answer
    if [ "$answer" != "y" ]; then
        echo "Exiting without modifying the file."
        exit 0
    fi
fi

random_line=$(((RANDOM % 3) * 2 + 1))
line_num=1
while IFS= read -r line
do

    if [ "$random_line" -eq $line_num ]; then
        echo "$line" > "$output_file"
        echo  >> "$output_file"
        break
    else
        ((line_num++))
    fi

done < "$input1_file"

random_line=$(((RANDOM % 3) * 2 + 1))
line_num=1
while IFS= read -r line
do

    if [ "$random_line" -eq $line_num ]; then
        echo "$line" >> "$output_file"
        echo  >> "$output_file"
        break
    else
        ((line_num++))
    fi

done < "$input2_file"

random_line=$(((RANDOM % 3) * 2 + 1))
line_num=1
while IFS= read -r line
do

    if [ "$random_line" -eq $line_num ]; then
        echo "$line" >> "$output_file"
        echo  >> "$output_file"
        break
    else
        ((line_num++))
    fi

done < "$input3_file"

echo "A random story is created and stored in $output_file."