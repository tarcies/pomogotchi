#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

void initWebServer();
void handleWebServer();

void setTheme(String mode);
void loadThemeFromSD();
String getTheme();

#endif
