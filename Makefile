
# Configuration
CXX      	:= g++
CXXFLAGS 	:= -Wall -Wextra -O2 `sdl2-config --cflags`
LDFLAGS  	:= `sdl2-config --libs`
TARGET   	:= demo
BUILD_DIR 	:= build

PROJ_DIR 	:= ./Nomad3DTest_Linux
NOMAD3D_DIR	:= ./source
CXXFLAGS 	+= -I$(PROJ_DIR)/include \
			   -I$(NOMAD3D_DIR)

EXTRA_CXXFLAGS	:= -fno-strict-aliasing
CXXFLAGS 	+= $(EXTRA_CXXFLAGS)

#---------------------------------------------------------------------------------
# Source files
#---------------------------------------------------------------------------------
SRCS := $(wildcard $(NOMAD3D_DIR)/*.cpp) $(wildcard $(PROJ_DIR)/source/*.cpp)

#---------------------------------------------------------------------------------
# Object files
#---------------------------------------------------------------------------------
OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Default task
all: $(TARGET)

# Rule to link
$(TARGET): $(OBJS)
	@echo "Linking $@"
	$(CXX) $(OBJS) -o demo/$@ $(LDFLAGS)

# Rule to compile
$(BUILD_DIR)/%.o: %.cpp
	@# Create the subfolder (e.g., build/source or build/linux) if it doesn't exist
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)
	rm demo/log.html

# Rebuild
rebuild:
	$(MAKE) clean
	$(MAKE) 
