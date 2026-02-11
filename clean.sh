#!/bin/bash


AUTO_CONFIRM=false
for arg in "$@"; do
    if [ "$arg" == "-y" ]; then
        AUTO_CONFIRM=true
        break
    fi
done


echo "Removing *.o, *.so and *.pyd files..."

#Find all relevant files
mapfile -t files < <(find . -type f \( -name "*.o" -o -name "*.so" -o -name "*.pyd" \))


#len(files) = 0
if [ ${#files[@]} -eq 0 ]; then
    echo "No matching files found."
    exit 0
fi


echo "The following files were found:"
for f in "${files[@]}"; do
    echo " - $f"
done


#If there was no -y flag then ask before deleting.
if [ "$AUTO_CONFIRM" = false ]; then
    read -p "Do you want to delete these files? [y/N]: " response
    case "$response" in
        [yY][eE][sS]|[yY]) 
            ;;
        *)
            echo "Aborted."
            exit 1
            ;;
    esac
fi


#rm the files.
for f in "${files[@]}"; do
    echo " = Deleting $f"
    rm -f "$f"
done


echo "Done."