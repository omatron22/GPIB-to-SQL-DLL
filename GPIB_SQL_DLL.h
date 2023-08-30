#ifndef GPIB_SQL_DLL_H
#define GPIB_SQL_DLL_H

#define MAX_CHANNELS 60  // Maximum number of channels to read from

__declspec(dllexport) int GetReadingsFromChannels(const char* address, const char* configCommand, double readings[MAX_CHANNELS]);
__declspec(dllexport) int GetPressureReadingsFromChannels(
    const char* address,
    const char* configCommand,
    double VoltMax, double VoltMin, 
    double PressureMax, double PressureMin, 
    double pressures[MAX_CHANNELS]
);
__declspec(dllexport) const char* SetDatabasePath(const char* path);
__declspec(dllexport) const char* GetLastSetMessage();

#endif // GPIB_SQL_DLL_H
