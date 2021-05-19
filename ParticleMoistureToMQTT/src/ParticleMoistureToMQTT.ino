#include <SparkFunTMP102.h>
#include <MQTT.h>
#include <particle_secrets.h>

//pin setups
const pin_t MOISTURE = A0;
const pin_t LIGHT_PIN = A1;
TMP102 sensor0(0x48); // Initialize sensor at I2C address 0x48
// Sensor address can be changed with an external jumper to:
// ADD0 - Address
//  VCC - 0x49
//  SDA - 0x4A
//  SCL - 0x4B

void callback(char* topic, byte* payload, unsigned int length);

// recieve message
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    delay(1000);
}

//Globally Available Stuff
byte server[] = { 10,0,0,10 };
MQTT client(server, 1883, 255, 120, callback);
const String MQTTusername = SECRETS_MQTTUsername;
const String MQTTpassword = SECRETS_MQTTPassword;

int soilMoistureRaw;
int lightSensor;
double soilMoisturePercentage;
bool MQTTConnected;
double temperature;

void setup() {
	Particle.variable("Soil Moisture Raw", soilMoistureRaw);
	Particle.variable("Soil Moisture Percentage", soilMoisturePercentage);
	Particle.variable("Connected to MQTT", MQTTConnected);
	Particle.variable("Light Sensor", lightSensor);

    connectMQTT();

    // publish/subscribe
    if (client.isConnected()) {
        client.publish("outTopic/message","hello world");
        Particle.publish("MQTT Client", " Connected!");
	}else
    {
        Particle.publish("MQTT Client", " Not Connected!");
    }
    
    //Temperature Setup
    sensor0.begin(); //Join I2C Bus
    Particle.publish("TMP102", " Connected!");
    
	Particle.variable("Temperature", temperature);
	
	//setup Customer MQTT Sensor Automatic Discovery from Home Assistant
	
    
}

void loop() {
	if (client.isConnected()) {
	    lightSensor = analogRead(LIGHT_PIN);
	    MQTTConnected = true;
        soilMoistureRaw = analogRead(MOISTURE);
        soilMoisturePercentage = (double(100) - (double(soilMoistureRaw) / double(4000)*double(100)));
	    Particle.publish("Soil Moisture Raw", String(soilMoistureRaw));
        Particle.publish("Soil Moisture Percentage", String(soilMoisturePercentage));
        
        
        //Temperature Collection
        sensor0.wakeup();
        temperature = sensor0.readTempC();
        sensor0.sleep();
        Particle.publish("Room Temperature", String(temperature));
        
        configureNewSensor();
        
	    //Collect values every minute
	    delay(60000ms);
	    client.loop();
	}
	else{
	    MQTTConnected = false;
	    connectMQTT();
	}
	
}

void connectMQTT(){
    client.connect("martyParticle_" + String(Time.now()),MQTTusername,MQTTpassword);
    //id, user, pass, 0, QOS0, 0, 0, true
}

void sendMQTTSensorValue(){
    String topic = "home";
}

void configureNewSensor(){
    //Any changes to this configuration will be updated automatically in homeassistnat
    //If an empty payload is sent for this configuration topic then the sensor will be deleted
    String configTopic = "homeassistant/sensor/soil_moisture/config";
    String stateTopic = "homeassistant/sensor/soil_moisture/state";
    bool retain = true;
    client.publish(configTopic, createMQTTConfigJSONPayload("Soil Moisture", stateTopic));
    client.publish(stateTopic, String(soilMoisturePercentage, 2), retain);
    
    
    String configTopicTemp = "homeassistant/sensor/soil_temperature/config";
    String stateTopicTemp = "homeassistant/sensor/soil_temperature/state";
    client.publish(configTopicTemp, createMQTTTempConfigJSONPayload("Soil Temperature", stateTopicTemp));
    client.publish(stateTopicTemp, String(temperature, 2), retain);
    
    String configTopicLight = "homeassistant/sensor/soil_light/config";
    String stateTopicLight = "homeassistant/sensor/soil_light/state";
    client.publish(configTopicLight, createMQTTLightConfigJSONPayload("Soil Light", stateTopicLight));
    client.publish(stateTopicLight, String(lightSensor), retain);
}

String createMQTTConfigJSONPayload(String sensorName, String stateTopic){
    char buf[256];
    JSONBufferWriter writer(buf, sizeof(buf) -1);
    writer.beginObject();
        writer.name("device_class").value("humidity");
        writer.name("name").value(sensorName);
        writer.name("state_topic").value(stateTopic);
        writer.name("unit_of_measurement").value("%");
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    
    return String(buf);
    
}

String createMQTTTempConfigJSONPayload(String sensorName, String stateTopic){
    char buf[256];
    JSONBufferWriter writer(buf, sizeof(buf) -1);
    writer.beginObject();
        writer.name("device_class").value("temperature");
        writer.name("name").value(sensorName);
        writer.name("state_topic").value(stateTopic);
        writer.name("unit_of_measurement").value("°C");
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    
    return String(buf);
    
}

String createMQTTLightConfigJSONPayload(String sensorName, String stateTopic){
    char buf[256];
    JSONBufferWriter writer(buf, sizeof(buf) -1);
    writer.beginObject();
        writer.name("device_class").value("illuminance");
        writer.name("name").value(sensorName);
        writer.name("state_topic").value(stateTopic);
        writer.name("unit_of_measurement").value("lx");
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    
    return String(buf);
    
}