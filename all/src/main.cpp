#include <mbed.h>
#include "CCS811.h"
#include "htu21d.h"
#include "stdio.h" 
#include "esp8266.h"
ESP8266 wifi(PB_6, PB_7, 115200);
Serial pc(USBRX,USBTX);
CCS811 ccs811(I2C_SDA2, I2C_SCL2);
HTU21D sensor(I2C_SDA, I2C_SCL);
AnalogIn KY37 (PA_0);
//buffers for wifi library
char resp[1000];
char http_cmd[300], comm[300];
int timeout = 8000; //timeout for wifi commands

//SSID and password for connection
#define SSID "VoNgocHoangTrinh" 
#define PASS "0934648013"  
#define IP "184.106.153.149" //Remote ip
float ECO2 = 0;
float TVOC = 0;
float value = 0;

//Update key for thingspeak
char* Update_Key = "AKJJAS0VI78REIAZ";

//Wifi init function
void wifi_initialize(void){
    
    pc.printf("******** Resetting wifi module ********\r\n");
    wifi.Reset();
    
    //wait for 5 seconds for response, else display no response receiveed
    if (wifi.RcvReply(resp, 5000))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response");
    
    pc.printf("******** Setting Station mode of wifi with AP ********\r\n");
    wifi.SetMode(1);    // set transparent  mode
    if (wifi.RcvReply(resp, timeout))    //receive a response from ESP
        pc.printf("%s",resp);    //Print the response onscreen
    else
        pc.printf("No response while setting mode. \r\n");
    
    pc.printf("******** Joining network with SSID and PASS ********\r\n");
    wifi.Join(SSID, PASS);     
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);   
    else
        pc.printf("No response while connecting to network \r\n");
        
    pc.printf("******** Getting IP and MAC of module ********\r\n");
    wifi.GetIP(resp);     
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while getting IP \r\n");
    
    pc.printf("******** Setting WIFI UART passthrough ********\r\n");
    wifi.setTransparent();          
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while setting wifi passthrough. \r\n");
    wait(1);    
    
    pc.printf("******** Setting single connection mode ********\r\n");
    wifi.SetSingle();             
    wifi.RcvReply(resp, timeout);
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while setting single connection \r\n");
    wait(1);
}
 
void wifi_send(void){
    
    pc.printf("******** Starting TCP connection on IP and port ********\r\n");
    wifi.startTCPConn(IP,80);    //cipstart
    wifi.RcvReply(resp, timeout);
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while starting TCP connection \r\n");
    wait(1);
    
    //create link 
    sprintf(http_cmd,"/update?api_key=%s&field1=%f&field2=%f&field3=%f&field4=%f&field5=%f\r\n",Update_Key,value,ECO2,TVOC,sensor.sample_ctemp(),sensor.sample_humid()); 
    pc.printf(http_cmd);
    
    pc.printf("******** Sending URL to wifi ********\r\n");
    wifi.sendURL(http_cmd, comm);   //cipsend and get command
    if (wifi.RcvReply(resp, timeout))    
        pc.printf("%s",resp);    
    else
        pc.printf("No response while sending URL \r\n");
}
void CCS811Callback(void)
{
    
    ccs811.readData(&ECO2, &TVOC);
 
    pc.printf("eCO2 reading : %f ppm, TVOC reading :%f ppb\r\n", ECO2, TVOC);
 
}

int main() {
    wifi_initialize();
    ccs811.init();
  // put your setup code here, to run once:

  while(1) {
    CCS811Callback();
    wait(1);
    pc.printf("temp reading : %d , humid reading : %d \r\n", sensor.sample_ctemp(),sensor.sample_humid() );
    wait(1);
    value = 100 * KY37.read();
    pc.printf("value : %f\r\n",value);
    wait(1);
    wifi_send();
    wait(1);
    // put your main code here, to run repeatedly:
  }
}