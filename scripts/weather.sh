#!/bin/bash
#	Script: weather.sh
#	Cycles between different weather types defined in Shaders directory

SCRIPT_DIR=~/src/SARndbox-2.3/scripts
SHADER_DIR=~/src/SARndbox-2.3/share/SARndbox-2.3/Shaders

# Possible weather values: Lava Ice Snow Water ToxicWaste ToxicDeath SparklyIce
# 				   PollutedWater
# Add/remove values from weather array to cycle through values in SARndbox
weather=(Lava Ice Snow Water)

# Pick up current state
current=$( cat "$SCRIPT_DIR"/weather_file.tmp )

# Find the index of the current weather in the weather array
i=0
for w in ${weather[@]}; do
	if [ "$w" == "$current" ]; then
		break
	fi
	((i++))
done

# Get the index of the next weather type, wrapping around the array
# If current weather is not in weather array, default to first weather type
((i++))
if [ "$i" -ge "${#weather[@]}" ]; then
	i=0
fi

# Update to new weather state
new="${weather[$i]}"
if [ "$new" != "$current" ]; then
	echo "Changing $current to $new"
	cp "$SHADER_DIR"/SurfaceAddWaterColor-$new.fs "$SHADER_DIR"/SurfaceAddWaterColor.fs
	echo "$new" > "$SCRIPT_DIR"/weather_file.tmp
fi

