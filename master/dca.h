#ifndef DCA_H
#define DCA_H
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "scheduler.h"
#include "tui.h"
#include "i2c.h"
#include "efp.h"
#include "log.h"

#define WORK_STEP_SIZE 5
#define WORK_MAX_REQUESTS 30
#define EFP_ORDER_TIMEOUT 500

#define DCA_HW_ADDR_PHOTON 0x10
#define DCA_HW_ADDR_MBED 0x50
#define DCA_CHECKSUM_OVERCOUNT 100

static i2c_obj slave_photon, slave_mbed;
static I2C_STATUS status;
static scheduler s;

static uint8_t results[WORK_STEP_SIZE * WORK_MAX_REQUESTS];
static uint8_t jobs[WORK_MAX_REQUESTS];

static char system_log[DCA_LOG_MAX_LINES][DCA_LOG_MAX_STR_LEN];
static char results_log[DCA_LOG_MAX_LINES][DCA_LOG_MAX_STR_LEN];
static char i2c_log[DCA_LOG_MAX_LINES][DCA_LOG_MAX_STR_LEN];
static uint32_t solved_by[2];
static uint32_t error_by[2];
static unsigned int checksum_counter[2];

static tui_mngr mngr;

static void log_render();
void setup_jobs();
bool setup_i2c_slaves();
bool setup_scheduler();
int job_get_next();
void auto_dispatch_work();
void check_results();
int dca_main();
void dca_cancel_job(slave *sl);
static void dca_reset();
#endif