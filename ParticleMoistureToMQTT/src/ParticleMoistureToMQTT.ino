SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler;

const pin_t MOISTURE = A0; 

int soilMoisture; 


void setup() {
	Particle.variable("Soil Moisture", soilMoisture);
}

void loop() {
	soilMoisture = analogRead(MOISTURE);
	Log.info("soilMoisture=%d", soilMoisture);
	Particle.publish("Soil Moisture", String(soilMoisture));
	//Collect values every minute
	delay(60000ms);
}