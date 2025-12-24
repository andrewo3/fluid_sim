# ------------------------
# Config
# ------------------------
TARGET := main

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -mconsole

# Directories (relative to Makefile)
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIRS := include x86_64-w64-mingw32/include
LIB_DIRS := x86_64-w64-mingw32/lib
LIBS := SDL3   # SDL3 only

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
$(BIN_DIR)/$(EXE): $(OBJS)
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

# Run the program
run: build
	"$(BIN_DIR)/$(EXE)"

# Clean build files (cross-platform)
clean:
	$(RMDIR) "$(BUILD_DIR)" || true
	$(RMDIR) "$(BIN_DIR)" || true
