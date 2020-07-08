## OLD VERSION OF EXAMPLE

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
