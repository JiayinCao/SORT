BIN_PATH = ../bin
OBJ_PATH = $(BIN_PATH)/obj

LIBSRCS = $(wildcard *.cpp)
LIBSRCS += $(wildcard geometry/*.cpp)
LIBSRCS += $(wildcard camera/*.cpp)
LIBSRCS += $(wildcard spectrum/*.cpp)
LIBSRCS += $(wildcard texture/*.cpp)
LIBSRCS += $(wildcard managers/*.cpp)
LIBSRCS += $(wildcard managers/texmanager/*.cpp)
LIBSRCS += $(wildcard managers/texmanager/texio/*.cpp)
LIBSRCS += $(wildcard utility/*.cpp )

LIBOBJS = $(addprefix $(OBJ_PATH)/, $(subst ._,, $(subst /,_,$(LIBSRCS:.cpp=.o)) ) )

sort : $(OBJ_PATH) $(LIBOBJS)
	@g++ -O3 -Wall -o $(BIN_PATH)/sort $(LIBOBJS)
	@echo "sort is generated in the 'bin' dir."

$(OBJ_PATH) :
	@echo "creating directory $@"
	@mkdir -p $(OBJ_PATH)

$(OBJ_PATH)/main.o : main.cpp
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/%.o : %.cpp %.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/geometry_%.o : geometry/%.cpp geometry/%.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/camera_%.o : camera/%.cpp camera/%.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/spectrum_%.o : spectrum/%.cpp spectrum/%.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/texture_%.o : texture/%.cpp texture/%.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/utility_%.o : utility/%.cpp utility/%.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/managers_%.o : managers/%.cpp managers/%.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/managers_texmanager_%.o : managers/texmanager/%.cpp managers/texmanager/%.h
	@echo "buliding object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/managers_texmanager_texio_%.o : managers/texmanager/texio/%.cpp managers/texmanager/texio/%.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@

$(OBJ_PATH)/sort.o : sort.cpp sort.h
	@echo "building object $@"
	@g++ -O3 -c $< -o $@
	
clean:
	@echo "clear"
	@rm -rf $(BIN_PATH)
