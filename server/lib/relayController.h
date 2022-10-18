#ifndef _TEMP_RELAY_CONTROLLER_H_
#define _TEMP_RELAY_CONTROLLER_H_

#define TEMP_RELAY_CONTROLLER_STRING_MAXLENGTH 256

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

class RelayController {
public:
    RelayController();
    void setData(char *relayUrlP, char *authUsernameP, char *authPasswordP);
    bool begin();
    bool on();
    bool off();
private:
    bool httpPost(char * url);
    int indexOfChar(const char *haystack, char needle, int startPos);
    int indexOfString(const char *haystack, char *needle);
    char * subString(const char *parentString, int startPos, int endPos);
    char * extractParam(const char *authReq, char *param, const char delimit);
    char * extractPathFromUrl(char *url);
    const char * getCNonce(const int len);
    char * getDigestAuth(const char *authReq, char *username, char *password, char *uri, unsigned int counter);
    WiFiClient wifiClient;
    HTTPClient httpClient;
    char relayUrl[TEMP_RELAY_CONTROLLER_STRING_MAXLENGTH];
    char relayUrlOn[TEMP_RELAY_CONTROLLER_STRING_MAXLENGTH];
    char relayUrlOff[TEMP_RELAY_CONTROLLER_STRING_MAXLENGTH];
    char relayAuthPath[TEMP_RELAY_CONTROLLER_STRING_MAXLENGTH];
    char authUsername[32];
    char authPassword[32];
    char digestAuthParameterRealm[8];
    char digestAuthParameterNonce[8];
};

#include "relayController.cpp"

#endif
