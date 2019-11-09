define HELP_TEXT

Available commands

    * final
        Build final version.
    * release
        Build release version.
    * debug
        Build debug version.
    * update
        Sync the latest code from Github
    * update_dep
        Update dependency files.
    * clean
    	Clean up all generated file, including the binaries.
    	But dependency will still exist.
    * clean_dep
    	Clean the dependency.
    * help
        Print this help message.
    * about
        Introduction about SORT and myself.
    * dep_info
        Introduction about the third party libraries used in SORT.

Convenience targets

    * final       
        The final version that has no debugging information. This is the
        best option for practical rendering.
    * release
        The relatively performant build version that provides reasonable 
        information, like statistics and log. However, there is no debug
        information.
    * debug
        Standard debugging version that has everything, which is only for
        debugging purposes.

endef

define DEP_TEXT

Following are dependencies used in SORT renderer

    * Open Shading Language
        OSL is what SORT is currently using as shading language. With the
        support of OSL, SORT can compile shaders during runtime to allow
        artist more flexible support in shader assets.
    * Easy Profiler
        Easy profiler is currently used as main profiling tool. It is more
        of a higher level profiling tool for tasks, instead of low level
        profiling tool.
    * stb_image
        Stb_image is used to read textures from file system. It is already
        included in the source code in the thirdparty folder.
    * Google Test Framework
        SORT uses Google test as its framework for unit testing. This is
        also included in the thirdparty folder in SORT source code.
    * Tiny Exr
        Tiny Exr is a open source library for reading and writting exr files.
        Like the other two libraries, the source code is also included in 
        'thirdpard' folder.

endef

define ABOUT_TEXT

SORT is short for Simple Open-source Ray Tracing. It is a stand along cross platform 
physically based renderer. It is my personal side project that I have been working 
on ever since 2011. Following are features supported in SORT renderer so far,

    * Path Tracing, Bidirectional Path Tracing, Light Tracing, Instant Radiosity, 
      Ambient Occlusion, Whitted Ray Tracing.
    * KD-Tree, BVH, OcTree, Uniform Grid.
    * Point Light, Spot Light, Directional Light, Area Light, HDR Image Based Lighting.
    * Disney BRDF (2015), Lambert, LambertTransmission, Oran Nayar, Microfacet Reflection,
      Microfacet Tranmission, MERL Measured BRDF, Fourier BRDF, AshikhmanShirley BRDF,
      Phong, Coat, Blend, Double-Sided, Ditribution BRDF, DreamWorks' Fabric BRDF.
    * Sursurface Scattering.
    * Hair, Fur.
    * Open Shading Language.
    * Depth of Field.
    * Multi-threaded Rendering.
    * Blender 2.8.

Please visit the following website for more information,

    * http://sort-renderer.com/

My name is Jiayin Cao. I'm currently working at Naughty Dog as a graphics engineer.
Prior to joining Naughty Dog, I worked at Ubisoft, NVIDIA, AMD and Microsoft before.
Following are my contacts, feel free to contact me for any reason,
	
    * Linkedin,   https://www.linkedin.com/in/caojiayin/
    * Twitter,    https://twitter.com/Jiayin_Cao
    * Tech blog,  https://agraphicsguy.wordpress.com/
    * Email,      caojiayin1985@icloud.com

endef

# SORT root directory
SORT_DIR       := $(shell pwd -P)
# SORT dependency folder
SORT_DEP_DIR   := $(SORT_DIR)/dependencies
# Operating system name, it could be Darwin or Linux
OS             := $(shell uname -s | tr A-Z a-z)

# Update dependency command
UPDATE_DEP_COMMAND = @echo "Unfortunately, the dependencies in this OS has not been prebuilt. You can choose to build them by yourself or find a platform with existed pre-built dependencies."

# Mac OS
ifeq ($(OS), darwin)
	UPDATE_DEP_COMMAND = sh ./getdep_mac.sh
endif

# Ubuntu
ifeq ($(OS), linux)
	# Different Ubuntu have different version of libraries, we need to tell which version it is.
	# I have only built dependencies for Ubuntu Xenial and Bionic. In order to build other versions,
	# it is necessary to build the library first.
	OS_VERS:=$(shell lsb_release -a 2>/dev/null | grep Description | awk '{ print $$2 "-" $$3 }')
	ifeq ($(findstring Ubuntu-16,$(OS_VERS)),Ubuntu-16)
		UPDATE_DEP_COMMAND = sh ./getdep_ubuntu_xenial.sh
	endif
	ifeq ($(findstring Ubuntu-18,$(OS_VERS)),Ubuntu-18)
		UPDATE_DEP_COMMAND = sh ./getdep_ubuntu_bionic.sh
	endif
endif

BUILD_RELEASE_COMMAND = @echo "building release version.";cd $(SORT_DIR); mkdir proj_release; cd proj_release; cmake -DCMAKE_BUILD_TYPE=Release ..;make -j 4;cd ..;
BUILD_DEBUG_COMMAND = @echo "building debug version.";cd $(SORT_DIR); mkdir proj_debug; cd proj_release; cmake -DCMAKE_BUILD_TYPE=Debug ..;make -j 4

# Check if the depedency files are already downloaded.
ifeq ("$(wildcard $(SORT_DEP_DIR))","")
    BUILD_RELEASE_COMMAND = @echo "Please check out the dependency files before building. Run 'make update_dep' first."
    BUILD_DEBUG_COMMAND   = @echo "Please check out the dependency files before building. Run 'make update_dep' first."
endif

release: .FORCE
	$(BUILD_RELEASE_COMMAND)

final: .FORCE
	@echo 'building final version.'
	@echo 'Currently not supported.'

debug: .FORCE
	$(BUILD_DEBUG_COMMAND)

update: .FORCE
	@echo 'Syncing source code from Github'
	git pull

update_dep: .FORCE
	@echo 'Syncing dependencies'
	@echo $(OS_NAME)
	$(UPDATE_DEP_COMMAND)

clean: .FORCE
	@echo 'Cleaning all generated files'
	cd $(SORT_DIR); rm -rf bin ; rm -rf proj_release ; rm -rf proj_debug ; rm -rf _out

clean_dep: .FORCE
	@echo 'Cleaning all dependency files'
	cd $(SORT_DIR); rm -rf dependencies

export HELP_TEXT
help: .FORCE
	@echo "$$HELP_TEXT"

export ABOUT_TEXT
about: .FORCE
	@echo "$$ABOUT_TEXT"

export DEP_TEXT
dep_info: .FORCE
	@echo "$$DEP_TEXT"

.FORCE: