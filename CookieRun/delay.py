#!/usr/bin/python3
import sys

if len(sys.argv) < 3:
    print("Usage: ./delay.py file.log +0.5")

delay_str = sys.argv[2]
delay = 1
if delay_str[0] == '-':
    delay = -1
    delay_str = delay_str[1:]
if delay_str[0] == '+':
    delay_str = delay_str[1:]

delay *= float(delay_str)

new_lines = []

with open(sys.argv[1]) as infile:
    for line in infile.readlines():
        sx, sy, st, sd = line.strip().split(';', 1)[0].split(',')
        t = float(st) + delay
        st = '{:.3f}'.format(t)
        new_lines.append(','.join([sx, sy, st, sd]))

with open(sys.argv[1], 'w') as outfile:
    for line in new_lines:
        print(line, file=outfile)
