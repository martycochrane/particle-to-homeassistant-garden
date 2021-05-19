# Particle.io To Home Assistant Garden

## Description

I use the great home-assistant.io to power all of my home automation activities and dashboards. Coming into the Spring 2021 I was getting some new vegetables ready for my garden but the temperature outside was changing a lot. Here started the journey of using some of my spare particle Argon boards to send temperature & moisture data to by home-assistant instance to help me track if my vegetables outside might die.

## Hardware Used

1. Particle Argon - https://docs.particle.io/argon/
2. TMP102 Temperature Sensor - https://www.sparkfun.com/products/13314
3. Generic FC28 Moisture Sensor - https://www.circuitstoday.com/arduino-soil-moisture-sensor
4. Generic photoresistor - https://learn.adafruit.com/photocells/arduino-code

## Architecture

All sensors sending data to home assistant mosquitto MQTT server with MQTT Broker integration installed to auto discover config and state topics

## Todo
- [ ] Expand Readme and setup instructions
- [ ] Wiring Diagram
- [ ] Create development roadmap
- [ ] Update with new big hardware changes coming
- [ ] Share 3D print enclousures
- [ ] Share Home Assistant Config
- [ ] Add pictures
- [ ] Create Architecture Overview Diagram
- [ ] MQTT Setup Considerations
