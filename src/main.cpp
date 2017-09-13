/*
 *Simple Controler for a Light via WiFi
 *
 *Created: 13.9. 2017
 * Author: BajaCali
 *
 */

#define SERIAL_BAUDRATE 115200

#define LIGHT  23
#define CONNECTED_LED  13
#define RESET_PIN 21

#define _GLIBCXX_USE_C99 1
#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include <WiFi.h>
#include <string>
#include <iostream> 

#include "credentials.h"

#include <esp_wifi.h>

system_event_cb_t oldhandler;
esp_err_t hndl(void* x, system_event_t* e) 
{
    printf("Event: %d\n",e->event_id);
    fflush(stdout);
    (*oldhandler)(x, e);
}

WiFiServer server(80);

// Client variables 
char linebuf[80];
int charcount = 0;

void reset();
void WiFi_setup();

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
	pinMode(RESET_PIN, OUTPUT);
	pinMode(LIGHT, OUTPUT);
	digitalWrite(RESET_PIN, HIGH);
	digitalWrite(LIGHT, HIGH);
    // WiFi.begin(); 
    // delay(1000);
    // Serial.println("WiFi begined");
    Serial.println("Starting...");
    WiFi_setup();
    server.begin();
}

void loop(){
	// listen for incoming clients
	WiFiClient client = server.available();
	if (client) {
		Serial.println("New client");
		memset(linebuf,0,sizeof(linebuf));
		charcount=0;
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				//read char by char HTTP request
				linebuf[charcount]=c;
				if (charcount<sizeof(linebuf)-1) charcount++;
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n'){
					if(strstr(linebuf,"GET /on_off")){
                        if(digitalRead(LIGHT) == LOW){
                            digitalWrite(LIGHT, HIGH);
                            Serial.println("on");
                        }
                        else {
                            digitalWrite(LIGHT, HIGH);
                            Serial.println("off");
                        }
				    }
				}
				if (c == '\n' && currentLineIsBlank) {
					// send a standard http response header
                    String Page;
                    Page += "\nHTTP/1.1 200 OK";
                    Page += "\nContent-Type: text/html";
                    Page += "\nConnection: close";  // the connection will be closed after completion of the response
                    Page += "\n";
                    Page += "\n<!DOCTYPE HTML><html><head>";
                    Page += "\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>";
                    Page += "\n<link rel=\"stylesheet\" type=\"text/css\" href=\"css.css\">";
                    Page += "\n<style type=\"text/css\">";
                    Page += "\nbutton {background-color: white; color: black; border: 2px solid #555555; border-radius: 4px}";
                    Page += "\nbutton:hover {background-color: #555555; color: white;border-radius: 4px}";
                    Page += "\n</style>";
                    Page += "\n<title>The Light</title>";
                    Page += "\n<p><a href=\"on_off\"><button style=\"position: absolute; left: 25%; width: 50%; height: auto\"><b>CHANGE</b></button></a></p>";
    				client.print(Page);
				}
				if (c == '\n') {
					// you're starting a new 
					currentLineIsBlank = true;
					currentLineIsBlank = true;
					memset(linebuf,0,sizeof(linebuf));
					charcount=0;
				}
				else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);

		// close the connection:
		client.stop();
		Serial.println("client disconnected");
	}
}

void WiFi_setup(){
	pinMode(CONNECTED_LED, OUTPUT);
	digitalWrite(CONNECTED_LED, 0);
	WiFi.begin(ssid, password);
	oldhandler = esp_event_loop_set_cb(hndl, nullptr);
	Serial.print("Connecting");
	int stat = WiFi.status();
	printf("\nStatus pred whilem: %d",stat);
	if(stat == 255){
		Serial.println("Restarting!!!");
		fflush(stdout);
		reset();
	}
	while (stat != WL_CONNECTED){
		printf("\nStatus: %d",stat);
		if (stat != WL_DISCONNECTED && stat != WL_CONNECTED){
			WiFi.begin(ssid, password);
			Serial.println("Reconnecting");
		}
		Serial.print(".");
		delay(500);
		stat = WiFi.status();
	}
	digitalWrite(CONNECTED_LED, 1);
	printf("\nStatus po whilu: %d",stat);
	Serial.print("\nConnected!\nIP address: ");
	Serial.print(WiFi.localIP());
}

void reset(){
	digitalWrite(RESET_PIN, LOW);
}