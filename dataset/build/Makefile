##############################################################
# Source: Makefile
# Date: 
# Author: Mario J. Martin
# Description: Makefile for ADEPT TOOLS
##############################################################

### TO BE MODIFIED ###
CC = gcc

include $(CURDIR)/configure/mingw32
#include $(CURDIR)/configure/linux_airbus_devel

##############################################################

AR = ar
	
ifeq ($(SYSTEM), MINGW)
	/:=$(strip \)
	RM = del
	CP = copy
	MV = move
	MKDIR = mkdir
	CD = cd
	MAKE = mingw32-make.exe
	PROJECTS_HOME = $(CURDIR)$/..$/..
	EXE = .exe
	PYD = .pyd
	
	LIBGLFW = -netcdf
endif

ifeq ($(SYSTEM), LINUX)
	/:=/
	RM = rm -f
	CP = cp
	MV = mv
	MK = mkdir -f
	CD = cd
	MAKE = make
	PROJECTS_HOME = $(PWD)/../..
	PYD = .so

	PIC = -fPIC

	LIBGLFW = -netcdf
endif

ifeq ($(SYSTEM), MAC)
	/:=/
	RM = rm -f
	CP = cp
	MV = mv
	MK = mkdir -f
	CD = cd
	MAKE = make
	PROJECTS_HOME = $(PWD)/../..
	PYD = .so

	PIC = -fPIC

	LIBGLFW = -netcdf
endif

##### General flags #####
CCFLAGS = -O2 -Wall $(PIC) 
#-fopenmp
#-Wextra

##### Target directories #####
LIBDIR = ..$/lib
OBJDIR = .
BINDIR = ..$/bin

##############################################################
include $(PROJECTS_HOME)$/common$/build$/common.mak
include $(PROJECTS_HOME)$/param_file$/build$/param_file.mak
include $(PROJECTS_HOME)$/dataset$/build$/dataset.mak

##############################################################

help:
	@echo make all : Compiles all binaries and libraries.
	@echo make dataset_lib : Creates the static library.
	@echo make dataset_py : Creates the python library.
	@echo make clean : Clears .o files.
	@echo make allclean : Clears all files.
	@echo with cgns=0 compiles the library without linking with CGNS library.

all: dataset_lib dataset_py clean

allclean:
	$(RM) *.o
	$(RM) $(LIBDIR)$/dataset.a
	$(RM) $(BINDIR)$/dataset.py
	$(RM) $(BINDIR)$/_dataset.$PYD)

clean:
	$(RM) *.o

ifeq ($(cgns), 0)
NOCGNSFLAG = -DNO_CGNS
endif

dataset_lib: common_obj param_file_obj dataset_obj
	@echo _________________________________________
	@echo Linking dataset
	@echo _________________________________________
	$(AR) rcs $(LIBDIR)$/libdataset.a $(COMMON_OBJ) $(DATASET_OBJ)
	

dataset_py: common_obj param_file_obj dataset_obj
	@echo _________________________________________
	@echo Compiling dataset_py
	@echo _________________________________________
	$(SWIG) -python -outdir $(BINDIR) -o $(PY_DATASET_DIR)dataset_wrap.cxx \
	$(PY_DATASET_DIR)dataset.i

	@echo ...
	$(CC) ${CCFLAGS} -O2 -x c++ -c -std=c++11 $(PY_DATASET_DIR)dataset_wrap.cxx \
	-o dataset_wrap.o $(COMMON_INC) $(DATASET_INC) $(PYTHON_INC)
	
	@echo ...
	$(CC) -shared -o $(BINDIR)$/_dataset$(PYD) $(COMMON_OBJ) $(DATASET_OBJ) dataset_wrap.o \
	$(NETCDF_LIB) $(CGNS_LIB) $(PYTHON_LIB) $(LIB_NETCDF) $(LIB_CGNS) $(LIB_PYTHON) -lstdc++

	@echo Done!!
