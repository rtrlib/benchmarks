#!/usr/bin/python

# Written by Nils Bars
# Programming language: python3
# using matplotlib 1.4.0-2


import matplotlib.pyplot as plt
import os
import sys

# print 'Number of arguments:', len(sys.argv), 'arguments.'
# print 'Argument List:', str(sys.argv)

#infile = open('entrada', 'r')

def print_usage():
	print("Usage")
	print("%s [<name> <file>] <output-file>\n" % (sys.argv[0]))
	print("If you provide more then one file, the x and y axis names will be taken from the first file!")
	print("The name argument is the name which will be used for the data set in the graph legend. (Use '' for no name)")
	print("The data format will be determined by the file extension.")
	print("	Supported formats: eps, pdf, pgf, png, ps, raw, rgba, svg, svgz.\n")

	print("The data in the files must be in the following format:")
	print("x-axis-name;y-axis-name")
	print("x-value-1;y-value-1")
	print("x-value-2;y-value-2")
	print("...................")
	print("x-value-n;y-value-n\n")

	print("Example")
	print("%s measurement1 measurement1.log measurement2 measurement2.log\n" % (sys.argv[0]))
	exit()


if len(sys.argv) < 4 or len(sys.argv) % 2 != 0 or sys.argv[1] == '--help' or sys.argv[1] == '-h':
	print_usage()

#Get axis captions
infile = open(sys.argv[2], 'r')
line = infile.readline()
line_components = line.split(";")
plt.xlabel(line_components[0])
print(line_components[1])
plt.ylabel(line_components[1])


for x in range(2,len(sys.argv),2):
	infile = open(sys.argv[x], 'r')
	lines = infile.readlines()

	data_set_x = [];
	data_set_y = [];
	for line in lines[1:]:
		line_components = line.strip().split(";")
		data_set_x.append(float(line_components[0]))
		data_set_y.append(float(line_components[1]))
	plt.plot(data_set_x, data_set_y, label=sys.argv[x-1])

plt.grid()
plt.legend()

#Save to file
plt.savefig(sys.argv[len(sys.argv)-1], dpi=1000)

plt.show()
