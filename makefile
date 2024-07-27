TARGETS = Server Client

SRCS1 = Server.cpp
SRCS2 = Client.cpp

OBJS1 = $(SRCS1:.cpp =.o)
OBJS2 = $(SRCS2:.cpp =.o)

all: $(TARGETS)

Server: $(OBJS1)
	g++ -o $@ $(OBJS1)

Client: $(OBJS2)
	g++ -o $@ $(OBJS2)

%.o: %.cpp
	g++ -c $<

clean:
	rm -f $(TARGETS)