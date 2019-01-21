##############################################################
# Source: Makefile
# Date: 
# Author: Mario J. Martin
# Description: Makefile for GLUUBS
##############################################################

### TO BE MODIFIED ###

CC = gcc

include $(CURDIR)/configure/mingw
#include $(CURDIR)/configure/linux

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
	PROJECTS_HOME = $(CURDIR)$/..
	EXE = .exe
	PYD = .pyd
	
	LIBGLFW = -lglfw3 -lopengl32 -lgdi32
endif

ifeq ($(SYSTEM), LINUX)
	/:=/
	RM = rm -f
	CP = cp
	MV = mv
	MK = mkdir -f
	CD = cd
	MAKE = make
	PROJECTS_HOME = $(PWD)/..
	PYD = .so

	PIC = -fPIC

	LIBGLFW = -lglfw3 -lGl -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXCursor -lpthread -lm -lrt
endif

ifeq ($(SYSTEM), MAC)
	/:=/
	RM = rm -f
	CP = cp
	MV = mv
	MK = mkdir -f
	CD = cd
	MAKE = make
	PROJECTS_HOME = $(PWD)/..
	PYD = .so

	PIC = -fPIC

	LIBGLFW = -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
endif

ifeq ($(CC), gcc)
	STDC11 = -std=c++11
endif

##### General flags #####
CCFLAGS = -O2 -Wall $(PIC) 
#-fopenmp
#-Wextra

##### Target directories #####
LIBDIR = $(PROJECTS_HOME)$/lib
BINDIR = $(PROJECTS_HOME)$/bin

#### Source directories #####
DEVDIR = $(PROJECTS_HOME)$/dev
SRCDIR = $(PROJECTS_HOME)$/src

GLEW_SRC = $(PROJECTS_HOME)$/ext_libs$/glew-1.12.0$/glew.c
GLEW_INC = -I$(PROJECTS_HOME)$/ext_libs$/glew-1.12.0$/include

COMMON_C = log.c file_encoding.c
COMMON_SRC := $(addprefix $(SRCDIR)$/common$/, $(COMMON_C))
COMMON_OBJ = $(COMMON_C:.c=.o)

GLW_CPP = camera.cpp glw.cpp glwthreads.cpp glwgl.cpp
GLW_SRC := $(addprefix $(SRCDIR)$/glw$/, $(GLW_CPP))
GLW_OBJ = $(GLW_CPP:.cpp=.o)

CONTEXT_GLFW_SRC = 	$(SRCDIR)$/context_glfw$/context_glfw.cpp 
CONTEXT_GLFW_OBJ = 	context_glfw.o

	
glew_obj:
	@echo ___________________________________________
	@echo Compiling GLEW...
	@echo _____ SYSTEM configured for $(SYSTEM) _____
	$(CC) -DGLEW_NO_GLU -DGLEW_BUILD -DGLEW_STATIC -O2 -x c -c $(PIC) \
	$(GLEW_INC) \
	$(GLEW_SRC) \
	-o glew.o 
	@echo Done!
	
common_lib: 
	@echo ___________________________________________
	@echo Compiling Common...
	@echo _____ SYSTEM configured for $(SYSTEM) _____
	$(CC) -O2 -x c -c $(PIC) \
	$(COMMON_SRC)
	@echo Done!
	
glw_lib:
	@echo ___________________________________________
	@echo Compiling GLW...
	@echo _____ SYSTEM configured for $(SYSTEM) _____
	$(CC) -O2 -x c++ $(STDC11) -c $(PIC) \
	-I$(PROJECTS_HOME)$/src $(GLEW_INC) \
	$(GLW_SRC) 
#	ar $(LIB)$/libglw.a $(GLW_OBJ)
	@echo Done!

context_glfw_lib:
	@echo ___________________________________________
	@echo Compiling Context_GLFW...
	@echo _____ SYSTEM configured for $(SYSTEM) _____
	$(CC) -O2 -x c++ $(STDC11) -c $(PIC) \
	$(GLEW_INC) $(GLFW_INC) -I$(PROJECTS_HOME)$/src $(GLW_INC) \
	$(CONTEXT_GLFW_SRC) 
#	ar $(LIBDIR)$/libcontext_glfw.a $(CONTEXT_GLFW_OBJ)
	@echo Done!


zdevgw00: glew_obj common_lib glw_lib
	@echo ___________________________________________
	@echo Compiling test bench gw00...
	@echo _____ SYSTEM configured for $(SYSTEM) _____
	$(CC) ${CCFLAGS} -DGLEW_STATIC $(STDC11) -o $(BINDIR)$/zdev_glw00$(EXE) \
	$(DEVDIR)$/zdev_glw00$/main.cpp \
	glew.o $(COMMON_OBJ) $(GLW_OBJ) \
	$(GLEW_INC) \
	$(GLFW_INC) \
	-I$(PROJECTS_HOME)$/src \
	$(GLEW_LIB) $(GLFW_LIB) -lstdc++ \
	$(LIBGLFW) 
	@echo Done!

zdevgw01: glew_obj common_lib glw_lib context_glfw_lib
	@echo ___________________________________________
	@echo Compiling test bench gw01...
	@echo _____ SYSTEM configured for $(SYSTEM) _____
	$(CC) ${CCFLAGS} -DGLEW_STATIC $(STDC11) -o $(BINDIR)$/zdev_glw01$(EXE) \
	$(DEVDIR)$/zdev_glw00$/main.cpp \
	glew.o $(COMMON_OBJ) $(GLW_OBJ) $(CONTEXT_GLFW_OBJ) \
	$(GLEW_INC) \
	$(GLFW_INC) \
	-I$(PROJECTS_HOME)$/src \
	$(GLEW_LIB) $(GLFW_LIB) -lstdc++ \
	$(LIBGLFW) 
	@echo Done!

allclear:
	$(RM) *.o
	$(RM) $(LIB)$/libglw.a
	$(RM) $(LIBDIR)$/libcontext_glfw.a
	$(RM) $(BINDIR)$/zdev_gw00$(EXE)
	$(RM) $(BINDIR)$/zdev_gw01$(EXE)
	$(MKDIR) $(BINDIR)
	$(MKDIR) $(LIBDIR)

all: glew_obj zdevgw00 zdevgw01
	$(RM) *.o