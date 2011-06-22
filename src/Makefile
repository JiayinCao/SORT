BIN_PATH = ../bin
OBJ_PATH = $(BIN_PATH)/obj
INC_PATH = -I.

COMPILE_FLAG = $(INC_PATH)

LIBSRCS = $(wildcard *.cpp)
LIBSRCS += $(wildcard geometry/*.cpp)
LIBSRCS += $(wildcard camera/*.cpp)
LIBSRCS += $(wildcard spectrum/*.cpp)
LIBSRCS += $(wildcard texture/*.cpp)
LIBSRCS += $(wildcard managers/*.cpp)
LIBSRCS += $(wildcard managers/texio/*.cpp)
LIBSRCS += $(wildcard managers/meshio/*.cpp)
LIBSRCS += $(wildcard utility/*.cpp )

LIBOBJS = $(addprefix $(OBJ_PATH)/, $(subst ._,, $(subst /,_,$(LIBSRCS:.cpp=.o)) ) )

sort : $(OBJ_PATH) $(LIBOBJS)
	@g++ $(COMPILE_FLAG) -o $(BIN_PATH)/sort $(LIBOBJS)
	@echo "sort is generated in the 'bin' dir."

$(OBJ_PATH) :
	@echo "creating directory $@"
	@mkdir -p $(OBJ_PATH)

$(OBJ_PATH)/main.o : main.cpp
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/%.o : %.cpp %.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/geometry_%.o : geometry/%.cpp geometry/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/camera_%.o : camera/%.cpp camera/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/spectrum_%.o : spectrum/%.cpp spectrum/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/texture_%.o : texture/%.cpp texture/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/utility_%.o : utility/%.cpp utility/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/managers_%.o : managers/%.cpp managers/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/managers_texio_%.o : managers/texio/%.cpp managers/texio/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/sort.o : sort.cpp sort.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@
	
clean:
	@echo "clear"
	@rm -rf $(BIN_PATH)
