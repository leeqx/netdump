vpath %.hpp ./
vpath %.cpp ./

TARGET=bin/mydump

SRC=$(wildcard *.cpp)
OBJS=$(SRC:.cpp=.o)

all:${TARGET}

${TARGET}:$(OBJS)
	g++ $? -o $@

$(OBJS):%.o:%.cpp
	g++ -c $? -o $@

.PHONY:clean
clean:
	rm -rf ${TARGET} ${OBJS}
