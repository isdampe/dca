/*#include "I2CSlaveRK.h"

// Set up this Photon as an I2C device device, address 0x10, with 10 uint32 registers
I2CSlave device(Wire, 0x10, 2);
static bool changed = false;

unsigned long lastCounterUpdate = 0;
uint32_t counter = 0;

void setup() {
	Serial.begin(9600);

	device.begin();
}

void loop() {

	if (millis() - lastCounterUpdate >= 1000) {
		// Once per second increment register 0
		lastCounterUpdate = millis();

		//Serial.printlnf("Counter %u", counter);
		//Serial.printlnf("Reg 0 is %u", device.getRegister(0));
		counter++;
		if (counter > 255)
		    counter = 0;
		//device.setRegister(0, counter);
		//device.setRegister(0, 50463231);
	}

	uint16_t regAddr;
	while(device.getRegisterSet(regAddr)) {
		// regAddr was updated from the I2C master
		int val = device.getRegister(regAddr);
		Serial.printlnf("master updated %u to %u", regAddr, val);
		Serial.printlnf("Bytes received:");
		Serial.printlnf("1: 0x%02x", (val) & 0xff);
		Serial.printlnf("2: 0x%02x", (val >> 8) & 0xff);
		Serial.printlnf("3: 0x%02x", (val >> 16) & 0xff);
		Serial.printlnf("4: 0x%02x", (val >> 24) & 0xff);
		
		unsigned int cmd = ((val) & 0xff);
		
		Serial.printlnf("Cmd is %02x", cmd);
		
		//If 0
		if (cmd == 0x0) {
		    Serial.printlnf("Ping!");
		    device.setRegister(0, 0x100);
		    Serial.printlnf("Reg 0 is %u", device.getRegister(0));
		} else if (cmd == 0x1) {
		    Serial.printlnf("Order!");
		    device.setRegister(0, 0x100);
		    Serial.printlnf("Reg 0 is %u", device.getRegister(0));
		} else if (cmd == 0x2) {
		    Serial.printlnf("Returning status!");
		    device.setRegister(0, 0x090100);
		    Serial.printlnf("Reg 0 is %u", device.getRegister(0));
		} else if (cmd == 0x3) {
		    unsigned int rv = ((val >> 16) & 0xff);
		    Serial.printlnf("Result req index 0x%02x", rv);
		    device.setRegister(0, (rv << 16) | (0x1000) | (0x00));
		} else if (cmd == 0x4) {
		    Serial.printlnf("Cancelling...");
		    device.setRegister(0, 0x100);
		}
		
		Serial.printlnf("\n");
		
		changed = true;
	}
}*/