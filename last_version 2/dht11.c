#include "dht11.h"  
DHT11::DHT11()
{
    MAX_TIME=85;
    DHT11PIN=7;
    ATTEMPTS=180;
	temperature=0;
	humidity=0;
	flag=0;
    for(int i=0;i<5;i++)
        dht11_val[i]=0;
}
int DHT11::handel_val()
{
    uint8_t lststate=HIGH;         //last state
    uint8_t counter=0;
    uint8_t j=0,i;
    for(i=0;i<5;i++)
        dht11_val[i]=0;
         
    //host send start signal    
    pinMode(DHT11PIN,OUTPUT);      //set pin to output 
    digitalWrite(DHT11PIN,LOW);    //set to low at least 18ms 
    delay(18);
    digitalWrite(DHT11PIN,HIGH);   //set to high 20-40us
    delayMicroseconds(40);
     
    //start recieve dht response
    pinMode(DHT11PIN,INPUT);       //set pin to input
    for(i=0;i<MAX_TIME;i++)         
    {
        counter=0;
        while(digitalRead(DHT11PIN)==lststate){     //read pin state to see if dht responsed. if dht always high for 255 + 1 times, break this while circle
            counter++;
            delayMicroseconds(1);
            if(counter==255)
                break;
        }
        lststate=digitalRead(DHT11PIN);             //read current state and store as last state. 
        if(counter==255)                            //if dht always high for 255 + 1 times, break this for circle
            break;
        // top 3 transistions are ignored, maybe aim to wait for dht finish response signal
        if((i>=4)&&(i%2==0)){
            dht11_val[j/8]<<=1;                     //write 1 bit to 0 by moving left (auto add 0)
            if(counter>16)                          //long mean 1
                dht11_val[j/8]|=1;                  //write 1 bit to 1 
            j++;
        }
    }
    // verify checksum and print the verified data
    if((j>=40)&&(dht11_val[4]==((dht11_val[0]+dht11_val[1]+dht11_val[2]+dht11_val[3])& 0xFF))){
        temperature=dht11_val[2];
        humidity=dht11_val[0];
        return 1;
    }
    else
        return 0;
}
  
data DHT11::dht11_read(){
    int attempts=ATTEMPTS;
    if(!flag&&wiringPiSetup()==-1)
        exit(1);
	else if(flag==0)
		flag=1;
    while(attempts){                        //you have 5 times to retry
        int success = handel_val();     //get result including printing out
        if (success) {                      //if get result, quit program; if not, retry 5 times then quit
            break;
        }
        attempts--;
        delay(5);
    }
    return data(temperature,humidity);
}
