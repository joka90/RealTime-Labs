C_FLAGS +=-c -Wall
INCLUDE_FLAGS = 
LIB_FLAGS =-lpthread 

C_FILES = $(basename $(notdir $(wildcard src/*.c)))
OBJECTS = $(addprefix obj/, $(addsuffix _$(TARGET_SUFFIX).o, $(C_FILES)))
H_FILES = $(wildcard src/*.h)

prog_$(TARGET_SUFFIX): $(OBJECTS)
	$(LD) $(LD_FLAGS) -o $@ $^ $(LIB_FLAGS)

obj/%_$(TARGET_SUFFIX).o: src/%.c $(H_FILES)
	$(CC) $(C_FLAGS) $< -o $@ $(INCLUDE_FLAGS)

clean: 
	rm ./obj/*_$(TARGET_SUFFIX).o