#!/bin/bash

# Command line arguments:
# -r: restore height color map to original SARndbox height color map
# none: replace height color map with shifted down color map

MAP_DIR=~/src/SARndbox-2.3/etc/SARndbox-2.3
SCRIPT_DIR=~/src/SARndbox-2.3/scripts

if [ "$1" = "-r" ]
then
	# restore original height color map
	cp "$MAP_DIR"/original.cpt "$MAP_DIR"/HeightColorMap.cpt
else
	# get shifted down color map
	"$SCRIPT_DIR"/./change-map-colors.sh - > temp1

	# make sure the number of lines of the files match
	temp_lines=$(wc -l temp1 | awk '{print $1'})
	map_lines=$(wc -l "$MAP_DIR"/HeightColorMap.cpt | awk '{print $1'})
	if [ "$temp_lines" -ne "$map_lines" ]
	then
		# exit if they don't match
		echo "Something went wrong"
		cat temp1
		rm -f temp1
		exit 0
	fi

	# copy it into the HeightColorMap file
	paste <(awk '{print $1}' "$MAP_DIR"/HeightColorMap.cpt) <(cat temp1) > temp2
	cp temp2 "$MAP_DIR"/HeightColorMap.cpt
fi

# clean up
rm -f temp1 temp2
