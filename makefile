vpath %.hpp ./
vpath %.cpp ./

TARGET=bin/mydump

SRC=$(wildcard *.cpp)
OBJS=$(SRC:.cpp=.o)

all:${TARGET}

${TARGET}:$(OBJS)
	rm -rf bin;mkdir bin
	g++ $? -o $@ -g

$(OBJS):%.o:%.cpp
	g++ -c $? -o $@ -g

.PHONY:clean
clean:
	rm -rf ${TARGET} ${OBJS}
