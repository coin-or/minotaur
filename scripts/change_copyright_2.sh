#!/bin/bash

# Usage: ./update_copyright_year.sh <directory>
# If no directory is specified, defaults to the current directory.

DIR=${1:-../../}
# Find all files recursively
find "$DIR" -type f -name '*' | while read -r FILE; do
  echo "Processing: $FILE"
  sed -i -E 's/(.*\(C\)opyright [0-9]{4} - )[0-9]{4}(.*The*)/\12025\2/' "$FILE"
    sed -i -E 's/(.*\(C\)opyright [0-9]{4} -- )[0-9]{4}(.*The*)/\12025\2/' "$FILE"
    sed -i -E 's/(.*\(C\)opyright [0-9]{4}--)[0-9]{4}(.*The*)/\12025\2/' "$FILE"


  # Replace the copyright range to end with 2025
  sed -i -E 's/(.*\(C\)opyright [0-9]{4})([^0-9]*[0-9]{4})?(.*The*)/\1 - 2025\3/' "$FILE"
   sed -i 's/2025 The MINOTAUR .*/2025 The Minotaur Team./1' "$FILE"
   
done

echo "Done! Updated copyright years to 2025."
