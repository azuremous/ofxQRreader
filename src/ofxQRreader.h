/*-----------------------------------------------------------/
ofxQRreader.h

github.com/azuremous
Created by Jung un Kim a.k.a azuremous on 2/5/20.
/----------------------------------------------------------*/

#pragma once
#include "hidapi.h"

const map<int,string> hdi_chars = {
    { 4, "a" },{ 5, "b" },{ 6, "c" },{ 7, "d" },{ 8, "e" },
    { 9, "f" },{ 10, "g" },{ 11, "h" },{ 12, "i" },{ 13, "j" },
    { 14, "k" },{ 15, "l" },{ 16, "m" },{ 17, "n" },{ 18, "o" },
    { 19, "p" },{ 20, "q" },{ 21, "r" },{ 22, "s" },{ 23, "t" },
    { 24, "u" },{ 25, "v" },{ 26, "w" },{ 27, "x" },{ 28, "y" },{ 29, "z" },
    { 30, "1" },{ 31, "2" },{ 32, "3" },{ 33, "4" },{ 34, "5" },
    { 35, "6" },{ 36, "7" },{ 37, "8" },{ 38, "9" },{ 39, "0" },
    { 44, " " },{ 45, "-" },{ 46, "=" },{ 47, "[" },{ 48, "]" },{ 49, "\\" },
    { 51, ";" },{ 52, "\'" },{ 53, "`" },{ 54, "," },{ 55, "." },{ 56, "/" }
};

const map<int,string> hdi_shifted_chars = { //US KEYBOARD LAYOUT
    { 4, "A" },{ 5, "B" },{ 6, "C" },{ 7, "D" },{ 8, "E" },
    { 9, "F" },{ 10, "G" },{ 11, "H" },{ 12, "I" },{ 13, "J" },
    { 14, "K" },{ 15, "L" },{ 16, "M" },{ 17, "N" },{ 18, "O" },
    { 19, "P" },{ 20, "Q" },{ 21, "R" },{ 22, "S" },{ 23, "T" },
    { 24, "U" },{ 25, "V" },{ 26, "W" },{ 27, "X" },{ 28, "Y" },{ 29, "Z" },
    { 30, "!" },{ 31, "@" },{ 32, "#" },{ 33, "$" },{ 34, "%" },
    { 35, "^" },{ 36, "&" },{ 37, "*" },{ 38, "(" },{ 39, ")" },
    { 44, " " },{ 45, "_" },{ 46, "+" },{ 47, "{" },{ 48, "}" },{ 49, "|" },
    { 51, ":" },{ 52, "\"" },{ 53, "~" },{ 54, "<" },{ 55, ">" },{ 56, "?" }
};

class ofxQRreader: public ofThread {
private:
    hid_device * handle;
    unsigned char * readBuffer;
    
    int res;
    bool useShift;
    string resultWord;
    
protected:
    
public:
    ofEvent<string>result;
    
    ofxQRreader():res(0),useShift(false),resultWord("")
    {
        handle = NULL;
        readBuffer = new unsigned char[8];
    }
    
    ~ofxQRreader(){
        stopThread();
        waitForThread(false);
        hid_close(handle);
        hid_exit();
    }
    
    bool setup(unsigned short vendor_id, unsigned short product_id, bool nonblock = false, const wchar_t * serial_number=NULL){
        int result = hid_init();
        handle = hid_open(vendor_id, product_id, serial_number);
        if(handle != NULL){
            wchar_t wstr[255];
            hid_get_manufacturer_string(handle, wstr, 255);
            printf("HID device connected. Manufacturer string: %ls\n", wstr);
            wstr[0] = 0x0000;
            hid_get_product_string(handle, wstr, 255);
            printf("product String: %ls\n", wstr);
            wstr[0] = 0x0000;
            hid_get_serial_number_string(handle, wstr, 255);
            printf("serial number String: %ls\n", wstr);
            hid_set_nonblocking(handle, nonblock?1:0);
            
            startThread();
            return true;
        }
        return false;
    }
    
    void setBlocking(bool nonblock){
        hid_set_nonblocking(handle, nonblock?1:0);
    }
    
    void threadedFunction(){
        while (isThreadRunning()) {
            read();
        }
    }
    
    void read(){
        res = hid_read(handle, readBuffer, 8);
        if(res > 0){
            for (int i = 0; i < res; i++){
                if(readBuffer[i] > 0 && readBuffer[i] != 40 && readBuffer[i] != 2){
                    int val = (int)readBuffer[i];
                    map <int,string> charRef = hdi_chars;
                    if(useShift) { charRef = hdi_shifted_chars; }
                    map<int,string>::const_iterator pos = charRef.find(val);
                    if (pos == hdi_chars.end()) {
                        printf("error\n");
                    } else {
                        string value = pos->second;
                        resultWord.append(value);
                        useShift = false;
                    }
                }else if(readBuffer[i] == 40){
                    notify(resultWord);
                }else if(readBuffer[i] == 2){
                    useShift = true;
                }
            }
        }
    }
    
    void notify(string word){
        string w = word;
        ofNotifyEvent(result, w, this);
        resultWord = "";
    }
    
    void close(){
        stopThread();
        waitForThread(false);
        hid_close(handle);
        hid_exit();
    }
    
};
