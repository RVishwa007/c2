#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "traffic_types.h"

void calculate_priorities(Intersection *intersection, DashboardState *dash);
void assign_signals(Intersection *intersection, EmergencyEvent *event, DashboardState *dash);
void predict_congestion(Intersection *intersection, PredictionEngine *pred);

#endif // AI_ENGINE_H
