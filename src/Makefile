BIN_PATH = ../bin
OBJ_PATH = $(BIN_PATH)/obj
INC_PATH = -I.

COMPILE_FLAG = $(INC_PATH) -g

LIBSRCS = $(wildcard *.cpp)
LIBSRCS += $(wildcard geometry/*.cpp)
LIBSRCS += $(wildcard camera/*.cpp)
LIBSRCS += $(wildcard spectrum/*.cpp)
LIBSRCS += $(wildcard texture/*.cpp)
LIBSRCS += $(wildcard managers/*.cpp)
LIBSRCS += $(wildcard managers/texio/*.cpp)
LIBSRCS += $(wildcard managers/meshio/*.cpp)
LIBSRCS += $(wildcard accel/*.cpp)
LIBSRCS += $(wildcard utility/*.cpp )
LIBSRCS += $(wildcard integrator/*.cpp)
LIBSCCS += $(wildcard thirdparty/ply/*.c)

LIBOBJS = $(addprefix $(OBJ_PATH)/, $(subst ._,, $(subst /,_,$(LIBSRCS:.cpp=.o)) ) )
LIBOBJS +=$(addprefix $(OBJ_PATH)/, thirdparty_ply_ply.o )

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

$(OBJ_PATH)/managers_texio_%.o : managers/texio/%.cpp managers/texio/%.h managers/texio/texio.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/managers_meshio_%.o : managers/meshio/%.cpp managers/meshio/%.h managers/meshio/meshloader.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/sort.o : sort.cpp sort.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/accel_%.o : accel/%.cpp accel/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/integrator_%.o : integrator/%.cpp integrator/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/thirdparty_ply_ply.o : thirdparty/ply/ply.c thirdparty/ply/ply.h
	@echo "building object $@"
	@gcc $(COMPILE_FLAG) -c $< -o $@
	

clean:
	@echo "clear"
	@rm -rf $(BIN_PATH)
