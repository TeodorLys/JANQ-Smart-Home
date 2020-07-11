<p align="center">
<img src="doc/Janq_logo.PNG" align="center" width="250" height="125"/> 
 </p>
 
# JANQ Smart Home Solution
With this system you can connect any LoT device there is, with the given protocol. Like a ESP8266/ESP32.
The server consists of a Raspberry pi and a router, that uses udp for responses and handling requests,
### WHY?
Why indeed, this is basically for me to learn about LoT devices and - in the future - sending information securely. I just needed a (sort of)solid platform to test stuff...
### Should you use this system?
OH DEAR GOD, PLEASE DONT! the entire system is soooo janky, I mean its even called Janq, I even created a logo for it! 

# LoT Device Features
All devices has a few features integrated into the library. 
#### GET:
Just sends a command to the server, which makes it display all information about the device i.e. ip, mac etc.
#### assign:
Each devices must have a name assigned to it, just for the sake of aesthetics.
#### function:
This feature does not have to be used. For example: a pair of lights that you want to start flashing, instead of telling the server to send a bunch of requests to the device. You can have the device do stuff internally. All of this is handled by the "function_handler.h" file.

## Example 
```c++

//This example will start sending the words "ON" or "OFF" to the server in a loop 
//until the device receives a new request
 
 net_api api;
 function_handler <1>loop;  // 1 meaning how many parameters the call will have
                            // in this case "loop{timer}"
 void setup() {
   function.register_function([] {
     static bool b = false;
     static long timer = millis();
     if(millis() - timer >= loop.p[0]) {
       if(b)
         net.send_status_to_server("ON");
       else
         net.send_status_to_server("OFF");
         
       b = !b;
       timer = millis();
     }
   });
   
   //Tells the api what parameters you want to use,
   //and what the parser will look for.
   //Tries to connect, if no credentials was found, 
   //it will start a accesspoint for configuration
   api.initialize(2, "Servo", "function");
 }
 
 void loop() {
   if(loop.active()) {
     loop.call_function();
   }
   
   if(api.got_request()){
     if(api.parameter("function", GET_STRING).length() > 2) {
       loop.parse_function_call(api.parameter("function", GET_STRING));
       loop.active(true);
     }else {
       loop.active(false);
     }
     // If parameter "Servo" were in the request
     if(parser.parameter("Servo", GET_EXISTANCE)) {
       api.push_message_to_server()("DID A THING -> " + String(parser.get_parameter("Servo")));
     }
   }
 }
 
 ```
