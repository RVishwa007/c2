#include "ui_dashboard.h"
#include <ncurses/ncurses.h>
#include <string.h>

void init_dashboard() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); // Hide cursor
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_WHITE, COLOR_RED); // Emergency
        init_pair(7, COLOR_BLUE, COLOR_BLACK);
    }
}

void cleanup_dashboard() {
    endwin();
}

void draw_box(int y, int x, int h, int w, const char* title, int color_pair) {
    attron(COLOR_PAIR(color_pair));
    mvhline(y, x, 0, w);
    mvhline(y+h, x, 0, w);
    mvvline(y, x, 0, h);
    mvvline(y, x+w, 0, h);
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x+w, ACS_URCORNER);
    mvaddch(y+h, x, ACS_LLCORNER);
    mvaddch(y+h, x+w, ACS_LRCORNER);
    mvprintw(y, x + 2, " %s ", title);
    attroff(COLOR_PAIR(color_pair));
}

void draw_dashboard(Intersection *intersection, DashboardState *dash, Statistics *stats, PredictionEngine *pred, EmergencyEvent *event) {
    clear();
    
    // Header
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(1, 30, "=== INTELLIGENT TRAFFIC MANAGEMENT SYSTEM ===");
    attroff(COLOR_PAIR(4) | A_BOLD);

    // 1. Visual Intersection Panel (y=3, x=2, h=20, w=50)
    draw_box(3, 2, 20, 50, "LIVE INTERSECTION", 4);
    
    // Draw simple cross
    for(int i = 5; i < 22; i++) {
        mvaddch(i, 20, '|');
        mvaddch(i, 30, '|');
    }
    for(int i = 3; i < 50; i++) {
        mvaddch(10, i, '-');
        mvaddch(16, i, '-');
    }
    
    // Traffic Signals
    mvprintw(9, 24, intersection->lanes[NORTH].signal_state == SIGNAL_GREEN ? "(G)" : "(R)");
    mvprintw(17, 24, intersection->lanes[SOUTH].signal_state == SIGNAL_GREEN ? "(G)" : "(R)");
    mvprintw(13, 16, intersection->lanes[WEST].signal_state == SIGNAL_GREEN ? "(G)" : "(R)");
    mvprintw(13, 32, intersection->lanes[EAST].signal_state == SIGNAL_GREEN ? "(G)" : "(R)");

    // Cars (Mock visualization)
    mvprintw(4, 24, "N:%d (+%d)", intersection->lanes[NORTH].vehicle_count, intersection->lanes[NORTH].last_arrivals);
    mvprintw(21, 24, "S:%d (+%d)", intersection->lanes[SOUTH].vehicle_count, intersection->lanes[SOUTH].last_arrivals);
    mvprintw(13, 2, "W:%d (+%d)", intersection->lanes[WEST].vehicle_count, intersection->lanes[WEST].last_arrivals);
    mvprintw(13, 38, "E:%d (+%d)", intersection->lanes[EAST].vehicle_count, intersection->lanes[EAST].last_arrivals);
    
    const char* car_n = intersection->lanes[NORTH].emergency_flag ? "[+]" : "(O)";
    const char* car_s = intersection->lanes[SOUTH].emergency_flag ? "[+]" : "(O)";
    const char* car_w = intersection->lanes[WEST].emergency_flag ? "[+]" : "(O)";
    const char* car_e = intersection->lanes[EAST].emergency_flag ? "[+]" : "(O)";
    
    if(intersection->closure_active) {
        if(intersection->closure_lane == NORTH) car_n = "/!\\";
        if(intersection->closure_lane == SOUTH) car_s = "/!\\";
        if(intersection->closure_lane == WEST) car_w = "/!\\";
        if(intersection->closure_lane == EAST) car_e = "/!\\";
    }

    if (intersection->lanes[NORTH].signal_state == SIGNAL_GREEN) attron(COLOR_PAIR(1)); else attron(COLOR_PAIR(2));
    mvprintw(8, 23, " %s ", car_n); attroff(COLOR_PAIR(1)|COLOR_PAIR(2));
    
    if (intersection->lanes[SOUTH].signal_state == SIGNAL_GREEN) attron(COLOR_PAIR(1)); else attron(COLOR_PAIR(2));
    mvprintw(18, 23, " %s ", car_s); attroff(COLOR_PAIR(1)|COLOR_PAIR(2));
    
    if (intersection->lanes[WEST].signal_state == SIGNAL_GREEN) attron(COLOR_PAIR(1)); else attron(COLOR_PAIR(2));
    mvprintw(13, 17, " %s ", car_w); attroff(COLOR_PAIR(1)|COLOR_PAIR(2));
    
    if (intersection->lanes[EAST].signal_state == SIGNAL_GREEN) attron(COLOR_PAIR(1)); else attron(COLOR_PAIR(2));
    mvprintw(13, 29, " %s ", car_e); attroff(COLOR_PAIR(1)|COLOR_PAIR(2));


    // 2. AI Reasoning Panel (y=3, x=54, h=8, w=44)
    draw_box(3, 54, 8, 44, "AI REASONING ENGINE", 5);
    
    int g_lane = -1;
    for(int i=0; i<4; i++) {
        if(intersection->lanes[i].signal_state == SIGNAL_GREEN) g_lane = i;
    }
    
    if(g_lane != -1) {
        mvprintw(5, 56, "Green Assigned: %s", g_lane == NORTH ? "NORTH" : g_lane == SOUTH ? "SOUTH" : g_lane == EAST ? "EAST" : "WEST");
        mvprintw(6, 56, "Priority Score: %.1f", intersection->lanes[g_lane].ai_priority_score);
        
        if (intersection->lanes[g_lane].emergency_flag) {
            attron(COLOR_PAIR(6) | A_BLINK);
            mvprintw(7, 56, "REASON: %s", intersection->lanes[g_lane].reason);
            attroff(COLOR_PAIR(6) | A_BLINK);
        } else if (intersection->lanes[g_lane].waiting_cycles >= 5) {
            attron(COLOR_PAIR(3));
            mvprintw(7, 56, "REASON: %s", intersection->lanes[g_lane].reason);
            attroff(COLOR_PAIR(3));
        } else {
            mvprintw(7, 56, "REASON: %s", intersection->lanes[g_lane].reason);
        }
        
        mvprintw(8, 56, "Passed Last Cycle: %d cars", intersection->lanes[g_lane].last_processed_count);
        mvprintw(9, 56, "Total Green Time: %d sec", intersection->lanes[g_lane].assigned_green_time);
    }

    // 3. Lane Details Panel (y=12, x=54, h=11, w=44)
    draw_box(12, 54, 11, 44, "LANE DETAILS", 3);
    
    const char* mode_str = "NORMAL";
    switch(intersection->current_mode) {
        case MODE_RUSH_HOUR: mode_str = "RUSH HOUR"; break;
        case MODE_NIGHT: mode_str = "NIGHT"; break;
        case MODE_RAIN: mode_str = "RAIN"; break;
        case MODE_CLOSURE: mode_str = "CLOSURE"; break;
        default: break;
    }
    mvprintw(14, 56, "Active Mode: %s", mode_str);
    
    const char* p_str = "LOW";
    int p_col = 1;
    if (pred->level == PREDICTION_MODERATE) { p_str = "MODERATE"; p_col = 3; }
    else if (pred->level == PREDICTION_HIGH) { p_str = "HIGH"; p_col = 2; }
    else if (pred->level == PREDICTION_CRITICAL) { p_str = "CRITICAL"; p_col = 6; }
    
    mvprintw(16, 56, "Congestion Prediction: ");
    attron(COLOR_PAIR(p_col) | A_BOLD);
    printw("%s", p_str);
    attroff(COLOR_PAIR(p_col) | A_BOLD);
    
    // Draw per-lane stats inside Lane Details box
    mvprintw(17, 56, "--- LANE CONGESTION SCORES ---");
    for(int i=0; i<4; i++) {
        const char* name = i == NORTH ? "NORTH" : i == SOUTH ? "SOUTH" : i == EAST ? "EAST" : "WEST";
        float c_perc = intersection->lanes[i].congestion_score * 2.0f;
        if(c_perc > 100.0f) c_perc = 100.0f;
        
        if (dash->highest_priority_lane == (Direction)i) {
            attron(COLOR_PAIR(1) | A_BOLD);
            mvprintw(18 + i, 56, "[*] %-5s: %3.0f%% Congested | Prio: %.1f", name, c_perc, intersection->lanes[i].ai_priority_score);
            attroff(COLOR_PAIR(1) | A_BOLD);
        } else {
            mvprintw(18 + i, 56, "    %-5s: %3.0f%% Congested | Prio: %.1f", name, c_perc, intersection->lanes[i].ai_priority_score);
        }
    }


    // 4. Alerts Panel (y=24, x=2, h=6, w=50)
    draw_box(24, 2, 6, 50, "SYSTEM ALERTS", 2);
    if (event->active || intersection->closure_active) {
        attron(COLOR_PAIR(6) | A_BLINK);
        mvprintw(26, 4, ">>> %s <<<", dash->alert_message);
        attroff(COLOR_PAIR(6) | A_BLINK);
    } else {
        attron(COLOR_PAIR(1));
        mvprintw(26, 4, "System Normal.");
        attroff(COLOR_PAIR(1));
    }
    if (intersection->simulation_paused) {
        attron(COLOR_PAIR(3) | A_BLINK);
        mvprintw(28, 4, "[SIMULATION PAUSED]");
        attroff(COLOR_PAIR(3) | A_BLINK);
    }

    // 5. Controls Panel (y=24, x=54, h=6, w=44)
    draw_box(24, 54, 6, 44, "CONTROLS", 7);
    mvprintw(25, 56, "S:Start/Pause  R:RushHour  N:Night");
    mvprintw(26, 56, "X:Closure      E:Emergency M:Manual");
    mvprintw(27, 56, "H:Analytics    C:Clear     Enter:Skip");
    const char* speed_str = "NORMAL";
    if (intersection->current_speed == SPEED_FAST) speed_str = "FAST";
    else if (intersection->current_speed == SPEED_SLOW) speed_str = "SLOW";
    mvprintw(28, 56, "F:Speed (Current: %s)", speed_str);

    refresh();
}

void draw_combined_analytics(Intersection *intersection, Statistics *stats, PredictionEngine *pred) {
    clear();
    draw_box(2, 5, 20, 70, "HISTORY & REAL-TIME ANALYTICS", 4);

    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(4, 8, "CURRENT SYSTEM HEALTH & HISTORY");
    attroff(COLOR_PAIR(3) | A_BOLD);

    mvprintw(6, 8, "Total Cycles Run:          %d", intersection->cycle_count);
    mvprintw(7, 8, "Total Vehicles Processed:  %d", stats->total_processed);
    mvprintw(8, 8, "Highest Recorded Wait:     %d cycles", stats->highest_waiting_time);
    mvprintw(9, 8, "Global Average Wait Time:  %.1f sec", stats->average_wait_time);
    mvprintw(10, 8, "Total Emergencies Handled: %d", stats->emergency_count);
    
    if (stats->total_processed == 0) {
        mvprintw(12, 8, "Busiest Lane History:      N/A (No Data)");
    } else {
        mvprintw(12, 8, "Busiest Lane History:      %s", 
            stats->busiest_lane == NORTH ? "NORTH" : 
            stats->busiest_lane == SOUTH ? "SOUTH" : 
            stats->busiest_lane == EAST ? "EAST" : "WEST");
    }

    const char* p_str = "LOW";
    if (intersection->cycle_count == 0) p_str = "N/A (No Data)";
    else if (pred->level == PREDICTION_MODERATE) p_str = "MODERATE";
    else if (pred->level == PREDICTION_HIGH) p_str = "HIGH";
    else if (pred->level == PREDICTION_CRITICAL) p_str = "CRITICAL";

    mvprintw(14, 8, "Predicted Congestion Trend: %s", p_str);

    attron(COLOR_PAIR(5));
    mvprintw(16, 8, "AI INFERENCE SUMMARY:");
    if (intersection->cycle_count == 0) {
        mvprintw(17, 8, "> Insufficient data. Run simulation to gather insights.");
    } else {
        mvprintw(17, 8, "> Adaptive priority algorithm confidence: %.1f%%", stats->ai_confidence);
        if (stats->average_wait_time < 5.0) {
            mvprintw(18, 8, "> Traffic is flowing freely with minimal delays.");
        } else if (stats->average_wait_time < 15.0) {
            mvprintw(18, 8, "> Traffic is experiencing moderate queuing.");
        } else {
            mvprintw(18, 8, "> Warning: High average wait times detected.");
        }
    }
    attroff(COLOR_PAIR(5));

    attron(COLOR_PAIR(1) | A_BLINK);
    mvprintw(20, 8, "Press any key to resume simulation...");
    attroff(COLOR_PAIR(1) | A_BLINK);

    refresh();
    while (getch() == ERR) {}
}
