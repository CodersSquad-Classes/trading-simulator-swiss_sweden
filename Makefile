CXX = g++
CXXFLAGS = -std=c++17 -O2 -pthread

TARGET = clob
SRC = main.cpp
HDR = orderbook.h

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
