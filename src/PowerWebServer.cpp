/*

 */

#include "PowerWebServer.h"



void loop() 
{
  mqtt.loop();
  server.handleClient();
 
  if (millis() - lastReadingTime > AUTO_READ_DELAY || force_read) 
  {
    lastReadingTime = millis();
    String st = getData();
    mqtt.publish( topic_send ,st.c_str());
    force_read = false;
  }
#if defined (ARDUINO_ARCH_ESP8266)
  MDNS.update();
#endif

  
}


String getData() 
{
   Serial.print("Reading data, I'm on ");
   Serial.println( WiFi.localIP() );

   irms = emon1.calcIrms(1480); // measure current
   double calcPower = irms*230.0;
   
  String st;  
  st = "{";
  st += "\"irms\":" + String(irms);
  st += ",";
  st += "\"real_power\":" + String(emon1.realPower);
  st += ",";
  st += "\"power\":" + String(calcPower);
  st += "}";

  return st;
}


void setup()
{
  Serial.begin(9600);
  Serial.flush();
  Serial.println();
  Serial.println();
  Serial.println( "------------------------------------------");
  
  while( WiFi.status() != WL_CONNECTED )
  {
    // Connect to the network
    WiFi.begin( _SSID, _PASSWORD);       
    Serial.println();

    Serial.print("Connecting to "); Serial.print(_SSID); Serial.println(" ...");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED)  // Wait for the Wi-Fi to connect
    {
      delay(1000);
      Serial.print(++i); Serial.print(' ');
      if ( i > 10 )
      {         
         WiFi.disconnect();
         Serial.println();
         Serial.println("Timeout, trying again in 5 seconds...");
         delay( 5000 );      
        break;
      }
    }
  }

  Serial.println("Connected!");
  Serial.print( "My IP address is "); Serial.println( WiFi.localIP() );

  Serial.println();

  Serial.println("Starting mDns...");

#if !defined (ARDUINO_ARCH_ESP8266)
    mdns_init();
    mdns_hostname_set(HOSTNAME);
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
#else
    if (MDNS.begin( HOSTNAME ) )
    {
        MDNS.addService("tcp", "tcp", 80);
        MDNS.addService("http", "tcp", 80);
    }
#endif

     Serial.println("mDns started.");

 
    Serial.println(  );

    Serial.println( "Web server setup..." );

    server.on("/", []() {
        Serial.println( "Serving data." );
        server.send(200, "application/json", getData() );
    });
    
    
    server.onNotFound([]() {
        Serial.println( "HTTP Error!" );
        server.send(404, "text/html", "<html><head><style>body {background-color: powderblue;} h3 {color: blue;} </style> </head><body><h3>Uri not found <b>"+server.uri()+"</b></h3></body></html>");
    });
  
    server.begin();    

    Serial.print("Webserver up and running on port ");  
    Serial.println( HTTP_PORT );

    mqtt.setServer( MQTT_BROKER, MQTT_PORT  );
    mqtt.setCallback( mqttCallback );
    mqtt.setClient( espClient );

    mqttConnect();

    Serial.println( "Sensor initialization..." );
    emon1.current(ANALOG_PIN, 111.1);  //pin for current measurement, calibration value
  
    //pinMode(8, OUTPUT);   //enable UEXT power supply
    //digitalWrite(8, LOW);

    Serial.println( "Done!" );
    Serial.println( "------------------------------------------");
}


bool mqttConnect()
{  
  bool retval = false;
  
  if (!mqtt.connected())
  {
    g_mqttConnectionTimeout = millis();

    Serial.println( "Attempting MQTT connection...");

    if (mqtt.connect( "power_client" ))
    {
        Serial.println( "MQTT connected!" );
        //register to topics
        Serial.print( "Subscribing to topic " ); Serial.println( topic_recv );

        mqtt.subscribe( topic_recv );


        retval = true;
    }
    else
    {
      delay(500);
    }    
  }
  else
  {
    retval = true;
  }
  
  return retval;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) 
{
   payload[ length ]  = '\0';
   Serial.print( "Received command from MQTT : " );
   Serial.println( (char*)payload );
   force_read = true;
}
