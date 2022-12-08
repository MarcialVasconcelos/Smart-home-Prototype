#include <ESP8266WiFi.h>   // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h>  // Importa a Biblioteca PubSubClient
#include <DHT.h>
#include <ArduinoJson.h>

/*defines - mapeamento de pinos do NodeMCU*/
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

/*Definições de Estados*/
#define MSG_BUFFER_SIZE (50)
#define JSON_BUFFER_SIZE 256
#define TYPE 5
#define QUANTITY 5
int count = 0;
int states[TYPE][QUANTITY] = {0};

/* Definicoes do sensor DHT22 */
#define DHTPIN D3     //GPIO que está ligado o pino de dados do sensor
 
//#define DHTTYPE DHT11
#define DHTTYPE DHT11   //sensor em utilização: DHT22
//#define DHTTYPE DHT21

/* Defines do MQTT */
#define TOPICO_SUBSCRIBE             "topico_breno_sub"
#define TOPIC_PUBLISH                "topico_breno_pub"
#define TOPICO_PUBLISH_TEMPERATURA   "Temp_Q3_esp8266"
#define TOPICO_PUBLISH_UMIDADE       "Umid_Q3_esp8266"

#define ID_MQTT "HomeAut"  //id mqtt (para identificação de sessão)

/* Variaveis, constantes e objetos globais */
DHT dht(DHTPIN, DHTTYPE);

const char* SSID = "Alunos";     // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "alunos2018";  // Senha da rede WI-FI que deseja se conectar

const char* BROKER_MQTT = "broker.emqx.io";  //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                       // Porta do Broker MQTT

//Variáveis e objetos globais
WiFiClient espClient;          // Cria o objeto espClient
PubSubClient MQTT(espClient);  // Instancia o Cliente MQTT passando o objeto espClient

/* Prototypes */
float faz_leitura_temperatura(void);
float faz_leitura_umidade(void);
void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);
void sendStates();

/*
 * Implementações
 */
/* Função: faz a leitura de temperatura (sensor DHT22)
 * Parametros: nenhum
 * Retorno: temperatura (graus Celsius)
 */
float faz_leitura_temperatura(void)
{
    float t = dht.readTemperature();
    float result;
     
    if (! (isnan(t)) )
        result = t;
    else
        result = -99.99;
    states[1][0] = result;
    return result;
}
 
/* Função: faz a leitura de umidade relativa do ar (sensor DHT22)
 * Parametros: nenhum
 * Retorno: umidade (0 - 100%)
 */
float faz_leitura_umidade(void)
{
    float h = dht.readHumidity();    
    float result;
     
    if (! (isnan(h)) )
        result = h;
    else
        result = -99.99;
    states[2][0] = result; 
    return result;
}

/* Função: inicializa e conecta-se na rede WI-FI desejada
*  Parâmetros: nenhum
*  Retorno: nenhum
*/
void initWiFi(void) {
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}

void piscaLed(int pin1, int pin2, int pin3, int v) {
  MQTT.publish(TOPIC_PUBLISH, "MODO FESTA");
  for (int i = 0; i < v; i++) {
    digitalWrite(pin1, HIGH);
    states[0][1] = 1;
    sendStates();
    delay(50);
    digitalWrite(pin1, LOW);
    states[0][1] = 0;
    sendStates();
    delay(50);
    digitalWrite(pin2, HIGH);
    states[0][2] = 1;
    sendStates();
    delay(50);
    digitalWrite(pin2, LOW);
    states[0][2] = 0;
    sendStates();
    delay(50);
    digitalWrite(pin3, HIGH);
    states[0][3] = 1;
    sendStates();
    delay(50);
    digitalWrite(pin3, LOW);
    states[0][3] = 0;
    sendStates();
    delay(50);
  }
}

/* Função: inicializa parâmetros de conexão MQTT(endereço do 
 *         broker, porta e seta função de callback)
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void initMQTT(void) {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);           //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

/* Função: função de callback 
 *         esta função é chamada toda vez que uma informação de 
 *         um dos tópicos subescritos chega)
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;

  /* obtem a string do payload recebido */
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.print("Chegou o seguinte comando via MQTT: ");
  Serial.println(msg);

  /* toma ação dependendo da string recebida */
  if (msg.equals("LED1 ON")) {
    digitalWrite(D0, HIGH);
    states[0][1] = 1;
    sendStates();
    Serial.print("O LED1 está aceso!!");
    MQTT.publish(TOPIC_PUBLISH, "O LED1 está aceso!!");
  }

  if (msg.equals("LED1 OFF")) {
    digitalWrite(D0, LOW);
    states[0][1] = 0;
    sendStates();
    Serial.print("O LED1 está apagado!!");
    MQTT.publish(TOPIC_PUBLISH, "O LED1 está apagado!!");
  }

  if (msg.equals("LED2 ON")) {
    digitalWrite(D1, HIGH);
    states[0][2] = 1;
    sendStates();
    Serial.print("O LED2 está aceso!!");
    MQTT.publish(TOPIC_PUBLISH, "O LED2 está aceso!!");
  }

  if (msg.equals("LED2 OFF")) {
    digitalWrite(D1, LOW);
    states[0][2] = 0;
    sendStates();
    Serial.print("O LED2 está apagado!!");
    MQTT.publish(TOPIC_PUBLISH, "O LED2 está apagado!!");
  }

  if (msg.equals("LED3 ON")) {
    digitalWrite(D2, HIGH);
    states[0][3] = 1;
    sendStates();
    Serial.print("O LED3 está aceso!!");
    MQTT.publish(TOPIC_PUBLISH, "O LED3 está aceso!!");
  }

  if (msg.equals("LED3 OFF")) {
    digitalWrite(D2, LOW);
    states[0][3] = 0;
    sendStates();
    Serial.print("O LED3 está apagado!!");
    MQTT.publish(TOPIC_PUBLISH, "O LED3 está apagado!!");
  }

  if (msg.equals("FESTA ON")) {
    states[0][0] = 1;
    sendStates();
    piscaLed(D0, D1, D2, 20);
  }

  if (msg.equals("FESTA OFF")) {
    digitalWrite(D0, LOW);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    MQTT.publish(TOPIC_PUBLISH, "A FESTA ACABOU");
    states[0][0] = 0;
    sendStates();
  }
}

/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected()) {
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
  }  
  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 *         em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnectMQTT(void) {
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }
}

/* Função: reconecta-se ao WiFi
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnectWiFi(void) {
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);  // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

void sendStates() {
  // JSON config  
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;
  char buffer[JSON_BUFFER_SIZE];
  doc["Device"] = "ESP32";
  doc["Room"] = "03";
  JsonArray lampStates = doc.createNestedArray("Lamps");
  JsonArray tempStates = doc.createNestedArray("Temp");
  JsonArray humiStates = doc.createNestedArray("Humi");
  lampStates.add(states[0][0]);
  lampStates.add(states[0][1]);
  lampStates.add(states[0][2]);
  lampStates.add(states[0][3]);
  tempStates.add(states[1][0]);
  humiStates.add(states[2][0]);

  serializeJson(doc, buffer);
  MQTT.publish("AutomataHomeStates", buffer);
}

/* Função de setup */
void setup() {
  Serial.begin(115200);

  /* Configuração do pino ligado ao LED como output 
       e inicialização do mesmo em LOW */
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  pinMode(D2, OUTPUT);
  digitalWrite(D2, LOW);

  /* Inicializacao do sensor de temperatura */
  dht.begin();  

  /* Inicializa a conexao wi-fi */
  initWiFi();

  /* Inicializa a conexao ao broker MQTT */
  initMQTT();
}

/* Loop principal */
void loop() {
  char temperatura_str[10] = {0};
  char umidade_str[10]     = {0};
     
  /* garante funcionamento das conexões WiFi e ao broker MQTT */
  VerificaConexoesWiFIEMQTT();

  /* Compoe as strings a serem enviadas pro dashboard (campos texto) */ 
  sprintf(temperatura_str,"%.2fC", faz_leitura_temperatura());
  sprintf(umidade_str,"%.2f", faz_leitura_umidade());
 
  /*  Envia as strings ao dashboard MQTT */
  MQTT.publish(TOPICO_PUBLISH_TEMPERATURA, temperatura_str);
  MQTT.publish(TOPICO_PUBLISH_UMIDADE, umidade_str);
  
  /* keep-alive da comunicação com broker MQTT */
  MQTT.loop();


  if (count == 10){
    count = 0;
    sendStates();
  }
  count++;

  /* Refaz o ciclo após 2 segundos */
  delay(2000);
}