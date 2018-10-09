#pragma once

#include "common.h"


enum{
    U_RECV,
    U_SEND,
    CUR_STATUS,
    lucas_status,
};

void msleep(uint32_t timeout);

uint32_t get_os_time(void);

void lower_to_upper(char *buf, uint32_t len);