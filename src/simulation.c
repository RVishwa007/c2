#include "simulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void init_intersection(Intersection *intersection) {
    intersection->current_mode = MODE_NORMAL;
    intersection->cycle_count = 0;
    intersection->global_efficiency = 100.0f;
    intersection->global_throughput = 0;
    intersection->simulation_paused = false;
    intersection->current_speed = SPEED_NORMAL;
    intersection->manual_input_mode = false;
    intersection->manual_first_entry = false;
    intersection->last_emergency_cycle = 0;
    intersection->closure_active = false;
    intersection->closure_lane = NORTH;

    for (int i = 0; i < NUM_LANES; i++) {
        intersection->lanes[i].dir = (Direction)i;
        intersection->lanes[i].vehicle_count = rand() % 10 + 2;
        intersection->lanes[i].waiting_cycles = 0;
        intersection->lanes[i].green_time = 0;
        intersection->lanes[i].emergency_flag = false;
        intersection->lanes[i].recent_green_count = 0;
        intersection->lanes[i].incoming_rate = 1.0f;
        intersection->lanes[i].processed_vehicles = 0;
        intersection->lanes[i].congestion_score = 0.0f;
        intersection->lanes[i].efficiency_score = 100.0f;
        intersection->lanes[i].traffic_growth_rate = 0.0f;
        intersection->lanes[i].total_wait_time = 0;
        intersection->lanes[i].signal_state = SIGNAL_RED;
        intersection->lanes[i].ai_priority_score = 0.0f;
        intersection->lanes[i].last_processed_count = 0;
        intersection->lanes[i].assigned_green_time = 0;
        intersection->lanes[i].reason[0] = '\0';
    }
}

void generate_traffic(Intersection *intersection) {
    int max_new = 5;
    int arrival_chance = 70;
    
    switch (intersection->current_mode) {
        case MODE_RUSH_HOUR: 
            max_new = 25; 
            arrival_chance = 90;
            break;
        case MODE_NIGHT: 
            max_new = 2; 
            arrival_chance = 30;
            break;
        case MODE_RAIN: 
            max_new = 6; 
            arrival_chance = 60;
            break;
        case MODE_CLOSURE: 
            max_new = 8; 
            arrival_chance = 70;
            break;
        default: 
            max_new = 8; 
            arrival_chance = 60;
            break;
    }

    // If manual input mode is active, completely pause automated generation
    if (intersection->manual_input_mode) {
        max_new = 0;
        arrival_chance = 0;
    }

    for (int i = 0; i < NUM_LANES; i++) {
        intersection->lanes[i].last_arrivals = 0; // Reset
        
        // Generate traffic dynamically based on mode
        if (rand() % 100 < arrival_chance) {
            int new_cars = rand() % max_new;
            intersection->lanes[i].vehicle_count += new_cars;
            intersection->lanes[i].last_arrivals = new_cars;
            
            // Calculate growth rate (simple moving average approximation)
            intersection->lanes[i].traffic_growth_rate = 
                (intersection->lanes[i].traffic_growth_rate * 0.7f) + (new_cars * 0.3f);
        } else {
            intersection->lanes[i].traffic_growth_rate *= 0.7f;
        }
    }
}

void update_traffic_state(Intersection *intersection) {
    for (int i = 0; i < NUM_LANES; i++) {
        Lane *l = &intersection->lanes[i];
        
        if (l->signal_state == SIGNAL_RED) {
            l->waiting_cycles++;
            l->total_wait_time += l->vehicle_count;
        } else if (l->signal_state == SIGNAL_GREEN) {
            l->waiting_cycles = 0;
            l->recent_green_count++;
        }

        // Decay recent green memory if red
        if (l->signal_state == SIGNAL_RED && l->recent_green_count > 0) {
            if (rand() % 2 == 0) l->recent_green_count--;
        }

        l->congestion_score = (float)l->vehicle_count * 1.5f + (float)l->waiting_cycles * 2.0f;
    }
}

void process_vehicles(Intersection *intersection) {
    for (int i = 0; i < NUM_LANES; i++) {
        Lane *l = &intersection->lanes[i];
        l->last_processed_count = 0;
        l->last_cleared = 0;

        if (l->signal_state == SIGNAL_GREEN) {
            // Realistic throughput: 1 car every 1.5 seconds approximately
            float avg_pass_time = 1.5f;
            if (intersection->current_mode == MODE_RAIN) avg_pass_time = 2.5f;
            
            int process_rate = (int)(l->green_time / avg_pass_time);
            
            // Road closure implies the lane gets basically 0 green/flow, but handled in AI engine. 
            // If somehow it gets green, it still processes 0.
            if (intersection->closure_active && intersection->closure_lane == (Direction)i) {
                process_rate = 0;
            }

            int processed = (l->vehicle_count < process_rate) ? l->vehicle_count : process_rate;
            l->vehicle_count -= processed;
            l->processed_vehicles += processed;
            l->last_processed_count = processed;
            l->last_cleared = processed;
            intersection->global_throughput += processed;
            
            // Re-evaluate efficiency
            l->efficiency_score = (l->processed_vehicles / (float)(l->total_wait_time + 1)) * 100.0f;
            if(l->efficiency_score > 100.0f) l->efficiency_score = 100.0f;
        }
    }
}

void trigger_random_emergency(Intersection *intersection, EmergencyEvent *event, DashboardState *dash, Statistics *stats) {
    if (intersection->manual_input_mode) return; // Pause automated emergencies during manual entry

    // Trigger deterministically every 8 cycles
    if (!event->active && intersection->cycle_count > 0 && 
        (intersection->cycle_count - intersection->last_emergency_cycle >= 8)) {
        
        intersection->last_emergency_cycle = intersection->cycle_count;

        int dir = rand() % NUM_LANES;
        // Prevent emergency in closed lane
        if (intersection->closure_active && intersection->closure_lane == (Direction)dir) {
            dir = (dir + 1) % NUM_LANES;
        }

        intersection->lanes[dir].emergency_flag = true;
        event->lane_dir = (Direction)dir;
        event->active = true;
        event->cycles_remaining = 1; // Lasts exactly 1 cycle then clears
        
        stats->emergency_count++; // Count automated emergency exactly once here
        
        int type = rand() % 2;
        if (type == 0) strcpy(event->type, "Ambulance");
        else strcpy(event->type, "Fire Truck");

        snprintf(dash->alert_message, sizeof(dash->alert_message), "%s detected at %s!", 
            event->type, 
            dir == NORTH ? "NORTH" : dir == SOUTH ? "SOUTH" : dir == EAST ? "EAST" : "WEST");
    }
}
