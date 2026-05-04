CXX = g++
CXXFLAGS = -std=c++20 -lz -lavformat -lavcodec -lavutil -lswscale -lswresample -lssl -lcrypto -pthread

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

app: $(OBJ)
	$(CXX) $(OBJ) -o app

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) app