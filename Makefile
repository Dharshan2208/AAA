CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Wpedantic -O2 -Iinclude

TARGET := build/optimal_caching
OBJ_DIR := build/obj

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

# If the first goal is "run", we treat everything else as arguments
ifeq ($(firstword $(MAKECMDGOALS)),run)
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  ifneq ($(RUN_ARGS),)
    $(RUN_ARGS):
		@:
  endif
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	@./$(TARGET) $(RUN_ARGS)

clean:
	rm -rf build

-include $(DEPS)
