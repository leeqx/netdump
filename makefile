vpath %.hpp ./
vpath %.cpp ./

TARGET=bin/mydump

SRC=$(wildcard *.cpp)
OBJS=$(SRC: .cpp:=.o)

all:${TARGET}

${TARGET}:$(OBJS)
	g++ $? -o $@

$(OBJS):%.o:%.cpp
	g++ $? -o $@

.PHONY:clean
clean:
	echo ${TARGET} ${OBJS}
