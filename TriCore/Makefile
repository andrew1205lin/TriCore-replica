EXE=tc

CFLAGS= -O3 -Wall -fopenmp
COMMFLAGS= -O3 --compiler-options -Wall -Xptxas -v -lpthread
CUCC= "$(shell which nvcc)"
CC= "$(shell which g++)"

CUFLAGS= -arch=sm_86  ${COMMFLAGS}#-Xptxas -dlcm=cg#disable l1 cache
CUFLAGS+= -ccbin=g++ -Xcompiler -fopenmp
CUFLAGS+= -O0 -G -g -lpthread



OBJS=  	main.o \
		graph.o\
		scan.o\
		wtime.o\

DEPS= 	Makefile \
		comm.h \
		gputimer.h \

%.o:%.c $(DEPS)
	${CC} -c  ${CFLAGS} $< -o $@
%.o:%.cu $(DEPS)
	${CUCC} -c  ${CUFLAGS} $< -o $@

${EXE}:${OBJS}
	${CUCC} ${OBJS} $(CUFLAGS) -o ${EXE}

clean:
	rm -rf *.o ${EXE}
