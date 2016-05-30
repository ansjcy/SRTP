client:epollClient.o dht11.o
		g++ -o client epollClient.o dht11.o -lwiringPi -pthread
epollClient.o:epollClient.cpp dht11.h
		g++ -c epollClient.cpp -pthread
dht11.o:dht11.c dht11.h
		g++ -c dht11.c -lwiringPi 
clean:
	rm *.o
