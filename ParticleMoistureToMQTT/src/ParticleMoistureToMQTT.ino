#include <SparkFunTMP102.h>
#include <MQTT.h>
#include <particle_secrets.h>


//MQTT Sensors for Home Assistant
//Moisture
const String moistureConfigTopic = "homeassistant/sensor/soil_moisture/config";
const String moistureStateTopic = "homeassistant/sensor/soil_moisture/state";
const String moistureSensorName = "Soil Moisture";
const String moistureDeviceClass = "humidity";
const String moistureUnitOfMeasure = "%";

//Temperature
const String temperatureConfigTopic = "homeassistant/sensor/soil_temperature/config";
const String temperatureStateTopic = "homeassistant/sensor/soil_temperature/state";
const String temperatureSensorName = "Soil Temperature";
const String temperatureDeviceClass = "temperature";
const String temperatureUnitOfMeasure = "°C";

//Light
const String lightConfigTopic = "homeassistant/sensor/soil_light/config";
const String lightStateTopic = "homeassistant/sensor/soil_light/state";
const String lightSensorName = "Soil Light";
const String lightDeviceClass = "illuminance";
const String lightUnitOfMeasure = "lx";

//pin setup
const pin_t MOISTURE = A0;
const pin_t LIGHT_PIN = A1;
TMP102 sensor0(0x48); 

void callback(char* topic, byte* payload, unsigned int length);

// recieve message
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    delay(1000);
}

//Globally Available
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
    //Varriables Accessible from Particle Console
	Particle.variable("Soil Moisture Raw", soilMoistureRaw);
	Particle.variable("Soil Moisture Percentage", soilMoisturePercentage);
	Particle.variable("Connected to MQTT", MQTTConnected);
	Particle.variable("Light Sensor", lightSensor);
	Particle.variable("Temperature", temperature);

    connectMQTT();
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

    configureMQTTSensors();   
}

void loop() {
	if (client.isConnected()) {
	    lightSensor = analogRead(LIGHT_PIN);
	    MQTTConnected = true;
        soilMoistureRaw = analogRead(MOISTURE);
        soilMoisturePercentage = (double(100) - (double(soilMoistureRaw) / double(4000)*double(100)));
             
        //Temperature Collection
        sensor0.wakeup();
        temperature = sensor0.readTempC();
        sensor0.sleep();
        //Particle.publish("Room Temperature", String(temperature));

        sendMQTTStateMessages();
            
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
    //Add Datetime stamp just for logging in case there's some debugging needed later for reconnects
    client.connect("martyParticle_" + String(Time.now()),MQTTusername,MQTTpassword);
}

void configureMQTTSensors(){
    //Any changes to this configuration will be updated automatically in homeassistnat
    //If an empty payload is sent for this configuration topic then the sensor will be deleted

    //Moisture
    client.publish(moistureConfigTopic, createMQTTConfigJSONPayload(moistureSensorName, moistureStateTopic, moistureDeviceClass, moistureUnitOfMeasure));
    //Temperature
    client.publish(temperatureConfigTopic, createMQTTConfigJSONPayload(temperatureSensorName, temperatureStateTopic, temperatureDeviceClass, temperatureUnitOfMeasure));
    //Light
    client.publish(lightConfigTopic, createMQTTConfigJSONPayload(lightSensorName, lightStateTopic, lightDeviceClass, lightUnitOfMeasure));   
}

String createMQTTConfigJSONPayload(String sensorName, String stateTopic, String deviceClass, String UOM){
    char buf[256];
    JSONBufferWriter writer(buf, sizeof(buf) -1);
    writer.beginObject();
        writer.name("device_class").value(deviceClass);
        writer.name("name").value(sensorName);
        writer.name("state_topic").value(stateTopic);
        writer.name("unit_of_measurement").value(UOM);
    writer.endObject();
    writer.buffer()[std::min(writer.bufferSize(), writer.dataSize())] = 0;
    
    return String(buf);
    
}

void sendMQTTStateMessages(){
    //make sure retain flag is set to retain MQTT server sends homeassistant the latest received message on startup
    bool retain = true;

    //Moisture
    client.publish(moistureStateTopic, String(soilMoisturePercentage, 2), retain);
    //Temperature
    client.publish(temperatureStateTopic, String(temperature, 2), retain);
    //Light
    client.publish(lightStateTopic, String(lightSensor), retain);
}