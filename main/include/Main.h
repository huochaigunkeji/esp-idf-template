#ifndef __MAIN_H__
#define __MAIN_H__

#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

extern EventGroupHandle_t smartconfig_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
#define     LINKED_BIT      (BIT0)
#define     CONNECTED_BIT   (BIT1)
#define     LINK_OVER_BIT   (BIT2)

#ifdef __cplusplus
}
#endif

#endif

