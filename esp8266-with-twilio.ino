#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <base64.h>

//https ceritfificate from Twilio server
const char IRG_Root_X1 [] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)CERT";

//WiFi credentials details
const char* w_name = "your_ssid";
const char* w_pass  = "your_password";

//Twilio contact details 
//For SMS e.g., +1010101010
//For Whatsapp e.g., Whatsapp:1010101010 
String toPhonenumber = "to_phonenumber"; //If you want to use whatsapp, first, add Whatsapp: at the begining and dont enter the + sign followed by the phone number, eg: whatsapp:14155238886
String fromPhonenumber = "from_phonenumber"; //If you want to use whatsapp, first, add Whatsapp: at the begining and dont enter the + sign followed by the phone number, eg: whatsapp:14155238886
String message = "message";

//Twilio account credetials 
String accountSID = "your_account_sid";
String authToken= "your_auth_token";

String auth = accountSID + ":" + authToken;

//Encode the credetials 
String encoded = base64::encode(auth);

//body of the HTTP request 
String body = "Body=" + message + "&To=" + toPhonenumber + "&From=" + fromPhonenumber;

//endPoit of the HTTP request 
String url = "https://api.twilio.com/2010-04-01/Accounts/" + accountSID + "/Messages.json";

// Create a list of certificates with the server certificate
X509List cert(IRG_Root_X1);

WiFiClientSecure client;
HTTPClient http;

void setup(){

  Serial.begin(9600);

  //Start the connection to the WiFi 
  WiFi.begin(w_name, w_pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(300);
    Serial.print("Connecting");
    delay(300);
    Serial.print(".");
    delay(300);
    Serial.print(".");
    delay(300);
    Serial.print(".");
    Serial.println("");
  }

  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.println(asctime(&timeinfo));

  //Set the list of cerificates as valid
  client.setTrustAnchors(&cert);

  //Make the post request to Twilio API
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setAuthorization(encoded);
  int sendRequest = http.POST(body);

  // read the status code and body of the response
  String reponse = http.getString();

  if(sendRequest ==  HTTP_CODE_OK || HTTP_CODE_CREATED){
    Serial.println("The response was successfull!");
    Serial.println(reponse);
  }
  else{
    Serial.println("There was an error with your request");
    Serial.print("Code: ");
    Serial.println(sendRequest);
    Serial.println(reponse);
  }

  //Close the connection
  http.end();
}

void loop(){
    while (true)
    ;
}