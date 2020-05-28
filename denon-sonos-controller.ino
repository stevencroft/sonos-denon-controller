//#include "application.h"
#include "HttpClient.h"

/**
* Declaring the variables.
*/
unsigned int nextTime = 0;    // Next time to contact the server
HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers with NULL
};

http_request_t denon_request;
http_response_t denon_response;

//TCPClient client; //for IFTTT

TCPClient sonos;
byte sonosIP[] = { 192, 168, 1, 149 }; //put your Sonos_IP_address
String denonIP = "192.168.1.140";
String sonosResponse;
String sonosTransportState = "";
String previousState;

unsigned long resetTimer = millis();


void setup() {
    Serial.begin(9600);
    
    Serial.println();
    Serial.println("Application>\tStart of Loop.");
    // Request path and body can be set at runtime or at setup.
    denon_request.hostname = denonIP; //Denon IP
    denon_request.port = 80;
    denon_request.path = "/MainZone/index.put.asp?cmd0=PutMasterVolumeBtn/>";

    // The library also supports sending a body with your request:
    //request.body = "{\"key\":\"value\"}";

    // Get request
    http.get(denon_request, denon_response, headers);
    Serial.print("Application>\tResponse status: ");
    Serial.println(denon_response.status);

    Serial.print("Application>\tHTTP Response Body: ");
    Serial.println(denon_response.body);
    
    //Set initial state 
    getSonosStatus();
    sonosTransportState = tryExtractString(readSonosResponse(),"<CurrentTransportState>","</CurrentTransportState>");
    previousState = sonosTransportState;
    Particle.publish("sonosTransportState", sonosTransportState);
}


void getSonosStatus()
/*
POST http://Sonos_IP_address:1400/MediaRenderer/AVTransport/Control HTTP/1.1
HOST: Sonos_IP_address:1400
SOAPACTION: "urn:schemas-upnp-org:service:AVTransport:1#GetTransportInfo"
CONTENT-TYPE: text/xml; charset="utf-8"
Content-Length: 354

<?xml version="1.0" encoding="utf-8"?>
<s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
   <s:Body>
      <u:GetTransportInfo xmlns:u="urn:schemas-upnp-org:service:AVTransport:1">
      <InstanceID>0</InstanceID>
      </u:GetTransportInfo>
   </s:Body>
</s:Envelope>
*/
{
    if (sonos.connect(sonosIP, 1400))
    {
        String contentSOAP ="<?xml version=\"1.0\" encoding=\"utf-8\"?>";
        contentSOAP += "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">";
        contentSOAP += "<s:Body>";
        contentSOAP += "<u:GetTransportInfo xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">";
        contentSOAP += "<InstanceID>0</InstanceID>";
        contentSOAP += "</u:GetTransportInfo>";
        contentSOAP += "</s:Body>";
        contentSOAP += "</s:Envelope>";
        
        String post = "POST http://";
        post += sonosText();
        post += ":1400/MediaRenderer/AVTransport/Control HTTP/1.1\r\n";
        post += "HOST: ";
        post += sonosText();
        post += ":1400\r\n";
        post += "SOAPACTION: \"urn:schemas-upnp-org:service:AVTransport:1#GetTransportInfo\"\r\n";
        post += "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n";
        post += "Content-Length: ";
        post += contentSOAP.length();
        post += "\r\n\r\n";
        post += contentSOAP;
        post += "\r\n\r\n";

        //Serial.println(post);
        sonos.print(post);

        delay(10);
    }
    else Serial.println("connection failed");
}

String readSonosResponse() {
    unsigned long lastdata = millis();
    String sonosResponse = "";
    while (sonos.connected()  || (millis()-lastdata < 500)) { //500ms timeout
        if (sonos.available()) {
            char c = sonos.read();
            sonosResponse += c;
            lastdata = millis();
        }
    }
    sonos.flush();
    sonos.stop();
    return sonosResponse;
}

String tryExtractString(String str, const char* start, const char* end) {
    if (str == NULL) {
        return "";
    }

    int idx = str.indexOf(start);
    if (idx < 0) {
        return "";
    }

    int endIdx = str.indexOf(end);
    if (endIdx < 0) {
        return "";
    }

    return str.substring(idx + strlen(start), endIdx);
}

String sonosText(){
    String addressIP = "";
    for (int i = 0; i < 4; i++) {
        addressIP += sonosIP[i];
        if (i < 3) {
            addressIP += ".";
        }
    }
    return addressIP;
}

void sendDenonCommand(String path) {
    String denonResponseCode;
    if (path != NULL) {
        denon_request.path = path;
        http.get(denon_request, denon_response, headers);
        Serial.print("Application>\tResponse status: ");
        Serial.println(denon_response.status);
        denonResponseCode = String::format("%d", denon_response.status);
        Particle.publish("Denon Response", denonResponseCode);    
    }
    
}

void loop() {
    //if (nextTime > millis()) {
    //    return;
    //}
    //nextTime = millis() + 10000;
    getSonosStatus();
    sonosTransportState = tryExtractString(readSonosResponse(),"<CurrentTransportState>","</CurrentTransportState>");
    //String denonResponseCode;
    if (previousState != sonosTransportState) {
        Particle.publish("sonosTransportState", sonosTransportState);
        Particle.publish("sonosPreviousState", previousState);   
        //Serial.println(sonosTransportState);
        //Particle.publish(sonosTransportState);
        if (sonosTransportState == "PLAYING") {
            //denon_request.path = "/MainZone/index.put.asp?cmd0=PutZone_OnOff%2FON";
            //http.get(denon_request, denon_response, headers);
            //Serial.print("Application>\tResponse status: ");
            //Serial.println(denon_response.status);
            //denonResponseCode = String::format("%d", denon_response.status);
            //Particle.publish("Denon PWR ON response", denonResponseCode);
            sendDenonCommand("/MainZone/index.put.asp?cmd0=PutZone_OnOff%2FON");
            delay(3000);
            //Wait for receiver to turn on and then select DVD input
            //denon_request.path = "/MainZone/index.put.asp?cmd0=PutZone_InputFunction%2FCD";
            //http.get(denon_request, denon_response, headers);
            //Serial.print("Application>\tResponse status: ");
            //Serial.println(denon_response.status);
            //denonResponseCode = String::format("%d", denon_response.status);
            //Particle.publish("Denon set DVD input response", denonResponseCode);
            sendDenonCommand("/MainZone/index.put.asp?cmd0=PutZone_InputFunction%2FCD");
            delay(50);
            //Set volume to 35 (-45=35-80)
            //denon_request.path = "/MainZone/index.put.asp?cmd0=PutMasterVolumeSet/-45.0";
            //http.get(denon_request, denon_response, headers);
            //Serial.print("Application>\tResponse status: ");
            //Serial.println(denon_response.status);
            //denonResponseCode = String::format("%d", denon_response.status);
            //Particle.publish("Denon set volume response", denonResponseCode);
            sendDenonCommand("/MainZone/index.put.asp?cmd0=PutMasterVolumeSet/-45.0");
            
        }
        else if (sonosTransportState == "PAUSED_PLAYBACK" || sonosTransportState == "STOPPED") {
            //denon_request.path = "/MainZone/index.put.asp?cmd0=PutMasterVolumeBtn/<";
            //denon_request.path = "/MainZone/index.put.asp?cmd0=PutSystem_OnStandby%2FSTANDBY";
            //http.get(denon_request, denon_response, headers);
            //Serial.print("Application>\tResponse status: ");
            //Serial.println(denon_response.status);
            //denonResponseCode = String::format("%d", denon_response.status);
            //Particle.publish("Denon PWR OFF response", denonResponseCode);
            sendDenonCommand("/MainZone/index.put.asp?cmd0=PutSystem_OnStandby%2FSTANDBY");
        }
        previousState = sonosTransportState;
    }
    delay(1000);
    
    // Reset after 7h of operation
    // ==================================
    if (millis() - resetTimer > 25200000) {
        System.reset();
    }
}

//Command list
// -- Turn the Main Zone On
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutZone_OnOff%2FON")

// -- Turn the Main Zone Off
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutZone_OnOff%2FOFF")

// -- Put the Main Zone in Standby
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutSystem_OnStandby%2FSTANDBY")

// -- Volume up:
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutMasterVolumeBtn/>")

// -- Volume down:
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutMasterVolumeBtn/<")

// -- Set volume to 35 (-45=35-80):
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutMasterVolumeSet/-45.0")

// -- Volume mute (on/off):
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutVolumeMute/on")
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutVolumeMute/off")

// -- Inputs (Sat/Cable):
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutZone_InputFunction%2FSAT%2FCBL")

// -- Inputs (DVD):
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutZone_InputFunction%2FDVD")

// -- Inputs (Media Player):
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutZone_InputFunction%2FMPLAY")

// -- Inputs (Phono):
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutZone_InputFunction%2FPHONO")

// -- Inputs (AUX 1):
// socket.http.request("http://your_receiver_address/MainZone/index.put.asp?cmd0=PutZone_InputFunction%2FAUX1")

// -- DRC:
// socket.http.request("http://your_receiver_address/SETUP/AUDIO/s_surrpara_1.asp?listDynamicComp=Mid")