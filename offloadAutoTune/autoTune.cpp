#include <windows.h>
#include <stdint.h>
#include <iostream>
#include <math.h>
#include <chrono>
#include <thread>
#include "PeakBasedShift.cpp"

void setUpSerialConnection(HANDLE &);
void getData(HANDLE &serialHandle, int n, char *szBuff, DWORD dwBytesRead);
void writeData(HANDLE &serialHandle, int n, char *szBuff, DWORD dwBytesRead, float value);

int main()
{
    //Serial Connection Start
    HANDLE serialHandle;
    setUpSerialConnection(serialHandle);

    const int n = 3;
    char szBuff[n + 1] = {0};
    DWORD dwBytesRead = 0;
    //Serial Connection End


    //Frequency Shift Start
    uint16_t windowSize = 64, numWindows = 4;
    float **input = new float*[numWindows];
    for(int i = 0; i < numWindows; i++)
    {
        input[i] = new float[windowSize * 2];
        for(int j = 0; j < windowSize; j++)
        {
            input[i][j * 2] = 0;
            input[i][j * 2 + 1] = 0;
        }
    }
    float samplingRate = 1500, overlapPercent = .75, noiseThreshold = 99.5;
    uint16_t hop = (1 - overlapPercent) * windowSize;
    int freqChange;
    uint16_t totalOutputSize = (numWindows - 1) * hop + windowSize;

    PeakBasedShift peakBasedShift(windowSize, numWindows, samplingRate, overlapPercent, noiseThreshold);
    //Frequency Shift End

    //Array Trackers Start
    uint16_t writeLoc = 0, readLoc = 1, currWindow = 0;
    //Array Trackers End

    while(1)
    {
        //Get inputs and start time
        auto start = std::chrono::high_resolution_clock::now();
        getData(serialHandle, n, szBuff, dwBytesRead);

        uint16_t lo = szBuff[0] & 0xFF;
        uint16_t hi = szBuff[1] & 0xFF;
        uint16_t newData = lo | uint16_t(hi) << 8;

        freqChange = szBuff[2];

        //Store Inputs Start
        for(int i = 0; i < numWindows; i++)
        {
            if(writeLoc - (i * hop) >= 0 && writeLoc - (i * hop) < windowSize)
            {
                input[i][(writeLoc - (i * hop)) * 2] = newData;
                input[i][(writeLoc - (i * hop)) * 2 + 1] = 0;
            }
        }
        writeLoc++;
        if(writeLoc >= (currWindow * hop + windowSize))
        {
            peakBasedShift.captureWindow(input[currWindow], freqChange);
            currWindow++;
            if(currWindow >= numWindows) 
            {
                writeLoc = 0;
                currWindow = 0;
            }
        }
        //Store Inputs End

        //Write Outputs Start
        float outputValue = 0;
        for(int i = 0; i < numWindows; i++)
        {
            if(readLoc - (i * hop) >= 0 && readLoc - (i * hop) < windowSize)
                outputValue += input[i][(readLoc - (i * hop)) * 2] * .5 * (1.0 - cos(2 * MPI * (readLoc - (i * hop)) / (windowSize - 1)));
        }
        //Scale outputs back up to the analog range
        outputValue += 2048;
        readLoc++;
        if(readLoc >= (numWindows - 1) * hop + windowSize) readLoc = 0;
        writeData(serialHandle, n, szBuff, dwBytesRead, outputValue);
        //Write Outputs End

        //Delay the process until it reaches the sampling rate
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        int waitTime = ((1 / samplingRate) * 1e6) - microseconds;
        if(waitTime > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(waitTime));
    }
}

void setUpSerialConnection(HANDLE &serialHandle)
{
    serialHandle = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if(serialHandle == INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND) std::cout << "COM Port does not exist" << std::endl;
        else std::cout << "An error other than no com port has occurred" << std::endl;
        exit(-1);
    }
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    GetCommState(serialHandle, &serialParams);
    serialParams.BaudRate = 115200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    SetCommState(serialHandle, &serialParams);

    // Set timeouts
    COMMTIMEOUTS timeout = { 0 };
    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 50;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(serialHandle, &timeout);
}

void getData(HANDLE &serialHandle, int n, char *szBuff, DWORD dwBytesRead)
{
    if(!ReadFile(serialHandle, szBuff, n, &dwBytesRead, NULL))
    {
        std::cout << "Error Reading COM Port" << std::endl;
        exit(-1);
    }
}

void writeData(HANDLE &serialHandle, int n, char *szBuff, DWORD dwBytesRead, float value)
{
    uint16_t output = value;
    uint8_t lo = output & 0xFF;
    uint8_t hi = output >> 8;

    szBuff[0] = lo;
    szBuff[1] = hi;

    if(!WriteFile(serialHandle, szBuff, n, &dwBytesRead, NULL))
    {
        std::cout << "Error Writing COM Port" << std::endl;
        exit(-1);
    }
}