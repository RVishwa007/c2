#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses/ncurses.h>
#include <unistd.h>
#include <locale.h>
#include "traffic_types.h"
#include "simulation.h"
#include "ai_engine.h"
#include "ui_dashboard.h"
#include "analytics.h"

int main() {
    srand(time(NULL));

    Intersection intersection;
    DashboardState dash = {0};
    Statistics stats;
    PredictionEngine pred = {0};
    EmergencyEvent event = {0};

    init_intersection(&intersection);
    init_statistics(&stats);
    init_dashboard();

    int tick_delay = 10000000; // 10 seconds initially
    bool tick_delay_skip = false;

    while (1) {
        tick_delay_skip = false;
        // Handle input
        int ch = getch();
        if (ch == 'Q' || ch == 'q') {
            break;
        } else if (ch == 'S' || ch == 's') {
            intersection.simulation_paused = !intersection.simulation_paused;
        } else if (ch == 'F' || ch == 'f') {
            if (intersection.current_speed == SPEED_NORMAL) {
                intersection.current_speed = SPEED_FAST;
                tick_delay = 5000000; // 5s
            } else if (intersection.current_speed == SPEED_FAST) {
                intersection.current_speed = SPEED_SLOW;
                tick_delay = 15000000; // 15s
            } else {
                intersection.current_speed = SPEED_NORMAL;
                tick_delay = 10000000; // 10s
            }
        } else if (ch == '\n' || ch == '\r') {
            // Instant skip to next cycle
            tick_delay_skip = true;
        } else if (ch == 'H' || ch == 'h') {
            draw_combined_analytics(&intersection, &stats, &pred);
            clear();
        } else if (ch == 'C' || ch == 'c') {
            FILE *f = fopen("traffic_log.txt", "w");
            if(f) fclose(f);
            init_statistics(&stats);
            init_intersection(&intersection);
        } else if (ch == 'R' || ch == 'r') {
            intersection.current_mode = (intersection.current_mode == MODE_RUSH_HOUR) ? MODE_NORMAL : MODE_RUSH_HOUR;
        } else if (ch == 'N' || ch == 'n') {
            intersection.current_mode = (intersection.current_mode == MODE_NIGHT) ? MODE_NORMAL : MODE_NIGHT;
        } else if (ch == 'X' || ch == 'x') {
            if (intersection.closure_active) {
                intersection.closure_active = false;
                intersection.current_mode = MODE_NORMAL;
                strcpy(dash.alert_message, "System Normal."); // Clear alert on exit
            } else {
                intersection.current_mode = MODE_CLOSURE;
                intersection.closure_active = true;
                nodelay(stdscr, FALSE); echo(); curs_set(1);
                mvprintw(0, 0, "Closure Lane (0=N, 1=S, 2=E, 3=W): ");
                int l = -1; scanw("%d", &l);
                if(l >= 0 && l <= 3) {
                    intersection.closure_lane = (Direction)l;
                    intersection.lanes[l].vehicle_count = rand() % 3; // Drop to 0-2
                }
                nodelay(stdscr, TRUE); noecho(); curs_set(0); clear();
            }
        } else if (ch == 'M' || ch == 'm') {
            intersection.manual_input_mode = !intersection.manual_input_mode;
            if (intersection.manual_input_mode) {
                intersection.manual_first_entry = true;
            }
        } else if (ch == 'E' || ch == 'e') {
            nodelay(stdscr, FALSE); echo(); curs_set(1);
            mvprintw(0, 0, "Emergency Lane (0=N, 1=S, 2=E, 3=W): ");
            int l = -1; scanw("%d", &l);
            nodelay(stdscr, TRUE); noecho(); curs_set(0); clear();
            
            if(l >= 0 && l <= 3) {
                int dir = l;
                intersection.lanes[dir].emergency_flag = true;
                event.lane_dir = (Direction)dir;
                event.active = true;
                event.cycles_remaining = 1; // Lasts exactly 1 cycle
                stats.emergency_count++; // Count manual emergency
                snprintf(dash.alert_message, sizeof(dash.alert_message), "MANUAL EMERGENCY IN %s!", 
                    dir == NORTH ? "NORTH" : dir == SOUTH ? "SOUTH" : dir == EAST ? "EAST" : "WEST");
            }
        }

        if (!intersection.simulation_paused) {
            if (intersection.manual_input_mode) {
                nodelay(stdscr, FALSE); echo(); curs_set(1);
                clear();
                mvprintw(2, 2, "=== MANUAL INPUT MODE ===");
                mvprintw(4, 2, "Enter new vehicles for NORTH lane (or -1 to exit manual mode): ");
                int n_cars = 0; scanw("%d", &n_cars);
                if (n_cars == -1) {
                    intersection.manual_input_mode = false;
                } else {
                    mvprintw(5, 2, "Enter new vehicles for SOUTH lane: ");
                    int s_cars = 0; scanw("%d", &s_cars);
                    mvprintw(6, 2, "Enter new vehicles for EAST lane: ");
                    int e_cars = 0; scanw("%d", &e_cars);
                    mvprintw(7, 2, "Enter new vehicles for WEST lane: ");
                    int w_cars = 0; scanw("%d", &w_cars);
                    
                    if (intersection.manual_first_entry) {
                        intersection.lanes[NORTH].vehicle_count = n_cars;
                        intersection.lanes[SOUTH].vehicle_count = s_cars;
                        intersection.lanes[EAST].vehicle_count = e_cars;
                        intersection.lanes[WEST].vehicle_count = w_cars;
                        intersection.manual_first_entry = false;
                    } else {
                        intersection.lanes[NORTH].vehicle_count += n_cars;
                        intersection.lanes[SOUTH].vehicle_count += s_cars;
                        intersection.lanes[EAST].vehicle_count += e_cars;
                        intersection.lanes[WEST].vehicle_count += w_cars;
                    }
                }
                nodelay(stdscr, TRUE); noecho(); curs_set(0); clear();
                
                if (!intersection.manual_input_mode) {
                    continue; // Skip this tick if they just exited
                }
            }
            intersection.cycle_count++;

            // --- Emergency lifecycle: clear previous cycle's emergency FIRST ---
            if (event.active) {
                event.cycles_remaining--;
                if (event.cycles_remaining <= 0) {
                    event.active = false;
                    // Clear the emergency flag on the lane
                    intersection.lanes[event.lane_dir].emergency_flag = false;
                }
            }

            // Set alert message based on current state
            if (intersection.closure_active) {
                snprintf(dash.alert_message, sizeof(dash.alert_message), "[ ROAD CLOSURE IN %s ]",
                    intersection.closure_lane == NORTH ? "NORTH" : 
                    intersection.closure_lane == SOUTH ? "SOUTH" : 
                    intersection.closure_lane == EAST ? "EAST" : "WEST");
            } else if (event.active) {
                // Keep existing emergency message
            } else {
                strcpy(dash.alert_message, "System Normal.");
            }

            // 1. Generate traffic
            generate_traffic(&intersection);
            
            // 2. Random Emergency logic (only if no active emergency)
            if (!event.active) {
                trigger_random_emergency(&intersection, &event, &dash, &stats);
            }

            // 3. Update States
            update_traffic_state(&intersection);

            // 4. AI Decision
            calculate_priorities(&intersection, &dash);
            assign_signals(&intersection, &event, &dash);

            // 5. Predict
            predict_congestion(&intersection, &pred);

            // 6. Process moving cars
            process_vehicles(&intersection);

            // 7. Stats & Logs
            update_statistics(&stats, &intersection, &event);
            log_cycle(&intersection, &dash);
        }

        // Draw Frame
        draw_dashboard(&intersection, &dash, &stats, &pred, &event);

        // Responsive sleep loop
        int slept = 0;
        while (slept < tick_delay && !tick_delay_skip) {
            int peek_ch = getch();
            if (peek_ch != ERR) {
                ungetch(peek_ch); // Put it back for the main loop to process
                break;
            }
            usleep(100000); // Sleep for 100ms
            slept += 100000;
        }
    }

    cleanup_dashboard();
    return 0;
}
