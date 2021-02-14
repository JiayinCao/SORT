#
#    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
#    platform physically based renderer.
#
#    Copyright (c) 2011-2020 by Jiayin Cao - All rights reserved.
#
#    SORT is a free software written for educational purpose. Anyone can distribute
#    or modify it under the the terms of the GNU General Public License Version 3 as
#    published by the Free Software Foundation. However, there is NO warranty that
#    all components are functional in a perfect manner. Without even the implied
#    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along with
#    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
#

# SORT root directory
SORT_DIR       := $(shell pwd -P)

# detect the current arch, by default it is what the current platform is.
# this can be overwritten like this so that we can do cross compiling
#   make release ARCH=arm64
ARCH ?= $(shell uname -m)

# unified command by using python script
UPDATE_DEP_COMMAND           = @python3 ./scripts/get_dep.py FALSE "${ARCH}"
FORCE_UPDATE_DEP_COMMAND     = @python3 ./scripts/get_dep.py TRUE "${ARCH}"

BUILD_RELEASE_COMMAND        = @echo "building release version.";cd $(SORT_DIR); mkdir proj_release; cd proj_release; cmake -DCMAKE_OSX_ARCHITECTURES=${ARCH} -DCMAKE_BUILD_TYPE=Release ..;make -j 4;cd ..;
BUILD_DEBUG_COMMAND          = @echo "building debug version.";cd $(SORT_DIR); mkdir proj_debug; cd proj_debug; cmake -DCMAKE_OSX_ARCHITECTURES=${ARCH} -DCMAKE_BUILD_TYPE=Debug ..;make -j 4;cd ..;
BUILD_RELWITHDEBINFO_COMMAND = @echo "building release version with debug information.";cd $(SORT_DIR); mkdir proj_relwithdebinfo; cd proj_relwithdebinfo; cmake -DCMAKE_OSX_ARCHITECTURES=${ARCH} -DCMAKE_BUILD_TYPE=RelWithDebInfo ..;make -j 4;cd ..;

release: .FORCE
	$(UPDATE_DEP_COMMAND)
	$(BUILD_RELEASE_COMMAND)

debug: .FORCE
	$(UPDATE_DEP_COMMAND)
	$(BUILD_DEBUG_COMMAND)

relwithdebinfo: .FORCE
	$(UPDATE_DEP_COMMAND)
	$(BUILD_RELWITHDEBINFO_COMMAND)

update: .FORCE
	@echo 'Syncing source code from Github'
	@git pull

update_dep: .FORCE
	@echo 'Syncing dependencies'
	$(UPDATE_DEP_COMMAND)

force_update_dep: .FORCE
	@echo 'Syncing dependencies'
	$(FORCE_UPDATE_DEP_COMMAND)

clean: .FORCE
	@echo 'Cleaning all generated files'
	@cd $(SORT_DIR); rm -rf bin ; rm -rf proj_release ; rm -rf proj_debug ; rm -rf proj_relwithdebinfo ; rm -rf _out ; rm -rf generated_src ;

clean_dep: .FORCE
	@echo 'Cleaning all dependency files'
	@cd $(SORT_DIR); rm -rf dependencies

help: .FORCE
	@python3 ./scripts/show_help.py

about: .FORCE
	@python3 ./scripts/show_about.py

dep_info: .FORCE
	@python3 ./scripts/show_dep_info.py

generate_src: .FORCE
	@echo 'Generating source code'
	@python3 ./scripts/generate_src.py

verify_builds: .FORCE
	@echo 'Verifying builds'
	@python3 ./scripts/verify_builds.py

.FORCE:
