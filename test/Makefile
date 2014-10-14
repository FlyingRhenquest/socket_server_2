OBJS=run_tests.o test_client_server.o test_fdes_stream.o
# Some of the tests depend on my time library for string converts.
# Set SANDBOX_HOME to where you clone that archive to
SANDBOX_HOME=${HOME}/sandbox
EXE=run_cppunit

CPPFLAGS += -ggdb -std=c++11 -I.. -I${SANDBOX_HOME} -pthread
LFLAGS = -lpthread -lcppunit -lboost_thread -lboost_system

.cpp.o:
	g++ -c ${CPPFLAGS} $<

all: ${OBJS}
	g++ -o ${EXE} -pthread ${OBJS}  ${LFLAGS}

clean:
	rm -f *~ ${EXE} ${OBJS} core blarg.txt
