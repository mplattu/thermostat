RelayController::RelayController() {
    strcpy(this->digestAuthParameterRealm, "realm=\"");
    strcpy(this->digestAuthParameterNonce, "nonce=\"");
}

void RelayController::setData(char *relayUrlP, char *authUsernameP, char *authPasswordP) {
    strcpy(this->relayUrl, relayUrlP);
    strcpy(this->relayUrlOn, relayUrlP);
    strcpy(this->relayUrlOff, relayUrlP);

    char * suffixOn = "turn_on";
    char * suffixOff = "turn_off";

    strcat(this->relayUrlOn, suffixOn);
    strcat(this->relayUrlOff, suffixOff);

    strcpy(this->authUsername, authUsernameP);
    strcpy(this->authPassword, authPasswordP);

    strcpy(this->relayAuthPath, this->extractPathFromUrl(relayUrlP));
}

bool RelayController::begin() {
    randomSeed(RANDOM_REG32);
    return true;
}

bool RelayController::on() {
    return this->httpPost(this->relayUrlOn);
}

bool RelayController::off() {
    return this->httpPost(this->relayUrlOff);
}

bool RelayController::httpPost(char * url) {
    WiFiClient client;
    HTTPClient http;

    if (WiFi.status() != WL_CONNECTED) {
        Serial.print("[Not connected]");
        return false;
    }

    http.begin(client, this->relayUrl);

    const char* keys[] = { "WWW-Authenticate" };
    http.collectHeaders(keys, 1);

    int httpCodeHandshake = http.POST("");

    if (httpCodeHandshake > 0) {
        String authReq = http.header("WWW-Authenticate");
        char * authorization = new char[1024];
        authorization = this->getDigestAuth(authReq.c_str(), this->authUsername, this->authPassword, this->relayAuthPath, 1);
        http.end();
        http.begin(this->wifiClient, url);

        http.addHeader("Authorization", authorization);

        int httpCodeProfit = http.POST("");

        if (httpCodeProfit != 200) {
            Serial.print("[HTTP POST (profit) failed: ");
            Serial.print(httpCodeProfit);
            Serial.print("] ");
            return false;
        }
    }
    else {
        Serial.print("[HTTP POST (handshake) failed: ");
        Serial.print(httpCodeHandshake);
        Serial.print("] ");
        return false;
    }

    http.end();

    return true;
}

int RelayController::indexOfChar(const char *haystack, char needle, int startPos) {
    int i = startPos;
    while (haystack[i]) {
        if (haystack[i] == needle) {
            return i;
        }
        i++;
    }
    return -1;
}

int RelayController::indexOfString(const char *haystack, char *needle) {
    char *result = strstr(haystack, needle);
    int position = result - haystack;

    return position;
}

char * RelayController::subString(const char *parentStr, int startPos, int endPos) {
    char * subStr = new char [endPos - startPos + 1];

    int currentPos = 0;
    while ((startPos + currentPos) < endPos) {
        subStr[currentPos] = parentStr[startPos + currentPos];
        currentPos++;
    }
    subStr[currentPos] = '\0';
    
    return subStr;
}

// Taken from ESP8266HTTPClient examples

char * RelayController::extractParam(const char *authReq, char *param, const char delimit) {
    int _begin = this->indexOfString(authReq, param);
    if (_begin == -1) { return ""; }

    const int paramLength = strlen(param);
    const int delimitPosition = this->indexOfChar(authReq, delimit, _begin + paramLength);
    char * substr = this->subString(authReq, _begin + paramLength, delimitPosition);

    return substr;
}

char * RelayController::extractPathFromUrl(char *url) {
    char * path = new char[strlen(url)];

    int slashPos = this->indexOfChar(url, '/', 7);
    path = this->subString(url, slashPos, strlen(url));

    return path;
}

const char * RelayController::getCNonce(const int len) {
    static const char alphanum[] = "0123456789"
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz";
    
    char * cNonce = new char[len + 1];
    
    for (int i = 0; i < len; ++i) {
        cNonce[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    cNonce[len] = '\0';

    return cNonce;
}

char * RelayController::getDigestAuth(const char *authReq, char *username, char *password, char *uri, unsigned int counter) {
    // extracting required parameters for RFC 2069 simpler Digest
    char * realm = this->extractParam(authReq, this->digestAuthParameterRealm, '"');
    char * nonce = this->extractParam(authReq, this->digestAuthParameterNonce, '"');
    const char * cNonce = this->getCNonce(8);

    char nc[9];
    snprintf(nc, sizeof(nc), "%08x", counter);

    // parameters for the RFC 2617 newer Digest
    MD5Builder md5;
    md5.begin();
    md5.add(String(username) + ":" + String(realm) + ":" + String(password));
    md5.calculate();
    String h1 = md5.toString();

    char methodUriString[TEMP_RELAY_CONTROLLER_STRING_MAXLENGTH] = "POST"; // NB! HTTP method POST is hard-coded here
    strcat(methodUriString, ":");
    strcat(methodUriString, uri);

    md5.begin();
    md5.add(methodUriString);   
    md5.calculate();
    String h2 = md5.toString();

    md5.begin();
    md5.add(h1 + ":" + String(nonce) + ":" + String(nc) + ":" + String(cNonce) + ":" + "auth" + ":" + h2);
    md5.calculate();
    String response = md5.toString();

    String authorization = "Digest username=\"" + String(username) + "\", realm=\"" + realm + "\", nonce=\"" + nonce + "\", uri=\"" + uri + "\", algorithm=\"MD5\", qop=auth, nc=" + String(nc) + ", cnonce=\"" + cNonce + "\", response=\"" + response + "\"";

    char * authorizationCStr = new char[authorization.length()];
    strcpy(authorizationCStr, authorization.c_str());

    return authorizationCStr;
}
