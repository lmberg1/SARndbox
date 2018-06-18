#!/usr/bin/python

# Prints a list of rgb colors either shifted up or down from the current values
# in HeightColorMap.cpt
#
# Command line arguments:
# '+': shifts colors up
# '-': shifts colors down
# none: sets colors to default

import sys

# update defaults if necessary
LINES_IN_HEIGHT_COLOR_MAP = 15
DEFAULT_LINE = 5
SCRIPT_DIR = "/home/sandpit/src/SARndbox-2.3/scripts/"

# get the current start line
with open(SCRIPT_DIR + "color.tmp") as tmp:
	currentLine = int(tmp.readline())
	
	# open file that holds rgb values
	with open(SCRIPT_DIR + "colors.txt") as fid:
		# get number of lines in the file
		num_lines = sum(1 for line in fid)

		# get new line number
		if len(sys.argv) == 2:
			if sys.argv[1] == "-":
				newLine = max(0, currentLine - 1)
			elif sys.argv[1] == "+":
				newLine = max(0, currentLine + 1)
			else:
				print "Invalid command line argument:", sys.argv[1], 
				"\nUsage: ./change_map_colors ([+] | [-])"
				sys.exit(0)
		else:
			newLine = DEFAULT_LINE
		newLine = min(num_lines - LINES_IN_HEIGHT_COLOR_MAP, newLine)	
		
		# go back to start of file	
		fid.seek(0)
		
		# skip to newLine
		for i in range(0, newLine):
			line = fid.readline()

		# print rgb values starting at newLine
		line = fid.readline()
		for i in range(0, LINES_IN_HEIGHT_COLOR_MAP):
			col = line.split()
			r = float(col[0])
			g = float(col[1])
			b = float(col[2])
			print "%d\t%d\t%d" % (r, g, b)
			line = fid.readline();

		# write new line number
		with open(SCRIPT_DIR + "color.tmp", "w+") as tmp:
			tmp.write(str(newLine))
