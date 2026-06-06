#include "ai_engine.h"
#include <stdio.h>
#include <string.h>

const char* get_dir_name(Direction dir) {
    switch(dir) {
        case NORTH: return "NORTH";
        case SOUTH: return "SOUTH";
        case EAST:  return "EAST";
        case WEST:  return "WEST";
        default:    return "UNKNOWN";
    }
}

void calculate_priorities(Intersection *intersection, DashboardState *dash) {
    (void)dash; /* reserved for future dashboard feedback */
    for (int i = 0; i < NUM_LANES; i++) {
        Lane *l = &intersection->lanes[i];

        float density_weight = 1.0f;
        float starvation_weight = 2.0f;
        if (intersection->current_mode == MODE_RUSH_HOUR) {
            density_weight = 1.5f;
            starvation_weight = 1.5f;
        } else if (intersection->current_mode == MODE_NIGHT) {
            density_weight = 0.5f;
            starvation_weight = 3.0f;
        }

        float v_count_score = l->vehicle_count * density_weight;
        float wait_score = l->waiting_cycles * starvation_weight;
        float emergency_bonus = l->emergency_flag ? 500.0f : 0.0f;
        float congestion_trend_bonus = l->traffic_growth_rate * 5.0f;
        float recent_green_penalty = l->recent_green_count * 10.0f;

        l->ai_priority_score = v_count_score + wait_score + emergency_bonus + congestion_trend_bonus - recent_green_penalty;

        if (l->ai_priority_score < 0) l->ai_priority_score = 0;

        /* Populate human-readable reason for why this lane might be picked */
        if (l->emergency_flag) {
            strcpy(l->reason, "Emergency Override");
        } else if (l->vehicle_count > 25) {
            strcpy(l->reason, "Critical Density");
        } else if (l->vehicle_count > 10) {
            strcpy(l->reason, "High Density");
        } else if (l->waiting_cycles >= 3) {
            strcpy(l->reason, "Starvation Guarantee");
        } else {
            strcpy(l->reason, "Standard Flow");
        }
    }
}

void assign_signals(Intersection *intersection, EmergencyEvent *event, DashboardState *dash) {
    (void)event; // Suppress unused parameter warning
    int best_lane = -1;
    float max_score = -1.0f;
    bool starvation_forced = false;

    /* Check starvation (5 continuous red cycles to guarantee turn without getting stuck) */
    for (int i = 0; i < NUM_LANES; i++) {
        if (intersection->closure_active && intersection->closure_lane == (Direction)i) continue; // Skip closed lane
        
        if (intersection->lanes[i].waiting_cycles >= 5) {
            best_lane = i;
            starvation_forced = true;
            strcpy(intersection->lanes[i].reason, "Starvation Guarantee");
            break;
        }
    }

    if (!starvation_forced) {
        for (int i = 0; i < NUM_LANES; i++) {
            if (intersection->closure_active && intersection->closure_lane == (Direction)i) continue; // Skip closed lane
            
            if (intersection->lanes[i].ai_priority_score > max_score) {
                max_score = intersection->lanes[i].ai_priority_score;
                best_lane = i;
            }
        }
    }

    /* Record which lane the AI chose, for the dashboard panel */
    if (best_lane >= 0) {
        dash->highest_priority_lane = (Direction)best_lane;
    }

    /* Set signals */
    for (int i = 0; i < NUM_LANES; i++) {
        if (i == best_lane) {
            intersection->lanes[i].signal_state = SIGNAL_GREEN;
            
            if (intersection->lanes[i].emergency_flag) {
                intersection->lanes[i].green_time = 60;
            } else {
                float scaling_factor = 1.0f;
                if (intersection->current_mode == MODE_RUSH_HOUR) scaling_factor = 1.2f;
                
                intersection->lanes[i].green_time = 10 + (int)(intersection->lanes[i].vehicle_count * scaling_factor) + (intersection->lanes[i].waiting_cycles * 2);
                
                if (intersection->lanes[i].green_time > 60) intersection->lanes[i].green_time = 60;
                if (intersection->lanes[i].green_time < 10) intersection->lanes[i].green_time = 10;
            }
            
            intersection->lanes[i].assigned_green_time = intersection->lanes[i].green_time;

        } else {
            intersection->lanes[i].signal_state = SIGNAL_RED;
        }
    }
}

void predict_congestion(Intersection *intersection, PredictionEngine *pred) {
    float total_congestion = 0;
    for (int i = 0; i < NUM_LANES; i++) {
        total_congestion += intersection->lanes[i].congestion_score;
    }

    pred->history[pred->history_index] = total_congestion;
    pred->history_index = (pred->history_index + 1) % 100;

    /* Average last 5 entries */
    float sum = 0;
    int count = 0;
    for (int i = 1; i <= 5; i++) {
        int idx = (pred->history_index - i + 100) % 100;
        sum += pred->history[idx];
        count++;
    }
    float avg = sum / count;

    if      (avg < 20.0f)  pred->level = PREDICTION_LOW;
    else if (avg < 50.0f)  pred->level = PREDICTION_MODERATE;
    else if (avg < 100.0f) pred->level = PREDICTION_HIGH;
    else                   pred->level = PREDICTION_CRITICAL;
}
