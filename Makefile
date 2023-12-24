### Settings ###

## File Structure ##

SOURCE_DIR	:= .
BUILD_DIR	:= build
INSTALL_DIR	:= .
LIB_DIR	:= lib
APP_DIR	:= app
TEST_DIR	:= test


## Building ##

# Set the default building configurations
DEFAULT_CFG	:= Release
PARALLELISM	:= $(shell nproc)


## Output Logs ##

# Suppress noisy logs about entering and leaving directories
MAKEFLAGS	+= --no-print-directory


## Miscellaneous ##

CP ?= cp -f
RSYNC	?= rsync -ut --progress



### Targets ###

## Default Target ##

.PHONY: default
default: clean gen build install


## Commands Handling the Building System ##

# Clean whole the generated files and directories
.PHONY: clean c
clean: clean-build clean-lib clean-cache clean-test clean-app
c: clean

# Clean whole the generated build system
.PHONY: clean-build cb
clean-build:
	$(RM) -rd -- "$(BUILD_DIR)"
cb: clean-build

# Clean whole the compiled libraries
.PHONY: clean-lib cl
clean-lib:
	$(RM) -rd -- "$(INSTALL_DIR)/$(LIB_DIR)"
	$(RM) -- \
		"$(INSTALL_DIR)/$(TEST_DIR)/"*".so" \
		"$(INSTALL_DIR)/$(TEST_DIR)/"*".pyi" \
		"$(INSTALL_DIR)/$(APP_DIR)/"*".so" \
		"$(INSTALL_DIR)/$(APP_DIR)/"*".pyi"
cl: clean-lib

# Clean whole the caches
.PHONY: clean-cache cc
clean-cache:
	$(RM) -rd -- \
		"$(INSTALL_DIR)/$(TEST_DIR)/__pycache__" \
		"$(INSTALL_DIR)/$(TEST_DIR)/.pytest_cache" \
		"$(INSTALL_DIR)/$(APP_DIR)/__pycache__" \
		"$(INSTALL_DIR)/$(APP_DIR)/.pytest_cache" \
		"__pycache__" \
		".pytest_cache"
cc: clean-cache

# Clean whole the test outputs
.PHONY: clean-test ct
clean-test:
	$(RM) -rd -- "$(INSTALL_DIR)/$(TEST_DIR)/out"
ct: clean-test

# Clean whole the app outputs
.PHONY: clean-app ca
clean-app:
	$(RM) -rd -- "$(INSTALL_DIR)/$(APP_DIR)/out"
ca: clean-app

# Generated the build system
.PHONY: gen g
gen:
	cmake -S "$(SOURCE_DIR)" -B "$(BUILD_DIR)" \
		-DCMAKE_BUILD_TYPE=$(DEFAULT_CFG)
g: gen

# Build with the generated build system
.PHONY: build b
build:
	cmake --build "$(BUILD_DIR)" --config $(DEFAULT_CFG) \
		--parallel $(PARALLELISM)
b: build

# Install the compiled libraries
.PHONY: install-lib install-app install-test install i
install-lib:
	cmake --install "$(BUILD_DIR)" --config $(DEFAULT_CFG) \
		--prefix "$(INSTALL_DIR)"
install-app:
	$(RSYNC) -r "$(INSTALL_DIR)/$(LIB_DIR)/"* "$(INSTALL_DIR)/$(APP_DIR)/"
install-test:
	$(RSYNC) -r "$(INSTALL_DIR)/$(LIB_DIR)/"* "$(INSTALL_DIR)/$(TEST_DIR)/"
install: install-lib install-app install-test
i: install

# Run the testing Python scripts
.PHONY: test t
test:
	cd "$(INSTALL_DIR)/$(TEST_DIR)/" && python3 -m pytest -sv .
t: test

# Run the Python application
.PHONY: run-app r
run-app:
	cd "$(APP_DIR)" && python3 "app.py" ./samples/samples.xml ./samples \
		-o "./out" -n "Red Maze" "Trick Knot" "3Bricks" -t 3 -c
r: run-app
