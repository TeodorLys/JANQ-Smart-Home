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

//This example will start sending the phrase ON / OFF to the server in a loop 
//until the device receives a new request
 class response : public response_handler {
 public:
   void client_print(WiFiClien& client){
     client.print("<p>JANKY</p>");
   }
 };
 
 response res;
 net_parser parser;
 net_connection net(&res, parser.get_assigned_name());
 function_handler <3>loop;  // 3 meaning how many parameters the call will have
                            // in this case "loop{ON:OFF:timer}"
 
 void setup(){
   parser.add_parameter("Servo");
   parser.add_parameter("function");
   function.register_function([]{
     static bool b = false;
     static long timer = millis();
     if(millis() - timer >= loop.p[2]) {
       if(b)
         net.send_status_to_server("ON");
       else
         net.send_status_to_server("OFF");
         
       b = !b;
       timer = millis();
     }
   });
   //Gives the net_connection class all of the registered parameters
   //this will always be sent as a response to any device sending a request to this device
   //Example: http://192.168.0.195/actionpage.php?Servo=100&function=0
   //Example: http://192.168.0.195/actionpage.php?Servo=-1&function=loop{0:1:1000}
   net.set_request_form(parser.get_formatted_request_form());
   
   //Tries to connect, if no credentials was found, 
   //it will start a accesspoint for configuration
   net.connect();
 }
 
 void loop() {
   if(loop.active()){
     loop.call_function();
   }
   
   if(net.new_client_connected()){
     parser.parse_header(net.get_header());
     //if function wasnt requested this will return ""
     if(parser.get_string_parameter("function").length() > 2){
       loop.parse_function_call(parser.get_string_parameter("function"));
       loop.active(true);
     }else {
       loop.active(false);
     }
     
     // If "Servo" wasnt requesed this will return -1
     if(parser.get_parameter("Servo") > 0){
       net.send_status_to_server("DID A THING -> " + String(parser.get_parameter("Servo")));
     }
   }
 }
 
 ```
