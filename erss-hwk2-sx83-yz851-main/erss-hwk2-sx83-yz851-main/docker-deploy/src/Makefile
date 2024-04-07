CXX=g++
CXXFLAGS=-g -std=gnu++11 -pthread
SOURCES=main.cpp ProxyServer.cpp Logger.cpp HttpResponse.cpp HttpRequest.cpp HttpCache.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
