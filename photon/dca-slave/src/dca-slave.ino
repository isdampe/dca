#include "I2CSlaveRK.h"
#include "efp.h"

static I2CSlave device(Wire, EFP_SLAVE_ADDR, EFP_SLAVE_REGISTERS);
static efp_slave slave;

void setup()
{
	slave.mode = EFP_MODE_IDLE;
	slave.current_job_start_idx = 0x0;
	slave.current_job_progress = 0x0;
	slave.current_job_results[EFP_JOB_FACTOR] = {0};

	Serial.begin(9600);
	device.begin();
}

void loop()
{
	Serial.printlnf("Hello world...");
}