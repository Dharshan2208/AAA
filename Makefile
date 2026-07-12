CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Wpedantic -O2 -Iinclude

TARGET := build/optimal_caching
OBJ_DIR := build/obj

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

ARGS := $(filter-out all run clean,$(MAKECMDGOALS))
FIRST_GOAL := $(firstword $(MAKECMDGOALS))

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	@./$(TARGET) $(ARGS)

clean:
	rm -rf build

%: $(TARGET)
	@if [ "$(FIRST_GOAL)" = "$@" ]; then ./$(TARGET) $(MAKECMDGOALS); fi

-include $(DEPS)
