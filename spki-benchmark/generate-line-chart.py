#!/usr/bin/python3

# Written by Nils Bars
# Programming language: python3
# using matplotlib 1.4.0-2
import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import sys


def print_usage():
    print("Usage")
    print("%s <chart-title> <source-file> <output-file>\n" % (sys.argv[0]))
    print("The output data format will be determined by the file extension.")
    print("Supported formats: eps, pdf, pgf, png, ps, raw, rgba, svg, svgz.\n")

    print("The data in the source files must be in the following format:")
    print("x-axis-name;y-axis-name")

    print("0.Data set title")
    print("x0-value-1;y0-value-1")
    print("x0-value-2;y0-value-2")
    print("...................")
    print("x0-value-n;y0-value-n")

    print("\\n")
    print("1.Data set title")
    print("x1-value-1;y1-value-1")
    print("x1-value-2;y1-value-2")
    print("...................")
    print("x1-value-n;y1-value-n\n")

    print("Example")
    print("%s my-chart measurement1.log result.pdf\n" % (sys.argv[0]))
    exit()

# Parse arguments
if len(sys.argv) != 4 or sys.argv[1] == '--help' or sys.argv[1] == '-h':
    print_usage()

plt.title(sys.argv[1])

# Get axis captions
infile = open(sys.argv[2], 'r')
line = infile.readline()
line_components = line.split(";")
plt.xlabel(line_components[0])
plt.ylabel(line_components[1])

set_name = line = infile.readline()
minxval = None
while line != "":
    setx = []
    sety = []
    while True:
        line = infile.readline()
        if line == '\n' or line == "":
            break
        line_components = line.strip().split(";")
        setx.append(float(line_components[0]))
        sety.append(float(line_components[1]))
        if minxval is None or minxval > float(setx[0]):
            minxval = float(setx[0])

    plt.plot(setx, sety, label=set_name)
    set_name = line = infile.readline()
    plt.xlim(minxval)

    plt.plot(setx, sety, label=set_name)
    set_name = line = infile.readline()

plt.margins(0.1)
plt.grid()
plt.legend()
plt.savefig(sys.argv[len(sys.argv) - 1], dpi=1000, figsize=(2, 2))
