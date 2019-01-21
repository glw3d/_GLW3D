
#### Source directories #####
DATASET_DIR = $(PROJECTS_HOME)$/dataset$/src
DATASET_INC = -I$(PROJECTS_HOME)$/dataset$/src
PY_DATASET_DIR = $(PROJECTS_HOME)$/dataset$/src$/dataset_py$/

DATASET_BASIC_CPP = dataset_arrays.cpp
DATASET_BASIC_DIR = $(DATASET_DIR)$/dataset$/
DATASET_BASIC_SRC := $(addprefix $(DATASET_BASIC_DIR), $(DATASET_BASIC_CPP))
DATASET_BASIC_OBJ = $(DATASET_BASIC_CPP:.cpp=.o)

DATASET_CGNS_C = cgns_import.c cgnsGrid.c
DATASET_CGNS_DIR = $(DATASET_DIR)$/cgns_tools$/
DATASET_CGNS_SRC := $(addprefix $(DATASET_CGNS_DIR), $(DATASET_CGNS_C))
DATASET_CGNS_OBJ = $(DATASET_CGNS_C:.c=.o)

DATASET_TAU_CPP = owTauGrid.cpp tau_export.cpp tau_gradients.cpp tau_import.cpp tau_normals.cpp
DATASET_TAU_DIR = $(DATASET_DIR)$/tau_tools$/
DATASET_TAU_SRC := $(addprefix $(DATASET_TAU_DIR), $(DATASET_TAU_CPP))
DATASET_TAU_OBJ = $(DATASET_TAU_CPP:.cpp=.o)

DATASET_CSV_CPP = csv_import.cpp
DATASET_CSV_DIR = $(DATASET_DIR)$/dataset$/csv$/
DATASET_CSV_SRC := $(addprefix $(DATASET_CSV_DIR), $(DATASET_CSV_CPP))
DATASET_CSV_OBJ = $(DATASET_CSV_CPP:.cpp=.o)

DATASET_IMG_C = image_edit.c import_bmp.c import_tga.c
DATASET_IMG_DIR = $(DATASET_DIR)$/dataset$/img$/
DATASET_IMG_SRC := $(addprefix $(DATASET_IMG_DIR), $(DATASET_IMG_C))
DATASET_IMG_OBJ = $(DATASET_IMG_C:.c=.o)


DATASET_OBJ = $(DATASET_BASIC_OBJ) $(DATASET_CGNS_OBJ) $(DATASET_TAU_OBJ) $(DATASET_CSV_OBJ) $(DATASET_IMG_OBJ)

dataset_basic_obj: 
	@echo ________________________________________________
	@echo Compiling dataset_basic...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c++ -c -std=c++11 $(PIC) $(NETCDF_INC) \
	$(COMMON_INC) $(DATASET_INC) $(DATASET_BASIC_SRC)
	
	@echo Done!!

dataset_cgns_obj: 
	@echo ________________________________________________
	@echo Compiling cgns_tools...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c -c $(PIC) ${NOCGNSFLAG} $(COMMON_INC) $(DATASET_INC) $(CGNS_INC) \
	$(DATASET_CGNS_SRC) 
	
	@echo Done!!

dataset_tau_obj: 
	@echo ________________________________________________
	@echo Compiling tau_tools...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c++ -c -std=c++11 $(PIC) $(COMMON_INC) $(DATASET_INC) $(NETCDF_INC) \
	$(DATASET_TAU_SRC)
	
	@echo Done!!

dataset_csv_obj: 
	@echo ________________________________________________
	@echo Compiling csv_import...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c++ -c -std=c++11 $(PIC) $(COMMON_INC) $(DATASET_INC) $(DATASET_CSV_SRC)
	
	@echo Done!!

dataset_img_obj:
	@echo ________________________________________________
	@echo Compiling dataset_img...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c -c $(PIC) $(COMMON_INC) $(DATASET_IMG_SRC) 
	
	@echo Done!!

dataset_obj: dataset_basic_obj dataset_tau_obj dataset_csv_obj dataset_img_obj dataset_cgns_obj
