# name
EXEC = nca

# Compiler
CPP = clang++

# Directories
INCLUDE = /opt/homebrew/include
LIB_DIR = /opt/homebrew/lib

# Compiler flags
STD = -std=c++17
OPT = -Ofast
CPPFLAGS = $(STD) $(OPT)

# Linker related
LDLIBS = -lSDL2 -ltbb

# Files
SRC_FILES = main.cpp
HEADERS = nca.hpp
OBJ = main.o

all: neural_cellular_automata
	@echo "Finished build :3"

neural_cellular_automata: $(OBJ)
	@echo "Building executable!"
	$(CPP) $(OBJ) -o \
		$(EXEC) -I$(INCLUDE) -L$(LIB_DIR) \
		$(LDLIBS)

main.o: $(HEADERS) $(SRC_FILES)
	@echo "Building main.o!"
	$(CPP) $(CPPFLAGS) -c main.cpp -I$(INCLUDE)

.PHONY: all clean very-clean super-clean automagic

# Makes executable and cleans
automagic: all clean

# Deletes everything generated
super-clean:
	rm $(OBJ)
	rm $(EXEC)
	@echo "Super clean! :3"

# Deletes the object files
clean:
	rm $(OBJ)
	@echo "Cleaned objects :D"

# Deletes the executable file
deep-clean:
	rm $(EXEC)
	@echo "Deep clean: removed executable file $(EXEC) :D"
