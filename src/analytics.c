#include "analytics.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void init_statistics(Statistics *stats) {
    stats->total_processed = 0;
    stats->highest_waiting_time = 0;
    stats->busiest_lane = NORTH;
    stats->emergency_count = 0;
    stats->average_wait_time = 0.0f;
    stats->ai_confidence = 100.0f;
}

void update_statistics(Statistics *stats, Intersection *intersection, EmergencyEvent *event) {
    (void)event; // Suppress unused parameter warning
    stats->total_processed = intersection->global_throughput;
    
    int max_wait = 0;
    int max_cars = -1;
    
    for (int i = 0; i < NUM_LANES; i++) {
        if (intersection->lanes[i].waiting_cycles > max_wait) {
            max_wait = intersection->lanes[i].waiting_cycles;
        }
        
        if (intersection->lanes[i].vehicle_count > max_cars) {
            max_cars = intersection->lanes[i].vehicle_count;
            stats->busiest_lane = (Direction)i;
        }
    }
    
    if (max_wait > stats->highest_waiting_time) {
        stats->highest_waiting_time = max_wait;
    }
    
    int global_wait_sum = 0;
    for (int i = 0; i < NUM_LANES; i++) {
        global_wait_sum += intersection->lanes[i].total_wait_time;
    }
    if (stats->total_processed > 0) {
        stats->average_wait_time = (float)global_wait_sum / stats->total_processed;
    } else {
        stats->average_wait_time = 0.0f;
    }
    
    // AI Confidence simulation (reliable mapping based on wait time)
    float base_confidence = 100.0f - (stats->average_wait_time * 1.5f);
    if (base_confidence < 65.0f) {
        base_confidence = 65.0f + (rand() % 50) / 10.0f; // Keep it above 65% with slight fluctuation
    }
    if (base_confidence > 100.0f) base_confidence = 100.0f;
    stats->ai_confidence = base_confidence;
}

void log_cycle(Intersection *intersection, DashboardState *dash) {
    FILE *f = fopen("traffic_log.txt", "a");
    if (f) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        
        fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] Cycle: %d | Mode: %d | Total Throughput: %d\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec,
                intersection->cycle_count, intersection->current_mode, intersection->global_throughput);
        
        for (int i = 0; i < NUM_LANES; i++) {
            fprintf(f, "  Lane %d: Cars=%d, Wait=%d, State=%s\n", 
                    i, intersection->lanes[i].vehicle_count, intersection->lanes[i].waiting_cycles,
                    intersection->lanes[i].signal_state == SIGNAL_GREEN ? "GREEN" : "RED");
        }
        
        if (dash->alert_message[0] != '\0') {
            fprintf(f, "  ALERT: %s\n", dash->alert_message);
        }
        fprintf(f, "--------------------------------------------------------\n");
        fclose(f);
    }
}
