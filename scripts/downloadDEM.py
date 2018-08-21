#!/usr/bin/python2.7
# ./downloadDem.py [-h] -c LAT LON (-w WIDTH | -v HEIGHT) [filename]

import sys
import elevation
import argparse
import os
from subprocess import call

# Parse command line arguments
parser = argparse.ArgumentParser(description='Download DEM from central latitude and longitude.')
parser.add_argument('-c', '--center', nargs=2, type=float, metavar=('LAT', 'LON'),  
                    help='central coordinate of DEM in degrees', required=True)
dimension = parser.add_mutually_exclusive_group(required=True)
dimension.add_argument('-w', '--width', type=float, metavar='WIDTH',  
                    help='width of DEM in degrees; makes width the longer dimension')
dimension.add_argument('-v', '--height', type=float, metavar='HEIGHT',  
                    help='height of DEM in degrees; makes height the longer dimension')
parser.add_argument('filename', nargs='?', default='out.tif',
			  help='output .tif filename')
args = parser.parse_args()

# Sandbox dimensions in pixels
WIDTH=640.0
HEIGHT=480.0

# Get width and height of DEM box in degrees
if (args.width):
	width = args.width
	height = HEIGHT/WIDTH * width
	dimensions=str(WIDTH) + 'x' + str(HEIGHT)
elif (args.height):
	height = args.height
	width = HEIGHT/WIDTH * height
	dimensions=str(HEIGHT) + 'x' + str(WIDTH)

# Get coordinates of bottom left and top right bounding box corners of the DEM
lat=args.center[0]
lon=args.center[1]
lon_min = lon - width/2.0;
lon_max = lon + width/2.0;
lat_min = lat - height/2.0;
lat_max = lat + height/2.0;

# Download DEM of given bounds
print "Downloading DEM with bounds: [(%.2f, %.2f), (%.2f, %.2f)]" % (lat_min, lon_min, lat_max, lon_max)
elevation.clip(product='SRTM3', bounds=(lon_min, lat_min, lon_max, lat_max), output='out.tif')
elevation.clean();

# Get path of recently downloaded DEM
path=str(elevation.CACHE_DIR) + '/SRTM3/out.tif'

# Convert the DEM to the correct dimensions
call(['convert', path, '-resize', dimensions, args.filename])
