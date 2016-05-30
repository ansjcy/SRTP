#ifndef DHT11_H_
#define DHT11_H_
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef struct data{
	unsigned char temperature;
	unsigned char humidity;
	data(int t,int h)
	{
		temperature=(unsigned char)t;
		humidity=(unsigned char)h;	
	}
}data;

class DHT11{
private:
	int MAX_TIME;
	int DHT11PIN;
	int ATTEMPTS;
	int dht11_val[5];
	int temperature;
	int humidity;
	int flag; //determine whether open /proc/cpuinfo
public:
	DHT11();
	int handel_val(); 
	data dht11_read(); //use this function to get the data
};

#endif
