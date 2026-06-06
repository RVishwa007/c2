#ifndef SIMULATION_H
#define SIMULATION_H

#include "traffic_types.h"

void init_intersection(Intersection *intersection);
void generate_traffic(Intersection *intersection);
void update_traffic_state(Intersection *intersection);
void process_vehicles(Intersection *intersection);
void trigger_random_emergency(Intersection *intersection, EmergencyEvent *event, DashboardState *dash, Statistics *stats);

#endif // SIMULATION_H
