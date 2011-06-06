SRC_PATH = .
BIN_PATH = ../bin
OBJ_PATH = $(BIN_PATH)/obj

LIBSRCS = $(wildcard $(SRC_PATH)/geometry/*.cpp)
LIBSRCS += $(wildcard $(SRC_PATH)/texture/*.cpp)
LIBSRCS += $(wildcard $(SRC_PATH)/texture/texshower/*.cpp)

LIBOBJS = $(OBJ_PATH)/sort.o
LIBOBJS += $(addprefix $(OBJ_PATH)/, $(subst ._,, $(subst /,_,$(LIBSRCS:.cpp=.o)) ) )

sort : $(OBJ_PATH) $(LIBOBJS)
	g++ -o $(BIN_PATH)/sort $(LIBOBJS)

$(OBJ_PATH) :
	mkdir -p $(OBJ_PATH)

$(OBJ_PATH)/geometry_%.o : $(SRC_PATH)/geometry/%.cpp $(SRC_PATH)/geometry/%.h
	g++ -c $< -o $@

$(OBJ_PATH)/texture_%.o : $(SRC_PATH)/texture/%.cpp $(SRC_PATH)/texture/%.h
	g++ -c $< -o $@

$(OBJ_PATH)/texture_texshower_%.o : $(SRC_PATH)/texture/texshower/%.cpp $(SRC_PATH)/texture/texshower/%.h
	g++ -c $< -o $@

$(OBJ_PATH)/sort.o : $(SRC_PATH)/sort.cpp
	g++ -c $(SRC_PATH)/sort.cpp -o $(OBJ_PATH)/sort.o

clean:
	rm -rf $(BIN_PATH)
