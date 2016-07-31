vpath %.hpp ./
vpath %.cpp ./

TARGET=bin/mydump

SRC=$(wildcard *.cpp)
OBJS=$(SRC:.cpp=.o)
OBJDIR=./objs/

all:${TARGET}

${TARGET}:$(OBJS)
	[ ! -e bin ] && mkdir bin
	g++ $(addprefix ${OBJDIR},$?) -o $@ -g

$(OBJS):%.o:%.cpp
	[ ! -e $(dir $(addprefix ${OBJDIR}/,$^)) ] && mkdir -p $(dir $(addprefix ${OBJDIR},$^));g++ -c $? -o $(addprefix ${OBJDIR},$@ ) -g

.PHONY:clean
clean:
	rm -rf ${TARGET} $(addprefix ./${OBJDIR}/,${OBJS})
