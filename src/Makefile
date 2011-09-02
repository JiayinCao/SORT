BIN_PATH = ../bin
OBJ_PATH = $(BIN_PATH)/obj
INC_PATH = -I. -I./thirdparty/ilmbase-1.0.2/ -I./thirdparty/openexr-1.7.0/ -I./thirdparty/zlib-1.2.5/

COMPILE_FLAG = $(INC_PATH) -g -fopenmp

LIBSRCS = $(wildcard *.cpp)
LIBSRCS += $(wildcard geometry/*.cpp)
LIBSRCS += $(wildcard geometry/sky/*.cpp)
LIBSRCS += $(wildcard camera/*.cpp)
LIBSRCS += $(wildcard spectrum/*.cpp)
LIBSRCS += $(wildcard texture/*.cpp)
LIBSRCS += $(wildcard managers/*.cpp)
LIBSRCS += $(wildcard managers/texio/*.cpp)
LIBSRCS += $(wildcard managers/meshio/*.cpp)
LIBSRCS += $(wildcard accel/*.cpp)
LIBSRCS += $(wildcard utility/*.cpp )
LIBSRCS += $(wildcard integrator/*.cpp)
LIBSRCS += $(wildcard light/*.cpp)
LIBSRCS += $(wildcard bsdf/*.cpp)
LIBSRCS += $(wildcard sampler/*.cpp)
LIBSRCS += $(wildcard shape/*.cpp)
LIBSRCS += $(wildcard material/*.cpp)
LIBSRCS += $(wildcard thirdparty/tinyxml/*.cpp)
LIBSRCS += $(wildcard thirdparty/ilmbase-1.0.2/*.cpp)
LIBSRCS += $(wildcard thirdparty/openexr-1.7.0/*.cpp)
LIBSRCS += $(wildcard thirdparty/zlib-1.2.5/*.c)
LIBSRCS += $(wildcard thirdparty/ply/*.c)

LIBOBJS_ = $(addprefix $(OBJ_PATH)/, $(subst ._,, $(subst /,_,$(LIBSRCS:.cpp=.o)) ) )
LIBOBJS = $(LIBOBJS_:.c=.o)

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
	
$(OBJ_PATH)/geometry_sky_%.o : geometry/sky/%.cpp geometry/sky/%.h geometry/sky/sky.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/camera_%.o : camera/%.cpp camera/%.h camera/camera.h
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

$(OBJ_PATH)/light_%.o : light/%.cpp light/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/bsdf_%.o : bsdf/%.cpp bsdf/%.h bsdf/fresnel.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@
	
$(OBJ_PATH)/sampler_%.o : sampler/%.cpp sampler/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/shape_%.o : shape/%.cpp shape/%.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/material_%.o : material/%.cpp material/%.h utility/propertyset.h
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@
	
$(OBJ_PATH)/thirdparty_tinyxml_%.o : thirdparty/tinyxml/%.cpp
	@echo "building object $@"
	@g++ $(COMPILE_FLAG) -c $< -o $@
	
$(OBJ_PATH)/thirdparty_ply_%.o : thirdparty/ply/%.c thirdparty/ply/%.h
	@echo "building object $@"
	@gcc $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/thirdparty_ilmbase-1.0.2_%.o : thirdparty/ilmbase-1.0.2/%.cpp
	@echo "building object $@"
	@gcc $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/thirdparty_openexr-1.7.0_%.o : thirdparty/openexr-1.7.0/%.cpp
	@echo "building object $@"
	@gcc $(COMPILE_FLAG) -c $< -o $@

$(OBJ_PATH)/thirdparty_zlib-1.2.5_%.o : thirdparty/zlib-1.2.5/%.c
	@echo "building object $@"
	@gcc $(COMPILE_FLAT) -c $< -o $@

clean:
	@echo "clear"
	@rm -rf $(BIN_PATH)
