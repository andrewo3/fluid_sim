# ------------------------
# Config
# ------------------------
TARGET := main

CXX := g++
CC := gcc
CXXFLAGS := -Wall -Wextra -std=c++17
CFLAGS := -Wall -Wextra -std=c11

# Directories (relative to Makefile)
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIRS := include
LIB_DIRS := lib
LIBS := glew32 opengl32 glu32 gdi32 user32 ws2_32

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
    SRCS_CPP := $(call rwildcard,$(SRC_DIR)/,*.cpp)
    SRCS_C   := $(call rwildcard,$(SRC_DIR)/,*.c)
else
    EXE := $(TARGET)
    MKDIR = mkdir -p $(1)
    RM = rm -f
    RMDIR = rm -rf
    SRCS_CPP := $(shell find $(SRC_DIR) -name '*.cpp')
    SRCS_C   := $(shell find $(SRC_DIR) -name '*.c')
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
OBJS_CPP := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS_CPP))
OBJS_CPP := $(patsubst $(SRC_DIR)\%.cpp,$(BUILD_DIR)/%.o,$(OBJS_CPP))  # Windows paths
OBJS_C   := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS_C))
OBJS_C   := $(patsubst $(SRC_DIR)\%.c,$(BUILD_DIR)/%.o,$(OBJS_C))      # Windows paths
OBJS := $(OBJS_CPP) $(OBJS_C)

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

# Compile C++ object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c "$<" -o "$@"

# Compile C object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(call MKDIR,$(dir $@))
	$(CC) $(CFLAGS) $(INCFLAGS) -c "$<" -o "$@"

# Windows backslash paths
$(BUILD_DIR)/%.o: $(SRC_DIR)\%.cpp
	$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c "$<" -o "$@"

$(BUILD_DIR)/%.o: $(SRC_DIR)\%.c
	$(call MKDIR,$(dir $@))
	$(CC) $(CFLAGS) $(INCFLAGS) -c "$<" -o "$@"

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
