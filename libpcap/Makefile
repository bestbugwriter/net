CC = gcc
FLAGS = -Wall -lpcap
OBJS = pcap.o callback.o filter.o

pcap : $(OBJS)
	cc -o pcap $(OBJS) $(FLAGS)

callback.c : callback.h
filter.c : filter.h 
pcap.c : filter.h callback.h

.PHONY : clean
clean :
	rm $(OBJS) pcap

