#ifndef TRAFFIC_TYPES_H
#define TRAFFIC_TYPES_H

#include <stdbool.h>

typedef enum {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NUM_LANES
} Direction;

typedef enum {
    SIGNAL_RED,
    SIGNAL_GREEN,
    SIGNAL_YELLOW
} SignalState;

typedef enum {
    MODE_NORMAL,
    MODE_RUSH_HOUR,
    MODE_NIGHT,
    MODE_RAIN,
    MODE_CLOSURE
} SimMode;

typedef enum {
    PREDICTION_LOW,
    PREDICTION_MODERATE,
    PREDICTION_HIGH,
    PREDICTION_CRITICAL
} CongestionLevel;

typedef enum {
    SPEED_FAST,
    SPEED_NORMAL,
    SPEED_SLOW
} SimSpeed;

typedef struct {
    Direction dir;
    int vehicle_count;
    int waiting_cycles;
    int green_time;
    bool emergency_flag;
    int recent_green_count;
    float incoming_rate;
    int processed_vehicles;
    float congestion_score;
    float efficiency_score;
    float traffic_growth_rate;
    int total_wait_time;
    SignalState signal_state;
    float ai_priority_score;
    int last_processed_count;
    int assigned_green_time;
    int last_arrivals;
    int last_cleared;
    char reason[64];
} Lane;

typedef struct {
    Lane lanes[NUM_LANES];
    SimMode current_mode;
    int cycle_count;
    float global_efficiency;
    int global_throughput;
    bool simulation_paused;
    SimSpeed current_speed;
    bool manual_input_mode;
    bool manual_first_entry;
    Direction closure_lane;
    bool closure_active;
    int last_emergency_cycle;
} Intersection;

typedef struct {
    Direction lane_dir;
    char type[32]; // "Ambulance", "Police", "Fire Truck"
    bool active;
    int cycles_remaining;
} EmergencyEvent;

typedef struct {
    int total_processed;
    int highest_waiting_time;
    Direction busiest_lane;
    int emergency_count;
    float average_wait_time;
    float ai_confidence;
} Statistics;

typedef struct {
    float history[100]; // Rolling history of congestion
    int history_index;
    CongestionLevel level;
} PredictionEngine;

typedef struct {
    char ai_reasoning[256];
    char alert_message[128];
    int ticks;
    Direction highest_priority_lane;
} DashboardState;

#endif // TRAFFIC_TYPES_H
