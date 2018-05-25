#include <stdint.h>
#include "I2CSlaveRK.h"
#include "algorithm.h"
#include "efp.h"

static I2CSlave device(Wire, EFP_SLAVE_ADDR, EFP_SLAVE_REGISTERS);
static efp_slave slave;

//Allow application level system thread interrupts.
//This is required to prevent I2C timeouts especially when computing pi(n) for n > ~50
SYSTEM_THREAD(ENABLED);

//Run the computation work in a separate thread.
static Thread *compute_thread;

/**
 * The photon initialiser function.
 */
void setup()
{
	efp_slave_init(&slave);

	//For debugging purposes.
	Serial.begin(9600);
	device.begin();
	compute_thread = new Thread("compute_thread", compute);
}

/**
 * The main system thread loop.
 * Mostly checks for I2C changes and dispatches actions.
 * @return void
 */
void loop()
{
	//If there I2C register has not been written by the master, there is nothing
	//to do in this main thread.
	if(! device.getRegisterSet(slave.reg_val))
		return;

	//At this point, the I2C register has been updated by the master.
	efp_slave_parse_registers(device.getRegister(slave.reg_val), &slave, 0x0);
	Serial.printf("Command received from master: ");

	//The I2C master writes 32 bits at a time. Each byte represents some context.
	//EFP_CMD_REGISTER_BYTE represents byte 1, which is the command the master has
	//given for action.
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
				//The requested work value is stored in the third byte, as notified in EFP_CMD_REGISTER_DATA_BYTE.
				uint8_t work_value = efp_get_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE);
				Serial.printlnf("Requested work value is: %u", work_value);

				efp_set_job(&slave, work_value);
				efp_set_ack(&slave, EFP_ACK_OK);
			}

			device.setRegister(0x0, efp_pack_registers(&slave));

		break;
		case EFP_CMD_STATUS:
			Serial.printlnf("Check status");
			efp_set_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE, slave.current_job_progress);
			efp_set_ack(&slave, EFP_ACK_OK);
			device.setRegister(0x0, efp_pack_registers(&slave));
		break;
		case EFP_CMD_RESULT:
			Serial.printlnf("Request result");
			if (slave.mode != EFP_MODE_DONE)
			{
				Serial.printlnf("Cannot give results while still computing");
				efp_set_ack(&slave, EFP_ACK_ERR);
			}
			else
			{
				//Write the requested result number to the data byte of the slave register.
				//The master will iteratively retrieve this as it needs it.
				uint8_t idxRequested = efp_get_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE);
				if (idxRequested > EFP_JOB_FACTOR || idxRequested <= 0)
				{
					Serial.printlnf("The requested result index is greater than EFP job factor. No buffer overflows here!");
					efp_set_ack(&slave, EFP_ACK_ERR);
				}
				else
				{
					efp_set_register_byte(&slave, EFP_CMD_REGISTER_DATA_BYTE, slave.current_job_results[idxRequested -1]);
					efp_set_ack(&slave, EFP_ACK_OK);
				}
			}
			device.setRegister(0x0, efp_pack_registers(&slave));

		break;
		case EFP_CMD_RESET:
			Serial.printlnf("Reset");
			if (slave.mode != EFP_MODE_DONE)
			{
				Serial.printlnf("Can only reset when done");
				efp_set_ack(&slave, EFP_ACK_ERR);
			}
			else
			{
				efp_set_ack(&slave, EFP_ACK_OK);
			}
			efp_set_idle(&slave);
			device.setRegister(0x0, efp_pack_registers(&slave));

		break;
		default:
			//Unless there's some serious interference going on, this should never happen.
			Serial.printlnf("Unknown command byte received: 0x%02x", slave.registers[EFP_CMD_REGISTER_BYTE]);
		break;
	}

}

/**
 * The computation thread for the Photon slave.
 * Operates iteratively, blocking and yielding until there is work to do.
 * @return void
 */
void compute()
{
	while (1)
	{
		if (slave.mode != EFP_MODE_WORK)
		{
			//If there's nothing to do, context switch back to the system thread.
			os_thread_yield();
			continue;
		}

		int start = slave.current_job_start_idx * EFP_JOB_FACTOR +1;
		int end = start + (EFP_JOB_FACTOR -1);
		int n = start;
		Serial.printlnf("Computing %i to %i", start, end);

		for (slave.current_job_progress=0; n<=end; ++slave.current_job_progress)
		{
			slave.current_job_results[slave.current_job_progress] = get_nth_digit(n++);
			//After each digit of job has computed, give the system thread some
			//time to respond to the masters I2C requests.
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
