import sys
sys.path.append('../bin')
from dataset import *


tuple = cgnsGrid_import("../test/cgns/rae2822_coarse_upd.cgns");
if (tuple == None):
    exit(1)

grid = tuple[0]
ngrids = tuple[1]
print(grid.label)
print("num points: ", grid.num_points)
print(grid.point_x)
for i in range(0,10):
    print(grid.point_x[i], grid.point_y[i], grid.point_z[i])
cgnsGrid_free(grid, ngrids)

tuple = cgnsGrid_import("../test/cgns/HeatingCoil.cgns");
if (tuple == None):
    exit(1)

grid = tuple[0]
ngrids = tuple[1]
print(grid.label)
print("num points: ", grid.num_points)
for i in range(0,10):
    print(grid.point_x[i], grid.point_y[i], grid.point_z[i])
cgnsGrid_free(grid, ngrids)



