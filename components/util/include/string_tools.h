#pragma once

#include "cJSON.h"
#include "math.h"

void cleanString(char *s);
void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res);
void cleanStringSpace(char *s);
void hideString(char* str, int sz, int max);