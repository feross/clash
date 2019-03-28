#!/usr/bin/python
#
# Prints out its argument words (except for the first one, containing the
# program name). Used for testing clash.

import sys

i = 0;
for word in sys.argv:
    if (i != 0):
        print("Word %d: %s" %(i, word))
    i += 1
