#!/usr/bin/python3

# Written by Nils Bars
# Programming language: python3
# using matplotlib 1.4.0-2
import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import numpy as np
import sys


def print_usage():
    print("Usage")
    print("%s <chart-title> <source-file> <output-file>\n" % (sys.argv[0]))
    print("The output data format will be determined by the file extension.")
    print("Supported formats: eps, pdf, pgf, png, ps, raw, rgba, svg, svgz.\n")

    print("The data in the source files must be in the following format:")

    print("Example")
    print("%s my-chart measurement1.log result.pdf\n" % (sys.argv[0]))
    exit()

# Parse arguments
if len(sys.argv) != 4 or sys.argv[1] == '--help' or sys.argv[1] == '-h':
    print_usage()

fig = plt.figure()
ax = fig.add_subplot(111)
ax.set_title(sys.argv[1])

# Get axis captions
infile = open(sys.argv[2], 'r')
captions = infile.readline()
captionsList = captions.split(";")
plt.xlabel(captionsList[0])
plt.ylabel(captionsList[1])

# Group names
groupNames = infile.readline().strip().split(";")
groupCount = len(groupNames)

# Bar names
barNames = infile.readline().strip().split(";")
barCount = len(barNames)

# Group data
groupData = []

line = infile.readline()
while line != '':
    barValues = line.strip().split(";")
    barValues = [float(x) for x in barValues]
    groupData.append(barValues)
    line = infile.readline()

ind = np.arange(groupCount)      # the x locations for the groups
width = 0.1                    # the width of the bars

# Draw the bars
rects = []
colors = ["b", "g", "r", "c", "m", "y", "k", "w"]
for i in range(0, len(groupData)):
    rect = ax.bar(
        ind + (width * i), groupData[i], width, color=colors[i % len(colors)])
    rects.append(rect)

# Draw the x-axis group captions
ax.set_xticks(ind + (width * barCount / 2))
xtickNames = ax.set_xticklabels(groupNames)

# add a legend
ax.legend(rects, barNames, loc=2, fontsize="x-small")

ax.grid()
plt.margins(0.1)
plt.savefig(sys.argv[len(sys.argv) - 1], dpi=1000, figsize=(2, 2))
# plt.show()
