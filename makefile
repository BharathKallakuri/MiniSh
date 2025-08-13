CXX := g++
CXXFLAGS := -std=c++20 -Iexternal/ftxui/include -Wall -Wextra -pthread
LDFLAGS := -Lexternal/ftxui/build -lftxui-component -lftxui-dom -lftxui-screen

TARGET := minish
SRC := main.cpp

all: ftxui $(TARGET)

# Build your app
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Build FTXUI as a static library
ftxui:
	mkdir -p external/ftxui/build
	cd external/ftxui/build && cmake .. && make -j

clean:
	rm -f $(TARGET)
	rm -rf external/ftxui/build
