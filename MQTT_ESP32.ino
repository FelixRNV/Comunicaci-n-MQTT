/*
 Ejemplo básico de comunicación MQTT con ESP32.
  Basado en el ejemplo MQTT ESP8266

 En este código se realiza una comunicación MQTT,
 entre un broker y la ESP32, con el objetivo de:

  -Enceder el LED INTERNO (GIOP 2) al recibir un mensaje que empiece con "1"
  -Publicar en el monitor serial y como outTopic un mensaje de "Hola Mundo" + int (cada 2 seg);
  -Publicar en el monitor serial y como inTopic un mensaje con lo recibido.
  
 */

#include <WiFi.h> //Libreria para el uso de WIFI
#include <PubSubClient.h> //Libreria para la creación de un cliente para la conexión MQTT
#define BUILTIN_LED 2 //Se declara el puerto del led interno

// Se ingresan a continuación los datos de la red.

const char* ssid = "SweetHome.."; //Nombre de la red WiFi
const char* password = "xyshiz-9renpa-zujnEj"; //Contraseña de la red WiFi
const char* mqtt_server = "192.168.100.52"; //Se ingresa el IP del broker MQTT
/*Nota: por defecto la conexión MQTT en este código se da por el puerto 1883
        en caso de se necesario usar otro puerto revisar la línea de código
        en el void setup() ->  client.setServer(mqtt_server, 1883); y cambiar
        donde se encuentra el 1883 que es el valor del puerto.
*/

WiFiClient espClient; //Se crea una instancia para manejar el WiFi
PubSubClient client(espClient); //Se crea una instancia para la comunicación MQTT apartir del objeto anterior 
unsigned long lastMsg = 0; 
#define MSG_BUFFER_SIZE	(50) //Tamaño del buffer del mensaje
char msg[MSG_BUFFER_SIZE];   //Array tipo char de tamaño buffer del mensaje
int value = 0;

void setup_wifi() { //Se realiza la configuración y conexión WiFi

  delay(10);
  // Se inicia la conexión por WiFi
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid); //Se imprime en el monitor serial a que red se conecta

  WiFi.mode(WIFI_STA); //Se indica el modo de trabajo de la ESP32 
                       //en este caso como un dipositivo que se conecta
  WiFi.begin(ssid, password); //Se ingresan los datos para realizar la conexión

  while (WiFi.status() != WL_CONNECTED) { //Se crea un lazo para imprimir en el 
    delay(500);                           // monitor serial puntos '.' hasta que
    Serial.print(".");                    // se realice la conexión con la red.
  }

  randomSeed(micros());     //Se inicializa el generador de números aleatorios a partir
                            //del tiempo que lleva en funcionamiento de la ESP32.
  Serial.println("");                   //Se imprime en el monitor serial que
  Serial.println("WiFi connected");     //se ha conectado a la red y el IP del dispositivo.
  Serial.println("IP address: ");  
  Serial.println(WiFi.localIP());       //Método de WiFi para obtener el IP.
}

void callback(char* topic, byte* payload, unsigned int length) { //Se define la función callback que necesitará el PubSubClient
  Serial.print("Mensaje Recivido [");  //Se muestra en el monitor serial la información que llega a la ESP32.
  Serial.print(topic);                //El topic viene dado por la comunicación y este es de salida o entrada, para el callback
  Serial.print("] ");                 //se tiene un topic de entrada, por ejemplo, "inTopic".
  for (int i = 0; i < length; i++) {  //Se imprime el mensaje que llega de la comunicación MQTT.
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Esta parte del código realiza el encendido del LED Interno
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, HIGH);   // Se enciende (HIGH) el LED si llega un mensaje que contenga un '1'
                                       // en la primera posición del array tipo char
  } else {
    digitalWrite(BUILTIN_LED, LOW);  // En caso de que el mensaje no cumpla la condición
                                     // el LED permanece apagado (LOW).
  }

}

void reconnect() { // Se realiza la reconexión MQTT en caso de ser necesario 
  //Lazo para reconectar al ESP32
  while (!client.connected()) { //Se comprueba si el ESP esta conectado o no
    Serial.print("Intentado conexión MQTT..."); //Si no esta conectado se realiza:
          // Se crea un ID de cliente de forma aleatoria 
          //(Por se previamente se inicializó randomSeed()
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Se realiza un intento de conexión
    if (client.connect(clientId.c_str())) { //Se intenta la conexión dando el puntero de clientID
      Serial.println("conectado");
      // Se se realiza la conexión se publica un "Hola Mundo"
      client.publish("outTopic", "Hola Mundo");
      // ... luego se debe resubscribir
      client.subscribe("inTopic");
    } else { //En caso de no lograr la reconexión
      Serial.print("Error al conectar, rc=");   //Se muestra en el monitor serial un mensaje con el estado del cliente
      Serial.print(client.state());  //y que se reintentará en 5 segundos
      Serial.println(" se reintentará en 5 segundos");
      // Se crea un retardo de 5 segundos
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);           // Se define como salida al pin del LED Interno(GIOP2)
  Serial.begin(9600);                     // Se inicia el contador serial
  setup_wifi();                           // Se llama a la función para la conexión WiFi
  client.setServer(mqtt_server, 1883);    // Se setea el servidor y puerto de la comunicación MQTT 
  client.setCallback(callback);           // Se declara la función callback como la propia de client 
}

void loop() {

  if (!client.connected()) { //Se revisa si se encuentra activa la conexión MQTT
    reconnect(); //Si no se está conectado se llama a reconectar
  }
  client.loop(); //Lee el buffer de datos y llama al callback si existe un msg

  unsigned long now = millis(); //Se obtiene el tiempo que se ha corrido el programa actual
  if (now - lastMsg > 2000) { // Se comprueba si han pasado más de 2 segundos
    lastMsg = now;            // Se almacena el tiempo que ha pasado
    ++value;                  // Se incrementa un contador
    snprintf (msg, MSG_BUFFER_SIZE, "Hola Mundo #%ld", value);
    
    /*Uso de snprintf():
    Este método permite almacenar dentro de buffer de un tamaño n, un array de char
    cuyo tamaño máximo es n, es decir, si el array a guardar es menor a n 
    elementos se almacenará todo el contenido, pero si el array a guardar es 
    mayor n elementos solo se almacenarán los primeros elementos que llenen
    el buffer. Su estructura es:
    snprintf(buffer,tamaño_buffer,cadena_de_caracteres,otras_opciones)
    Para este caso se está trabajando con chars y se tiene:
    > tamaño del buffer = MSG_BUFFER_SIZE
    > buffer = msg ->Array de char de tamaño del buffer
    > cadena de caracteres = "Hola Mundo #$ld" -> Se referencia a una variable tipo int
    > otras opciones = value -> Variable referenciada en la cadena
    
    Nota: msg es un buffer diferente al buffer que se menciona dentro la comunicación MQTT
    */
    
    Serial.print("Mensaje Publicado: "); //Se muestra el msg en el monitor serial
    Serial.println(msg);
    client.publish("outTopic", msg); //Publica un mensaje en el buffer de comunicación MQTT
  }
}
