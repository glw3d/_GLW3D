import sys
sys.path.append('../bin')
from dataset import *


grid = owTauGrid_import("naca0012/naca0012.grid");
print(grid.label)
print("num points: ", grid.points.length)
for i in range(0,10):
    print(grid.points[i].x, grid.points[i].y, grid.points[i].z)
owTauGrid_free(grid)

sol_pressure = owTauSol_import_dbl( "naca0012/naca0012.solution.pval.gg.790", "cp" );
if (sol_pressure != None):
    print(sol_pressure.length)
    for i in range(0,10):
        print(sol_pressure[i])
    owDoubleStream_free(sol_pressure)

gid = owTauSol_import_int( "naca0012/naca0012.solution.pval.gg.790", "global_id" );
if (gid != None):
    print(gid.length)
    for i in range(0,10):
        print(gid[i])
    owIntStream_free(gid)

c_lift = owTauSol_getatt( "../test/naca0012/naca0012.solution.pval.gg.790", "c_l" );
print("C_lift: ", c_lift)

a0 = owDoubleStream();
print(a0.stream)
print(a0.length)
owDoubleStream_free(a0)
print(a0.length)

a1 = owDoubleStream_create( 256 );
print(a1.stream)
print(a1.length)
owDoubleStream_free(a1)
