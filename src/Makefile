BUILD_DIR=../build
APP_NAME=aggregator
LIBRARIES=-lnetfilter_queue -lnfnetlink -lpthread
SOURCES=$(wildcard *.cc)
HEADERS=$(wildcard *.hh)

TARGET=$(addprefix $(BUILD_DIR)/, $(APP_NAME))

GXX=g++
#GXX=clang
GXXFLAGS=-std=c++11

.PHONY: default all clean

default: all

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES) $(HEADERS)
	$(GXX) $(GXXFLAGS) $(LIBRARIES) -o $(TARGET) $(SOURCES)

clean:
	rm -rf $(BUILD_DIR)
