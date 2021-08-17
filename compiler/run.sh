#!/bin/bash

clear

# echo "Enter the Directory Path (i.e examples/firstOccurence): "
# read directory
# echo $directory

echo "Choose one of the following to be the Directory Path (enter the number): "
echo "1. firstOccurence"
echo "2. multipleSemErrors"
echo "3. semCorrect/other"
echo "4. semCorrect/types"
echo "5. typeMismatch"
echo "6. warnings"
echo ""
read opt
echo ""

if [ $opt -eq 1 ]; then
  directory="examples/firstOccurence"
elif [ $opt -eq 2 ]; then
  directory="examples/multipleSemErrors"
elif [ $opt -eq 3 ]; then
  directory="examples/semCorrect/other"
elif [ $opt -eq 4 ]; then
  directory="examples/semCorrect/types"
elif [ $opt -eq 5 ]; then
  directory="examples/typeMismatch"
elif [ $opt -eq 6 ]; then
  directory="examples/warnings"
fi


echo "Do you want to use diff.py? [true/false]"
read diff

echo "Where is your git repo folder located? "
read folder

for file in $directory/*.lla; do
  cd src && make && ./llama < "../"$file > "../"$file.txt && cd ..
  if "$diff" == true; then
    python3 $HOME/$folder/compilers-NTUA/compiler/diff.py $file $HOME $folder
  fi
done