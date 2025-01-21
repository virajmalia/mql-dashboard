#ifndef POSITIONS_H
#define POSITIONS_H

#include <GxEPD2_3C.h>
#include <Fonts/FreeSans12pt7b.h> // Header
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>  // Label

// Initialize display (Waveshare 4.2 inch)
GxEPD2_3C<GxEPD2_420c_Z21, GxEPD2_420c_Z21::HEIGHT>
    display(GxEPD2_420c_Z21(/*CS=*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

// Metric position structures
struct MetricPosition {
    int x;
    int y;
    int width;
    int height;
};

struct MetricPositions {
    MetricPosition balance;
    MetricPosition equity;
    MetricPosition margin;
    MetricPosition profit;
} metricPos;

// Display layout constants
const uint16_t DISPLAY_WIDTH = 300;
const uint16_t DISPLAY_HEIGHT = 400;
const int PADDING = 4;
const int HEADER_HEIGHT = 40;
const int SECTION_MARGIN = 15;
const int ROW_HEIGHT = 30;
const int COLUMN_WIDTH = DISPLAY_WIDTH / 2;
const int MAX_STRING_LENGTH = 32;

void calculateMetricPositions() {
    int baseY = HEADER_HEIGHT + SECTION_MARGIN / 2;
    
    // Balance metric position
    metricPos.balance = {
        0,
        baseY,
        COLUMN_WIDTH,
        ROW_HEIGHT + PADDING
    };
    
    // Equity metric position
    metricPos.equity = {
        COLUMN_WIDTH,
        baseY,
        COLUMN_WIDTH,
        ROW_HEIGHT + PADDING
    };
    
    // Margin metric position
    metricPos.margin = {
        0,
        baseY + ROW_HEIGHT + SECTION_MARGIN,
        COLUMN_WIDTH,
        ROW_HEIGHT + 2*PADDING
    };
    
    // Profit metric position
    metricPos.profit = {
        COLUMN_WIDTH,
        baseY + ROW_HEIGHT + SECTION_MARGIN,
        COLUMN_WIDTH,
        ROW_HEIGHT + 2*PADDING
    };
}

void drawMetricPartial(const char* label, const char* value, const MetricPosition& pos) {
    display.setPartialWindow(pos.x, pos.y, pos.width, pos.height);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        
        display.setFont(&FreeMono9pt7b);
        display.setCursor(pos.x + 5, pos.y + 10);
        display.setTextColor(GxEPD_RED);
        display.print(label);
        
        display.setCursor(pos.x + 5, pos.y + ROW_HEIGHT + 5);
        display.setFont(&FreeSans9pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.print(value);
    } while (display.nextPage());
}

void formatValue(char* buffer, float value, const char* format, const char* suffix = "") {
    if (buffer == nullptr) return;
    if (!isfinite(value) || value < 0) {
        strncpy(buffer, "N/A", MAX_STRING_LENGTH - 1);
        buffer[MAX_STRING_LENGTH - 1] = '\0';
        return;
    }
    snprintf(buffer, MAX_STRING_LENGTH - strlen(suffix), format, value);
    strncat(buffer, suffix, MAX_STRING_LENGTH - strlen(buffer) - 1);
}

void drawInitialDisplay() {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        
        // Draw header
        display.setFont(&FreeSans12pt7b);
        display.fillRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        
        const char* title = "MetaTrader 5";
        int16_t tbx, tby;
        uint16_t tbw, tbh;
        display.getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
        
        if (tbw <= DISPLAY_WIDTH) {
            display.setCursor((DISPLAY_WIDTH - tbw) / 2, HEADER_HEIGHT - 10);
            display.print(title);
        }
        
        display.setTextColor(GxEPD_BLACK);
        
        // Draw separators
        int secondSeparatorY = HEADER_HEIGHT + 2*(SECTION_MARGIN + ROW_HEIGHT) + 7;
        
        display.drawFastHLine(SECTION_MARGIN, secondSeparatorY, 
                            DISPLAY_WIDTH - 2 * SECTION_MARGIN, GxEPD_BLACK);
    } while (display.nextPage());
}

void updateMetrics(const int updateParam) {
    char buffer[MAX_STRING_LENGTH];
    
    // Update balance if changed
    //if (finData.balance != prevFinData.balance) {
      if (updateParam == 1){
        formatValue(buffer, finData.balance, "$%.2f");
        drawMetricPartial("Balance", buffer, metricPos.balance);
        prevFinData.balance = finData.balance;
      }
    //}
    
    // Update equity if changed
    //if (finData.equity != prevFinData.equity) {
      if (updateParam == 2){
        formatValue(buffer, finData.equity, "$%.2f");
        drawMetricPartial("Equity", buffer, metricPos.equity);
        prevFinData.equity = finData.equity;
      }
    //}
    
    // Update margin if changed
    //if (finData.margin != prevFinData.margin) {
      if (updateParam == 3){
        formatValue(buffer, finData.margin, "$%.2f");
        drawMetricPartial("Margin", buffer, metricPos.margin);
        prevFinData.margin = finData.margin;
      }
    //}
    
    // Update profit if changed
    //if (finData.profit != prevFinData.profit) {
      if (updateParam == 4) {
        formatValue(buffer, finData.profit, "$%.2f");
        drawMetricPartial("Profit", buffer, metricPos.profit);
        prevFinData.profit = finData.profit;
      }
    //}
}

#endif  // POSITIONS_H
