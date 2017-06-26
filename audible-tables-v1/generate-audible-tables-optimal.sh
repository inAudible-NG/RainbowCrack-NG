#!/bin/bash

# http://www.tobtu.com/rtcalc.php#params
# keyspace is 256^4 (length is always 4)
# 99.95% (Total success rate)

./rtgen audible byte 4 4 0 10000 1362345 0 &
./rtgen audible byte 4 4 1 10000 1362345 0 &
./rtgen audible byte 4 4 2 10000 1362345 0 &
./rtgen audible byte 4 4 3 10000 1362345 0 &

# ./rtsort *.rt
