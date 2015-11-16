BUILD_DIR=build
APP_NAME=aggregator

GXX=g++
GXXFLAGS=-std=c++11

LIBRARIES=-lnetfilter_queue -lnfnetlink -lpthread
SOURCES=$(wildcard src/*.cc)
HEADERS=$(wildcard src/*.hh)
TARGET=$(addprefix $(BUILD_DIR)/, $(APP_NAME))

DOXYGEN=doxygen
DOXYGEN_DIR=doxygen
DOXYFILE=Doxyfile

.PHONY: default all clean doxygen

default: $(TARGET)

all: $(TARGET) doxygen

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(BUILD_DIR) $(SOURCES) $(HEADERS)
	$(GXX) $(GXXFLAGS) -o $(TARGET) $(SOURCES) $(LIBRARIES) 

doxygen: $(SOURCES) $(HEADERS)
	$(DOXYGEN) $(DOXYFILE)

clean:
	rm -rf $(BUILD_DIR) $(DOXYGEN_DIR)
