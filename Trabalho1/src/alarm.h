#ifndef ALARM_H
#define ALARM_H

#include "utils.h"
#include <stdio.h>
#include <signal.h>

static int alarmRing = false;

static inline void alarm_handler() {
    alarmRing = true;
}

#endif /* ALARM_H */
