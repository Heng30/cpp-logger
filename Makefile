#!/bin/sh

CC := gcc
CXX := g++
PROG_NAME := logger
PROG_VERSION := 1.0.0
TAR_TIME = $(shell date +"%Y-%M-%d-%H-%M-%S")
KERNEL_INFO = $(shell uname -snrvm)

EXE := ./clib_test
COVER_FLAGS := -fprofile-arcs -ftest-coverage -fPIC
COVER_PROF := cover.html
COVER := gcovr -r . -b -p --html --html-details --output=$(COVER_PROF)

UFTRACE_FLAGS := -pg
TRACE_FUNC := main
UFTRACE_RECORD := uftrace record --buffer=1M $(EXE)
UFTRACE_GRAPH := uftrace graph $(TRACE_FUNC)
UFTRACE_REPORT := uftrace report
UFTRACE_REPLAY := uftrace replay
UFTRACE_INFO := uftrace_info

MEM_CHECK := valgrind
	# --leak-check=full \
	# --show-leak-kinds=all \
	# --track-origins=yes \
	# --verbose \
	# --log-file=valgrind-out.txt \

MACRO := -DPROG_VERSION=\"$(PROG_VERSION)\" \
		 -DKERNEL_INFO=\""$(KERNEL_INFO)"\"	\
		 -D_TESTING \
		 -D_BENCHMARKING \
		 -DENABLE_ICONV \

LDFLAG := -lpthread
#CFLAGS := -g -std=c++11 -W -Wall -O0 $(COVER_FLAGS) $(UFTRACE_FLAGS) $(MACRO)
CFLAGS := -g -std=c++11 -W -Wall -O0 $(MACRO)
LIBPATH :=-L/usr/local/lib -L/usr/lib/

#SRCS的值就是当前面目录下面所有的以.cpp结尾的源文件
SRCS = $(wildcard *.cpp)

#SRCS目录下创建cpp文件相对的o文件
OBJS = $(patsubst %cpp,%o,$(SRCS))


all : $(PROG_NAME)

$(PROG_NAME) : $(OBJS)
	$(CXX) $(CFLAGS) $(LIBPATH) $^ -o $@ $(LDFLAG)

# 编译时无法加入$(CFLAGS)参数
# .o .cpp :
#	$(CXX) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

mem_check:
	$(MEM_CHECK) $(EXE) --nobenchmark

cover:
	$(COVER)

trace_record:
	$(UFTRACE_RECORD)

trace_report:
	$(UFTRACE_REPORT)

trace_replay:
	$(UFTRACE_REPLAY)

trace_graph:
	$(UFTRACE_GRAPH)

trace_info:
	$(UFTRACE_INFO)

.PHONY: clean
clean:
	-rm -f $(PROG_NAME) *.o *cover*.html *.gcda *.gcno gmon.out valgrind-out.txt gmon.out
	-rm -rf uftrace.data*

.PHONY: tar
tar: clean
	@echo tar...
	-tar -zcvf $(PROG_VERSION)-$(TAR_TIME).tar.gz *
	@echo make $(PROG_VERSION)-$(TAR_TIME).tar.gz successfully!
	@echo
