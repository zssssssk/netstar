CXX = g++
CXXFLAGS = -g -std=c++17  -fsanitize=address
LDFLAGS =  -lpthread -fsanitize=address

testserver: testserver.cc
	$(CXX) $(CXXFLAGS) -o testserver testserver.cc -lnetstar $(LDFLAGS)

clean:
	rm -f testserver
