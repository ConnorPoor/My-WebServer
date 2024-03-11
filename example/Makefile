# 更改成自己项目的根目录
PROJECT_PATH=/home/ubuntu/project/My-Muduo
HEADER_PATH=-I ${PROJECT_PATH}/include/base \
			-I ${PROJECT_PATH}/include/net \
			-I ${PROJECT_PATH}/include/net/eventloop \
			-I ${PROJECT_PATH}/include/net/poller\
			-I ${PROJECT_PATH}/include/net/tcp \
			-I ${PROJECT_PATH}/include/net/timer \
			-I ${PROJECT_PATH}/include/logger \
			

LIB_PATH=-lMy-Muduo -lpthread

CFLAGS= -g -Wall ${LIB_PATH} ${HEADER_PATH}

all: EchoServer HttpServerTest

EchoServer: EchoServer.cpp
	g++ EchoServer.cpp ${CFLAGS} -L ${PROJECT_PATH}/lib -o EchoServer 

HttpServerTest:
	g++ HttpServerTest.cpp ${CFLAGS} -L ${PROJECT_PATH}/lib -o HttpServerTest

clean:
	rm -r EchoServer HttpServerTest