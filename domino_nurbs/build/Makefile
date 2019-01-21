##############################################################
# Source: Makefile
# Date: 
# Author: Mario J. Martin
# Description: Standard Makefile
##############################################################

### TO BE MODIFIED ###

include $(CURDIR)/configure/mingw32
#include $(CURDIR)/configure/linux

##############################################################

CC = gcc
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
	PIC = -fPIC
	PYD = .so
endif
ifeq ($(SYSTEM), CYGWIN)
	/:=/
	RM = rm -f
	CP = cp
	MV = mv
	MK = mkdir -f
	CD = cd
	MAKE = make
	PROJECTS_HOME = /cygdrive/c/PROJECTS/DEVELOP
	EXE = .exe
	PYD = .so
endif

CCFLAGS = -O2 -Wall -fopenmp $(PIC)
#-Wextra

##### Target directories #####
LIBDIR = ..$/lib
OBJDIR = .
BINDIR = ..$/bin

##############################################################
include $(PROJECTS_HOME)$/common$/build$/common.mak
include $(PROJECTS_HOME)$/domino_nurbs$/build$/domino_nurbs.mak

##############################################################
	
help:
	@echo make all : Compiles all binaries and libraries.
	@echo make domino_nurbs_lib : Creates the static library.
	@echo make domino_nurbs_py : Creates the python library.
	@echo make clean : Clears .o files.
	@echo make allclean : Clears all files.
	
all: domino_nurbs_lib domino_nurbs_py clean

allclean:
	$(RM) *.o
	$(RM) $(LIBDIR)$/libdomino_nurbs.a
	$(RM) $(BINDIR)$/libdomino_nurbs.py
	$(RM) $(BINDIR)$/_libdomino_nurbs.$PYD)

clean:
	$(RM) *.o
	
domino_nurbs_lib: common_obj domino_nurbs_obj
	@echo _________________________________________
	@echo Linking domino_nurbs
	@echo _________________________________________
	$(AR) rcs $(LIBDIR)$/libdomino_nurbs.a $(COMMON_OBJ) $(DOMINO_NURBS_OBJ)
	@echo Done!!
	
domino_nurbs_py: common_obj domino_nurbs_obj  py_domino_nurbs_obj
	@echo _________________________________________
	@echo Compiling domino_nurbs_py
	@echo _________________________________________
	$(SWIG) -python -outdir $(BINDIR) -o $(PY_DOMINO_NURBS_DIR)domino_nurbs_wrap.cxx \
	$(PY_DOMINO_NURBS_DIR)domino_nurbs.i

	$(CC) ${CCFLAGS} -O2 -x c++ -c -std=c++11 $(PY_DOMINO_NURBS_DIR)domino_nurbs_wrap.cxx \
	-o domino_nurbs_wrap.o $(COMMON_INC) $(DOMINO_NURBS_INC) $(PYTHON_INC)
	
	$(CC) -shared -o $(BINDIR)$/_domino_nurbs$(PYD) $(COMMON_OBJ) $(DOMINO_NURBS_OBJ) $(PY_DOMINO_NURBS_OBJ) domino_nurbs_wrap.o \
	$(PYTHON_LIB) $(LIB_PYTHON) -lstdc++

	@echo Done!!
