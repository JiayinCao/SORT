BIN_PATH = ../bin
OBJ_PATH = $(BIN_PATH)/obj

LIBSRCS = $(wildcard geometry/*.cpp)
LIBSRCS += $(wildcard spectrum/*.cpp)
LIBSRCS += $(wildcard texture/*.cpp)
LIBSRCS += $(wildcard texture/texshower/*.cpp)

LIBOBJS = $(OBJ_PATH)/sort.o
LIBOBJS += $(addprefix $(OBJ_PATH)/, $(subst ._,, $(subst /,_,$(LIBSRCS:.cpp=.o)) ) )

sort : $(OBJ_PATH) $(LIBOBJS)
	@echo "linking"
	@g++ -Wall -o $(BIN_PATH)/sort $(LIBOBJS)
	@echo "sort is generated in the 'bin' dir."

$(OBJ_PATH) :
	@echo "creating directory $@"
	@mkdir -p $(OBJ_PATH)

$(OBJ_PATH)/geometry_%.o : geometry/%.cpp geometry/%.h
	@echo "building object $@"
	@g++ -c $< -o $@

$(OBJ_PATH)/spectrum_%.o : spectrum/%.cpp spectrum/%.h
	@echo "building object $@"
	@g++ -c $< -o $@

$(OBJ_PATH)/texture_%.o : texture/%.cpp texture/%.h
	@echo "building object $@"
	@g++ -c $< -o $@

$(OBJ_PATH)/texture_texshower_%.o : texture/texshower/%.cpp texture/texshower/%.h
	@echo "building object $@"
	@g++ -c $< -o $@

$(OBJ_PATH)/sort.o : sort.cpp sort.h
	@echo "building object $@"
	@g++ -c $< -o $@
	
clean:
	@echo "clear"
	@rm -rf $(BIN_PATH)
