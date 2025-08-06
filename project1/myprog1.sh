#!/usr/bin/env bash
if [ $# -ne 2 ]; then
    echo "Please give two arguments as inputs."
    exit 1
fi
string="$1"
char_num=${#string}
digit="$2"
digit_num=${#digit}
if [ "$char_num" -eq "$digit_num" ]; 
then 
    changed=""
    for ((i=0;i<"$char_num";i++)) do
        sh_char="${string:$i:1}"
        sh_digit="${digit:$i:1}"

    if [[ ! "$sh_char" =~ [a-zA-Z] ]]; then
        echo "first input should only contain letters"
        exit 1
    elif [[ ! "$sh_digit" =~ ^[0-9]$ ]]; then
        echo "2. string should consist of number."
        exit 1
    elif [[  "$sh_char" =~ ^[a-z]$ ]]; then
        ascii_value=$(printf "%d" "'$sh_char")
        ascii_value=$((((ascii_value + "$sh_digit"-97)%26)+97))
    elif [[  "$sh_char" =~ ^[A-Z]$ ]]; then
        ascii_value=$(printf "%d" "'$sh_char")
        ascii_value=$((((ascii_value + "$sh_digit"-65)%26)+65))
    fi

    sh_char=$(printf "\\$(printf '%03o' "$ascii_value")")
    changed+=$sh_char
    done
    echo "$changed"
elif [ "$digit_num" -eq 1 ];
then
    changed=""
    for ((i=0;i<"$char_num";i++)) do
        sh_char="${string:$i:1}"


    if [[ ! "$sh_char" =~ [a-zA-Z] ]]; then
    echo "first input should only contain letters"
    exit 1
    elif [[ ! "$digit" =~ ^[0-9]$ ]]; then
        echo "2. string should consist of number."
        exit 1
    elif [[  "$sh_char" =~ ^[a-z]$ ]]; then
        ascii_value=$(printf "%d" "'$sh_char")
        ascii_value=$((((ascii_value + "$digit"-97)%26)+97))
    elif [[  "$sh_char" =~ ^[A-Z]$ ]]; then
        ascii_value=$(printf "%d" "'$sh_char")
        ascii_value=$((((ascii_value + "$digit"-65)%26)+65))
    fi



    char=$(printf "\\$(printf '%03o' "$ascii_value")")
    changed+=$char
    done
    echo "$changed"

else
    echo "The number of characters in the string and the number of digits don't match conditionally."
    exit 1
fi