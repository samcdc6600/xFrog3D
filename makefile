CXX	= clang++
#CXX	= g++
CFLAGS	= -Wall -std=c++14 -Wpedantic -Wextra -O3 -I/usr/local/include/
LIBS	= -lglut -lGLU -lGL src/include/libSOIL.a -L/usr/local/lib/
BIN_NAME = xFrog3D
SRC   = ./src
IDIR  = $(SRC)/include
ODIR  = ./obj
DEPS  = $(wildcard $(IDIR)/*.h*)
CPP_DIRS  =  $(wildcard $(SRC)/*.cpp) # Get list of .cpp file directories.
CPP_FILES = $(subst $(SRC)/,,$(CPP_DIRS)) # Strip srings of paths.
OBJ_FILES = $(patsubst %.cpp,%.o,$(CPP_FILES)) # Replace .cpp with .o.
OBJ       = $(patsubst %,$(ODIR)/%,$(OBJ_FILES))  # Prepend object directory to .o file names.


$(ODIR)/%.o: $(SRC)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

$(BIN_NAME): $(OBJ)
	$(CXX) -o $@ $^ $(LIBS)

.PHONY: clean

# Files with '#' and '~' seem to be left around be emacs >:'^(
clean:
	$(RM) $(BIN_NAME) $(ODIR)/*.o *.core *~ $(SRC)/*~ $(IDIR)/*~ \#*\# $(SRC)/\#*\# $(IDIR)/\#*\#
