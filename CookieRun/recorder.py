#!/usr/bin/python3

"""
This converts raw evdev events into clicks.
Usage:
adb shell getevent -t /dev/input/event2 > raw.log
cat raw.log | recorder.py > output.log
FIXME: getevent | recorder.py will not work - recorder reads all stdin before printing. ctrl+c before stdin is done causes recorder to exit without printing
"""

import sys
import re

EV_ABS = 0x3
EV_SYN = 0x0

ABS_MT_TRACKING_ID = 0x39
ABS_MT_POSITION_X = 0x35
ABS_MT_POSITION_Y = 0x36
SYN_REPORT = 0x0
SYN_MT_REPORT = 0x2

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


class RawPress:
    def __init__(self):
        self.tracking_id = None
        self.x = None
        self.y = None


class SeqParser:
    def __init__(self):
        self.current_touch = None
        self.current_press = RawPress()

    def parse_sequence(self, events):
        # properties touched by this sequence
        p = self.current_press
        x = None
        y = None
        type_a_commit = False
        type_b_start = False
        type_b_end = False
        commit_time = None
        for ev in events:
            if ev.type == EV_ABS:
                if ev.code == ABS_MT_TRACKING_ID: # possible type B (trackable multi)
                    if ev.value != 0xffffffff:
                        if p.tracking_id is not None and p.tracking_id != ev.value: # type A seems to keep tracking ID constant. incorrect, but kills some noise
                            print(ev.time, "Multitouch detected! (another mt_tracking_id)", file=sys.stderr)
                        type_b_start = True
                        p.tracking_id = ev.value
                    else:
                        type_b_end = True
                        p.tracking_id = None
                elif ev.code == ABS_MT_POSITION_X:
                    p.x = ev.value
                elif ev.code == ABS_MT_POSITION_Y:
                    p.y = ev.value
            elif ev.type == EV_SYN:
                if ev.code == SYN_MT_REPORT:
                    if type_a_commit:
                        print(ev.time, "Type A multitouch - already seen MT_REPORT in sequence", file=sys.stderr)
                    type_a_commit = True
                elif ev.code == SYN_REPORT:
                    commit_time = ev.time

        ret = None
        if p.tracking_id is not None and not type_b_end and not type_b_start:
#            print(commit_time, "Type B slide: no change in tracking id", file=sys.stderr)
            return ret
        # apply changes
        if type_a_commit: # definitely type A
            if p.x is not None and p.y is not None:
                if self.current_touch is None:
                    self.current_touch = Touch([p.x, p.y], commit_time)
            elif p.x is None and p.y is None:
                self.current_touch.end(commit_time)
                ret = self.current_touch
                self.current_touch = None
            else:
                print(commit_time, "Type A only one of x, y updated", file=sys.stderr)
        elif type_b_start: # Type A already handled, must be type B device
            if self.current_touch is not None:
                print(commit_time, "Type B multitouch (two start sequences)", file=sys.stderr)
            else:
                if p.x is None or p.y is None:
                    print(commit_time, "Type B start misses a coord", file=sys.stderr)
                else: 
                    self.current_touch = Touch([p.x, p.y], commit_time)
        elif type_b_end:
            if self.current_touch is None:
                print(commit_time, "Type B Trying to clear touch that wasn't (another mt_tracking_id=-1)", file=sys.stderr)
            else:
                self.current_touch.end(ev.time)
                ret = self.current_touch
                self.current_touch = None
        return ret

start_timestamp = None
touches = []
current_sequence = []
current_touch = None
parser = SeqParser()
for i, line in enumerate(sys.stdin.readlines()):
    ev = InputEvent.from_string(line)
    if (start_timestamp is None):
        start_timestamp = ev.time
    current_sequence.append(ev)
    if ev.type == EV_SYN and ev.code == SYN_REPORT:
        touch = parser.parse_sequence(current_sequence)
        if touch is not None:
            touch.start_time -= start_timestamp
            touch.end_time -= start_timestamp
            touches.append(touch)
        current_sequence = []

    
for touch in touches:
    print("{},{},{:.3f},{:.3f}".format(touch.coords[0], touch.coords[1], touch.start_time, touch.duration()))
