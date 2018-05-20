#include <stdint.h>
#include "I2CSlaveRK.h"
#include "algorithm.h"
#include "efp.h"

static I2CSlave device(Wire, EFP_SLAVE_ADDR, EFP_SLAVE_REGISTERS);
static efp_slave slave;

SYSTEM_THREAD(ENABLED);
static Thread *compute_thread;

void setup()
{
	efp_slave_init(&slave);
	Serial.begin(9600);
	device.begin();

	compute_thread = new Thread("compute_thread", compute);
}

void loop()
{
	uint32_t result = 0x0;
	//If the master hasn't updated the I2C register, do nothing.
	if(! device.getRegisterSet(slave.reg_val))
		return;
	
	efp_slave_parse_registers(device.getRegister(slave.reg_val), &slave, 0x0);
	Serial.printf("Command received from master: ");
	switch (efp_get_register_byte(&slave, EFP_CMD_REGISTER_BYTE))
	{
		case EFP_CMD_PING:
			Serial.printlnf("Ping");
			efp_set_ack(&slave, EFP_ACK_OK);
			device.setRegister(0x0, efp_pack_registers(&slave));
		break;
		case EFP_CMD_ORDER:
			Serial.printlnf("Work order");

			if (slave.mode != EFP_MODE_IDLE)
			{
				Serial.printlnf("Cannot accept work, not in idle mode.");
				efp_set_ack(&slave, EFP_ACK_ERR);
			} 
			else
			{

				//Grab the work value.
				uint8_t work_value = efp_get_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE);
				Serial.printlnf("Requested work value is: %u", work_value);
				Serial.printlnf("Meaning I will compute digital %u to %u.", (work_value * EFP_JOB_FACTOR), ((work_value * EFP_JOB_FACTOR) + EFP_JOB_FACTOR -1));

				efp_set_job(&slave, work_value);
				efp_set_ack(&slave, EFP_ACK_OK);
			}

			device.setRegister(0x0, efp_pack_registers(&slave));

		break;
		case EFP_CMD_STATUS:
			Serial.printlnf("Check status");
		break;
		case EFP_CMD_RESULT:
			Serial.printlnf("Request result");
		break;
		case EFP_CMD_CANCEL:
			Serial.printlnf("Cancel");
		break;
		default:
			Serial.printlnf("Unknown command byte received: 0x%02x", slave.registers[EFP_CMD_REGISTER_BYTE]);
		break;
	}
	
	Serial.printlnf("Some number: 0x%08x", efp_pack_registers(&slave));
	
}

void compute()
{
	while (1)
	{
		if (slave.mode != EFP_MODE_WORK)
		{
			//Serial.printlnf("Waiting for work...");
			//delay(50);
			os_thread_yield();
			continue;
		}

		int start = slave.current_job_start_idx * EFP_JOB_FACTOR;
		int end = start + (EFP_JOB_FACTOR -1);
		int n = start, r;
		Serial.printlnf("Computing...");
		
		for (slave.current_job_progress=0; n<=end; ++slave.current_job_progress)
		{
			slave.current_job_results[slave.current_job_progress] = get_nth_digit(n++);
			os_thread_yield();
		}

		efp_set_done(&slave);
		Serial.printlnf("Digit computation done.");
		for (uint8_t x=0; x<EFP_JOB_FACTOR; ++x)
			Serial.printf("%i", slave.current_job_results[x]);
		Serial.printf("\n");
		os_thread_yield();
		
	}
}