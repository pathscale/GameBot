#!/usr/bin/python3

"""
Usage:
adb shell getevent -t /dev/input/event2 | recorder.py > output.log
"""

import sys
import re

EV_ABS = 0x3
EV_SYN = 0x0

ABS_MT_TRACKING_ID = 0x39
ABS_MT_POSITION_X = 0x35
ABS_MT_POSITION_Y = 0x36
SYN_REPORT = 0x0

class InputEvent:
    def __init__(self, time, type_, code, value):
        self.time = time
        self.type = type_
        self.code = code
        self.value = value
        
    def __str__(self):
        return '[{}] {:04x} {:04x} {:08x}'.format(self.time, self.type, self.code, self.value)

    @classmethod
    def from_string(cls, s):
        s = s.replace('[', '').replace(']', '').strip() # cut out formatting marks
        time, type_, code, value = s.split()
        return cls(float(time), int(type_, base=16), int(code, base=16), int(value, base=16))
        

class Touch:
    def __init__(self, coords, start_time):
        self.coords = coords
        self.start_time = start_time
    
    def end(self, end_time):
        self.end_time = end_time

    def duration(self):
        return self.end_time - self.start_time

start_timestamp = None
touches = []
current_touch = None
for i, line in enumerate(sys.stdin.readlines()):
    ev = InputEvent.from_string(line)
    if (start_timestamp is None):
        start_timestamp = ev.time
    ev.time -= start_timestamp
    if ev.type == EV_ABS:
        if ev.code == ABS_MT_TRACKING_ID:
            if ev.value != 0xffffffff:
                if current_touch is not None:
                    print("Multitouch detected! (another mt_tracking_id in line {})".format(i), file=sys.stderr)
                current_touch = Touch([None, None], ev.time)
            else:
                if current_touch is None:
                    print("Trying to clear touch that wasn't (another mt_tracking_id=-1 in line {})".format(i), file=sys.stderr)
                current_touch.end(ev.time)
                touches.append(current_touch)
                current_touch = None
        elif ev.code == ABS_MT_POSITION_X and current_touch.coords[0] is None:
            current_touch.coords[0] = ev.value
        elif ev.code == ABS_MT_POSITION_Y and current_touch.coords[1] is None:
            current_touch.coords[1] = ev.value

for touch in touches:
    print("{},{},{:.3},{:.3}".format(touch.coords[0], touch.coords[1], touch.start_time, touch.duration()))
