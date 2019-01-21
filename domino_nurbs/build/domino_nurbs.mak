#### Source directories #####
DOMINO_NURBS_DIR = $(PROJECTS_HOME)$/domino_nurbs$/src$/domino_nurbs$/
DOMINO_NURBS_INC = -I$(PROJECTS_HOME)$/domino_nurbs$/src
PY_DOMINO_NURBS_DIR = $(PROJECTS_HOME)$/domino_nurbs$/src$/domino_nurbs_py$/

#### Source files #####
DOMINO_NURBS_C = nurbs_ascii_io.c nurbs_basis.c nurbs_controlbox.c nurbs_curve.c nurbs_iges_io.c nurbs_surface.c nurbs_surface_inversion.c nurbs_surface_intersection.c
DOMINO_NURBS_SRC := $(addprefix $(DOMINO_NURBS_DIR), $(DOMINO_NURBS_C))
DOMINO_NURBS_OBJ = $(DOMINO_NURBS_C:.c=.o)

PY_DOMINO_NURBS_CPP = nurbs_py_tools.cpp
PY_DOMINO_NURBS_SRC := $(addprefix $(DOMINO_NURBS_DIR), $(PY_DOMINO_NURBS_CPP))
PY_DOMINO_NURBS_OBJ = $(PY_DOMINO_NURBS_CPP:.cpp=.o)

domino_nurbs_obj: 
	@echo ________________________________________________
	@echo Compiling domino_nurbs...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c -c $(PIC)  \
	$(COMMON_INC) $(DOMINO_NURBS_INC) $(DOMINO_NURBS_SRC)
	
	@echo Done!!

py_domino_nurbs_obj:
	@echo ________________________________________________
	@echo Compiling py_domino_nurbs tools...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c++ -c -std=c++11 $(PIC)  \
	$(COMMON_INC) $(DOMINO_NURBS_INC) $(PY_DOMINO_NURBS_SRC)
	
	@echo Done!!
