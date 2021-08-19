import time
import sys

with open(sys.argv[1], 'w') as f:
    f.write('#define BUILD_NUMBER %s' % hex(int(time.time())))
