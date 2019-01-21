

COMMON_C = 	log.c file_encoding.c string_ext.c 
COMMON_DIR = $(PROJECTS_HOME)$/common$/src$/common$/
COMMON_INC = -I$(PROJECTS_HOME)$/common$/src
COMMON_SRC := $(addprefix $(COMMON_DIR), $(COMMON_C))
COMMON_OBJ = $(COMMON_C:.c=.o)

common_obj:
	@echo ________________________________________________
	@echo Compiling common...  SYSTEM configured for $(SYSTEM)
	@echo ________________________________________________
	$(CC) -O2 -x c -c $(PIC) $(COMMON_SRC) 
	
	@echo Done!!
	