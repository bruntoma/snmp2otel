CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g -O0
LDFLAGS := -lcurl -lnetsnmp

SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
BIN_DIR := bin

TARGET := $(BIN_DIR)/snpm2otel
TEST_TARGET := $(BIN_DIR)/tests

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

INCLUDES := -I$(INCLUDE_DIR)

.PHONY: all run debug release clean rebuild test

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

TEST_HDRS := $(wildcard tests/*.hpp)
TEST_SRCS := $(wildcard tests/*.cpp)

$(TEST_TARGET): tests/tests.cpp $(INCLUDE_DIR)/ArgumentParser.hpp $(INCLUDE_DIR)/Mapping.hpp $(INCLUDE_DIR)/Config.hpp $(TEST_HDRS) $(TEST_SRCS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) tests/tests.cpp $(SRC_DIR)/SnmpClient.cpp $(SRC_DIR)/HttpOtelClient.cpp -o $(TEST_TARGET) $(LDFLAGS)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

run: $(TARGET)
	@./$(TARGET) --help

debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

release: CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O3 -DNDEBUG
release: clean $(TARGET)

clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

rebuild: clean all

test: $(TEST_TARGET)
	@echo "Running tests..."
	-@./$(TEST_TARGET)  

-include $(OBJECTS:.o=.d)
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM -MT $(BUILD_DIR)/$*.o $< > $@
