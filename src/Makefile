BUILD_DIR=../build
TESTS_DIR=tests
APP_NAME=aggregator
LIBRARIES=-lnetfilter_queue -lnfnetlink -lpthread
SOURCES=$(wildcard ./*.cc)
HEADERS=$(wildcard ./*.hh)
TEST_SOURCES=$(wildcard $(TESTS_DIR)/*.cc)

TARGET=$(addprefix $(BUILD_DIR)/, $(APP_NAME))
TEST_TARGETS=$(patsubst $(TESTS_DIR)/%.cc, $(BUILD_DIR)/%, $(TEST_SOURCES))

#GXX=g++
GXX=clang++
GXXFLAGS=-std=c++11

.PHONY: default all clean tests

default: all

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) $(HEADERS)
	$(GXX) $(GXXFLAGS) $(LIBRARIES) -o $(TARGET) $(SOURCES)

tests: $(BUILD_DIR) $(TEST_TARGETS)

#$(TEST_TARGETS): $(TEST_SOURCES)
$(TEST_TARGETS): $(BUILD_DIR)/%: $(TESTS_DIR)/%.cc $(SOURCES) $(HEADERS)
	$(GXX) $(GXXFLAGS) $(LIBRARIES) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
