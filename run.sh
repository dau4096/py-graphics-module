#!/bin/bash

clear
rm gl*.so


#Compile
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target clean
cmake --build build


#Success?
if [[ $? -ne 0 ]]; then
	echo "Build failed."
	exit 1
fi


#Move .SO (module) file to the main dir.
mv build/gl*.so ./


read -n1 -s -r -p "Press any key to run test script..."
echo ""
echo ""

python3 test.py