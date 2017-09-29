#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//WiFi credentials
const char* ssid = "SSIDHere";
const char* password = "SecurePassword";

//Access info for your device
const String deviceID = "uniqueIDToYourNetwork";
const String deviceSecret = "secretCodeHere";

//Where to update
const String urlIPUpdate = "https://www.example.com/iot/update_ip";
const String urlNotify = "https://www.example.com/iot/notify";

//constants for gpio
int LED_OFF = HIGH;
int LED_ON = LOW;
int PIN_BUTTON_ON = 4;
int PIN_BUTTON_OFF = 5;

WiFiServer server(80);
 
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_BUTTON_OFF, INPUT); //off
  pinMode(PIN_BUTTON_ON, INPUT); //on
  digitalWrite(LED_BUILTIN, LED_OFF);
  Serial.println();

  //WiFi connection
  connectToWiFi();

  //start server
  server.begin();
  Serial.println("Server started");
}
 
void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  int dropUser = 0;
  while(!client.available()){
    delay(1);
    dropUser++;
    if(dropUser>=1000){ //drop the connection if no request in 1000ms
      return;
    }
  }

  //skip through header information
  String request = client.readStringUntil(' ');
  String gPage = client.readStringUntil(' ');
  while(request!="")
  {
    request = client.readStringUntil('\r');
    client.read(); //get the \n char (to skip it)
  }
  request = client.readString(); //post data
  if(request.length()>0){
    Serial.println("Post data: "+request);
  }
  client.flush();
 
  // Match the request
  bool gotSecret = false;
  bool doneP = false;
  while(!doneP)
  {
    int iE = request.indexOf("=");
    int iA = request.indexOf("&");
    String q = "";
    String a = "";
    
    if(iE>-1)
    {
      if(iA==-1)
      {
        iA = request.length();
        doneP = true;
      }
        q = request.substring(0,iE);
        a = request.substring(iE+1,iA);
        if(!doneP)
        {
         request = request.substring(iA+1,request.length()); 
        }
        //To add a post option, put it here in if(q=="option"), etc; secret should come first
        if(q=="secret")
        {
          if(a==deviceSecret)
          {
            gotSecret = true;
          }
        }
        else if(q=="action")
        {
          if(gotSecret==true)
          {
            //to add an action, just continue with else if(a="etc")...
            if(a=="on")
            {
              pinMode(PIN_BUTTON_ON,OUTPUT);
              digitalWrite(PIN_BUTTON_ON,LOW);
              delay(100);
              pinMode(PIN_BUTTON_ON,INPUT);
              postNotif("on");
              
            }
            else if(a=="off")
            {
              pinMode(PIN_BUTTON_OFF,OUTPUT);
              digitalWrite(PIN_BUTTON_OFF,LOW);
              delay(100);
              pinMode(PIN_BUTTON_OFF,INPUT);
              postNotif("off");
            } 
          }
        }
    }
    else
    {
      doneP = true;
    }
  }


//page routing
  if(gPage=="/test"){
    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); // End the header
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("This is a test.");
    client.println("</html>");
  }
  else if(gPage=="/gui")
  {
    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); // End the header
    client.println("<!DOCTYPE HTML>");
    client.println("<html style='height:100%;'>");
    client.println("<title>Coffee Maker</title>");
    client.println("<body style='height:98%;'>");
    client.println("<form method=\"POST\" style='height:100%;'>");
    client.println("<input type='hidden' id='secret' name='secret' value='szechuan'/>");
   // client.println("<input type='text' id='action' name='action'/>");
   // client.println("<input type='submit'/>");
    client.println("<button type='submit' name='action' value='on' style='width:100%;height:50%;font-size:500%;'>Coffee On</button>");
    client.println("<button type='submit' name='action' value='off' style='width:100%;height:50%;font-size:500%;'>Coffee Off</button>");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
  }
  else
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); // End the header
  }
 
  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");
 
}

//Misc. Functions

void blink() //blinks the LED once
{
  digitalWrite(LED_BUILTIN, LED_ON); 
  delay(100); 
  digitalWrite(LED_BUILTIN, LED_OFF);
}

void connectToWiFi() //self explanitory
{
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
    blink();
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  postIP(WiFi.localIP().toString());
}

void onDisconnected(const WiFiEventStationModeDisconnected& event)
{
    connectToWiFi();
}

void httpPost(String host,String data)
{
  HTTPClient http;
  http.begin(host);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(data);
  http.writeToStream(&Serial);
  http.end();
}

void postIP(String ip)
{
  httpPost(urlIPUpdate,"id="+deviceID+"&ip="+ip+"&secret="+deviceSecret);
}
void postNotif(String func){
  httpPost(urlNotify,"id="+deviceID+"&func="+func+"&secret="+deviceSecret);
}
