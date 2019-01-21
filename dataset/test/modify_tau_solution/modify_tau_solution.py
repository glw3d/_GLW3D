import os
import sys
sys.path.append("../../bin/")
from param_file import *

pf = lpf_open("NACA0012.param", ":", "block end", "")
primgrid = lpf_get_value_n(pf, "Primary grid filename", 0, 0)
print("Primary grid filename: ", primgrid)
deformed_grid = lpf_get_value(pf, "Primary grid filename", 0)
print("Modified grid filename: ", deformed_grid)
ngroups = lpf_quote_groups(pf)
print("--- Boundaries --- ")
for i in range(1,ngroups+1):
    marker = lpf_get_value_n(pf, "markers", 0, i)
    if (marker != None):
        type = lpf_get_value_n(pf, "Type", 0, i)
        print("   Markers: ", marker)
        print("   Type: ", type)
        print("")
lpf_close(pf)
