#!/bin/bash

clear
cd src || exit 1

if [ $# -gt 0 ]; then
	echo "Recompiling files: $*"
	for arg in "$@"; do
		target="${arg}.o"
		if [ -f "$target" ]; then
			echo "Deleting $target"
			rm -f "$target"
		else
			#Try in src/
			found_files=$(find . -type f -name "$target")
			if [ -n "$found_files" ]; then
				echo "$found_files" | while read -r f; do
					echo "Deleting $f"
					rm -f "$f"
				done
			else
				echo "ERR: $target not found in src/..."
			fi
		fi
	done
else
	echo "Recompiling all files"
	find . -name '*.o' -delete
fi

#Always remove main.o
cd .. || exit 1
rm -f main.o

#gcc make
make

#Success?
if [[ $? -ne 0 ]]; then
	echo "Build failed."
	exit 1
fi



read -n1 -s -r -p "Press any key to run test script..."
echo ""
echo ""

python3 test.py