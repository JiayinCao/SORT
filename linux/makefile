SRC_PATH = ../src
OBJ_PATH = obj

LIBSRCS = $(wildcard $(SRC_PATH)/geometry/*.cpp)
LIBOBJS = $(addprefix $(OBJ_PATH)/, $(subst ..,, $(subst /,_,$(LIBSRCS:.cpp=.o)) ) )
LIBOBJS += $(OBJ_PATH)/sort.o

sort : $(OBJ_PATH) $(LIBOBJS)
	g++ -o sort $(LIBOBJS)

$(OBJ_PATH) :
	mkdir -p $(OBJ_PATH)

$(OBJ_PATH)/_src_geometry_%.o : $(SRC_PATH)/geometry/%.cpp
	g++ -c $< -o $@

$(OBJ_PATH)/sort.o : $(SRC_PATH)/sort.cpp
	g++ -c $(SRC_PATH)/sort.cpp -o $(OBJ_PATH)/sort.o

clean:
	rm -rf obj sort
