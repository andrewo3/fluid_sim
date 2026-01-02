# ------------------------
# Config
# ------------------------
TARGET := main

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -mwindows

# Directories (relative to Makefile)
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIRS := include
LIB_DIRS := lib
LIBS := SDL3 glew32 opengl32 glu32

SHADERS_SRC := $(SRC_DIR)/shaders
SHADERS_DST := $(BIN_DIR)/shaders

# ------------------------
# OS Detection
# ------------------------
ifeq ($(OS),Windows_NT)
    EXE := $(TARGET).exe
    MKDIR = $(shell if not exist "$(1)" mkdir "$(1)")
    RM = del /Q /F /S
    RMDIR = rmdir /S /Q
    # Recursive wildcard for Windows
    rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
    SRCS := $(call rwildcard,$(SRC_DIR)/,*.cpp)
else
    EXE := $(TARGET)
    MKDIR = mkdir -p $(1)
    RM = rm -f
    RMDIR = rm -rf
    SRCS := $(wildcard $(SRC_DIR)/**/*.cpp)
endif

ifeq ($(OS),Windows_NT)
    COPYDIR = robocopy
else
    COPYDIR = cp -r
endif

# ------------------------
# Objects
# ------------------------
# Convert source paths to object paths in build/
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
OBJS := $(patsubst $(SRC_DIR)\%.cpp,$(BUILD_DIR)/%.o,$(OBJS))  # Windows paths

# ------------------------
# Flags
# ------------------------
INCFLAGS := $(addprefix -I,$(INCLUDE_DIRS))
LDFLAGS := $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

# ------------------------
# Targets
# ------------------------
all: build

# Build executable
build: $(BIN_DIR)/$(EXE)

# Link object files
$(BIN_DIR)/$(EXE): $(OBJS) copy_shaders
	$(call MKDIR,$(BIN_DIR))
	$(CXX) $(CXXFLAGS) $(OBJS) -o "$@" $(LDFLAGS)

# Compile object files (Linux/macOS)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c "$<" -o "$@"

# Compile object files (Windows)
$(BUILD_DIR)/%.o: $(SRC_DIR)\%.cpp
	$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c "$<" -o "$@"

# Copy shaders to bin/
copy_shaders:
	$(call MKDIR,$(BIN_DIR))
	@echo Copying shaders to $(SHADERS_DST)
ifeq ($(OS),Windows_NT)
	@robocopy "$(SHADERS_SRC)" "$(SHADERS_DST)" /E /NFL /NDL /NJH /NJS /NC /NS >NUL || exit 0
else
	cp -r "$(SHADERS_SRC)" "$(SHADERS_DST)"
endif

# Run the program
run: build
	"$(BIN_DIR)/$(EXE)"

# Clean build files (cross-platform)
clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
	@if exist "$(BIN_DIR)" ( \
		for %%f in ("$(BIN_DIR)\*") do ( \
			if /I not "%%~nxf"=="SDL3.dll" if /I not "%%~nxf"=="glew32.dll" del /Q "%%f" \
		) \
	)
else
	rm -rf "$(BUILD_DIR)"
	find "$(BIN_DIR)" -type f ! -name "SDL3.dll" ! -name "glew32.dll" -delete
endif
