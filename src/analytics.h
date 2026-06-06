#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "traffic_types.h"

void init_statistics(Statistics *stats);
void update_statistics(Statistics *stats, Intersection *intersection, EmergencyEvent *event);
void log_cycle(Intersection *intersection, DashboardState *dash);

#endif // ANALYTICS_H
