#ifndef UI_DASHBOARD_H
#define UI_DASHBOARD_H

#include "traffic_types.h"

void init_dashboard();
void cleanup_dashboard();
void draw_dashboard(Intersection *intersection, DashboardState *dash, Statistics *stats, PredictionEngine *pred, EmergencyEvent *event);
void draw_combined_analytics(Intersection *intersection, Statistics *stats, PredictionEngine *pred);

#endif // UI_DASHBOARD_H
