#include <string.h>
#include <stdlib.h>
#include "string_tools.h"

void cleanString(char *s){
    int len = strlen(s);
    int i;
    char *ptr=s;
    for(i=0;i<len;i++){
        if(s[i] == '\\'){
            i++;
            continue;
        }
        if(s[i] < 0x20){
            continue;
        }
        *ptr++ = s[i];
    }
    *ptr = '\0'; 
}

void reverse(char *str, int len) 
{ 
    int i=0, j=len-1, temp; 
    while (i<j) 
    { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; j--; 
    } 
} 

void cleanStringSpace(char *s)
{
    int len = strlen(s);
    int i;
    char *ptr=s;
    for(i=0;i<len;i++){
        if(s[i] == '\\'){
            i++;
            continue;
        }
        if(s[i] < 0x20){
            continue;
        }
        if(s[i] == ' '){
            *ptr++ = '_';
            continue;
        }
        *ptr++ = s[i];
    }
    *ptr = '\0'; 
}


int intToStr(int x, char str[], int d){
    int i = 0; 
    while (x) 
    { 
        str[i++] = (x%10) + '0'; 
        x = x/10; 
    } 
   
    while (i < d) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
}

void ftoa(float n, char *res){
    // Extract integer part 
    int ipart = (int)n; 
  
    // Extract floating part 
    float fpart = n - (float)ipart; 
  
    // convert integer part to string 
    int i = intToStr(ipart, res, 0); 
    res[i] = '.';  // add dot 
    fpart = fpart * pow(10, 3); 
    intToStr((int)fpart, res + i + 1, 3); 
}

void hideString(char* str, int sz, int max){
    int len = strlen(str);
    memset(str, 0 , sz);
    if(len > max) len = max;
    for(int i = 0; i < len; i++)
        strcat(str,"*");
}