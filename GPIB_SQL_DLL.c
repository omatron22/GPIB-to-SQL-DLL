#include "visa.h"
#include "sqlite3.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>

#define MAX_CHANNELS 60  // Define the maximum number of channels that can be read
#define DEFAULT_DB_PATH "C:\\_LabTech\\Agilent DLL Data\\readings.db"  // Default path for the database

char dbFilePath[512] = DEFAULT_DB_PATH;  // Current database path
char lastSetMessage[512] = "";  // Global variable to store the last action message

// Function to recursively ensure that a directory (and its parents) exist.
void ensureDirectoryExistsRecursively(const char* dir) {
    char tmp[512];
    char* p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    
    // Check for single backslash at the end and remove if found
    if (tmp[len - 1] == '\\') {
        tmp[len - 1] = 0;
        len--;
    }
    
    // Check for double backslash at the end and remove if found
    if (len > 1 && tmp[len - 1] == '\\' && tmp[len - 2] == '\\') {
        tmp[len - 2] = 0;
    }

    // Create the directory if it doesn't exist
    for (p = tmp + 1; *p; p++) {
        if (*p == '\\') {
            *p = 0;
            DWORD dwAttrib = GetFileAttributes(tmp);
            if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
                CreateDirectory(tmp, NULL);
            }
            *p = '\\';
        }
    }
    DWORD dwAttrib = GetFileAttributes(tmp);
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        CreateDirectory(tmp, NULL);
    }
}

// Sets the database path. If an invalid or empty path is provided, defaults to DEFAULT_DB_PATH.
// Also updates the `lastSetMessage`.
__declspec(dllexport) const char* SetDatabasePath(const char* path) {
    const char *allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.:\\ ";

    // Check if the path is NULL or empty and set to default
    if (path == NULL || strlen(path) == 0) {
        strncpy(dbFilePath, DEFAULT_DB_PATH, sizeof(dbFilePath) - 1);
        dbFilePath[sizeof(dbFilePath) - 1] = '\0';  // Ensure null-termination
        strncpy(lastSetMessage, "Path is empty or NULL. Default path has been set.", sizeof(lastSetMessage) - 1);
        return dbFilePath;
    }

    // Validate the path against a whitelist of allowed characters
    for (size_t i = 0; i < strlen(path); i++) {
        if (strchr(allowedChars, path[i]) == NULL) {
            strncpy(dbFilePath, DEFAULT_DB_PATH, sizeof(dbFilePath) - 1);
            dbFilePath[sizeof(dbFilePath) - 1] = '\0';  // Ensure null-termination
            strncpy(lastSetMessage, "Path contains invalid characters. Default path has been set.", sizeof(lastSetMessage) - 1);
            return dbFilePath;
        }
    }

    // Update the database path and return a success message
    strncpy(dbFilePath, path, sizeof(dbFilePath) - 1);
    dbFilePath[sizeof(dbFilePath) - 1] = '\0';  // Ensure null-termination
    strncpy(lastSetMessage, "Database path has been updated successfully.", sizeof(lastSetMessage) - 1);
    
    return dbFilePath;
}

// Retrieves the last set message from `SetDatabasePath`.
__declspec(dllexport) const char* GetLastSetMessage() {
    return lastSetMessage;
}

// Helper function to extract individual channels and configuration type from the configCommand
void extractChannelsAndConfigType(const char* configCommand, char channels[MAX_CHANNELS][20], char* configType, int* num_channels) {
    // Find the start and end of the channel list in the command
    char* start = strstr(configCommand, "(@");
    char* end = strstr(configCommand, ")");
    char configChannels[300] = {0};
    strncpy(configChannels, start + 2, end - start - 2);

    // Split the channels using comma as the delimiter
    char* token = strtok(configChannels, ",");
    *num_channels = 0;
    while (token != NULL) {
        strcpy(channels[*num_channels], token);
        (*num_channels)++;
        token = strtok(NULL, ",");
    }

    // Extract the configuration type from the command
    strncpy(configType, configCommand, start - configCommand);
    configType[start - configCommand] = '\0';
}

// Function to store readings in a SQLite database
int StoreReadingsInDatabase(double readings[], int num_readings, const char* address, const char* configCommand) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;

    // Ensure directory for the database exists
    char directoryPath[512];
    strcpy(directoryPath, dbFilePath);
    char* lastSlash = strrchr(directoryPath, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
    }
    ensureDirectoryExistsRecursively(directoryPath);

    // Open database and handle potential errors
    rc = sqlite3_open(dbFilePath, &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    // Create table if it doesn't exist
    const char *sql_create = "CREATE TABLE IF NOT EXISTS Readings (Timestamp TEXT NOT NULL, Reading REAL, Channel INT, Configuration TEXT);";
    rc = sqlite3_exec(db, sql_create, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -1;
    }

    // Extract channels and configuration type from the provided command
    char channels[MAX_CHANNELS][20];
    char configType[100];
    int num_channels = 0;
    extractChannelsAndConfigType(configCommand, channels, configType, &num_channels);

    // Insert readings into the database
    for (int i = 0; i < num_readings && i < num_channels; i++) {
        char sql_insert[500];
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char modifiedConfig[500];
        snprintf(modifiedConfig, sizeof(modifiedConfig), "%s(@%s)", configType, channels[i]);

        // Format and execute the SQL command to insert the reading
        snprintf(sql_insert, sizeof(sql_insert), "INSERT INTO Readings(Timestamp, Reading, Channel, Configuration) VALUES('%04d-%02d-%02d %02d:%02d:%02d', %f, %d, '%s');",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, readings[i], atoi(channels[i]), modifiedConfig);

        rc = sqlite3_exec(db, sql_insert, 0, 0, &err_msg);
        if (rc != SQLITE_OK) {
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return -1;
        }
    }

    sqlite3_close(db);  // Close the database connection
    return 0;  // Return success
}

// Function to get readings from specified channels
__declspec(dllexport) int GetReadingsFromChannels(const char* address, const char* configCommand, double readings[MAX_CHANNELS]) {
    ViStatus status;
    ViSession defaultRM, vi;
    char response[1500];  // Buffer to store the response from the device

    // Open a session to the VISA Resource Manager
    status = viOpenDefaultRM(&defaultRM);
    if (status < VI_SUCCESS) return -1;

    // Open a session to the specified VISA instrument
    status = viOpen(defaultRM, (ViRsrc)address, VI_NULL, VI_NULL, &vi);
    if (status < VI_SUCCESS) {
        viClose(defaultRM);
        return -1;
    }

    // Configure the instrument using the provided command
    status = viWrite(vi, (ViBuf)configCommand, strlen(configCommand), VI_NULL);
    if (status < VI_SUCCESS) {
        viClose(vi);
        viClose(defaultRM);
        return -1;
    }

    // Command to fetch the readings from the instrument
    const char* fetchCommand = "READ?";
    status = viWrite(vi, (ViBuf)fetchCommand, strlen(fetchCommand), VI_NULL);
    if (status < VI_SUCCESS) {
        viClose(vi);
        viClose(defaultRM);
        return -1;
    }

    // Read the response from the instrument
    ViUInt32 bytesRead;
    status = viRead(vi, (ViPBuf)response, sizeof(response) - 1, &bytesRead);
    if (status < VI_SUCCESS) {
        viClose(vi);
        viClose(defaultRM);
        return -1;
    }

    // Split the response by commas and store the readings in the provided array
    char *token = strtok(response, ",");
    int idx = 0;
    while (token != NULL && idx < MAX_CHANNELS) {
        readings[idx++] = atof(token);
        token = strtok(NULL, ",");
    }

    // If there are fewer readings than the maximum allowed, fill the rest with zeros
    for (int i = idx; i < MAX_CHANNELS; i++) {
        readings[i] = 0;
    }

    // Store the readings in the database
    StoreReadingsInDatabase(readings, idx, address, configCommand);

    // Close the VISA sessions
    viClose(vi);
    viClose(defaultRM);

    return idx;  // Return the number of readings obtained
}

// Function to get readings from specified channels and convert them to pressure
__declspec(dllexport) int GetPressureReadingsFromChannels(
    const char* address,
    const char* configCommand,
    double VoltMax, double VoltMin, 
    double PressureMax, double PressureMin, 
    double pressures[MAX_CHANNELS]
) {
    ViStatus status;
    ViSession defaultRM, vi;
    char response[1500];  // Buffer to store the response from the device

    // Open a session to the VISA Resource Manager
    status = viOpenDefaultRM(&defaultRM);
    if (status < VI_SUCCESS) return -1;

    // Open a session to the specified VISA instrument
    status = viOpen(defaultRM, (ViRsrc)address, VI_NULL, VI_NULL, &vi);
    if (status < VI_SUCCESS) {
        viClose(defaultRM);
        return -1;
    }

    // Configure the instrument using the provided command
    status = viWrite(vi, (ViBuf)configCommand, strlen(configCommand), VI_NULL);
    if (status < VI_SUCCESS) {
        viClose(vi);
        viClose(defaultRM);
        return -1;
    }

    // Command to fetch the readings from the instrument
    const char* fetchCommand = "READ?";
    status = viWrite(vi, (ViBuf)fetchCommand, strlen(fetchCommand), VI_NULL);
    if (status < VI_SUCCESS) {
        viClose(vi);
        viClose(defaultRM);
        return -1;
    }

    // Read the response from the instrument
    ViUInt32 bytesRead;
    status = viRead(vi, (ViPBuf)response, sizeof(response) - 1, &bytesRead);
    if (status < VI_SUCCESS) {
        viClose(vi);
        viClose(defaultRM);
        return -1;
    }

    // Calculate the conversion factor for voltage to pressure
    double conversionFactor = (PressureMax - PressureMin) / (VoltMax - VoltMin);

    // Split the response by commas, convert the voltages to pressures, and store them in the provided array
    char *token = strtok(response, ",");
    int idx = 0;
    while (token != NULL && idx < MAX_CHANNELS) {
        double voltage = atof(token);
        pressures[idx++] = conversionFactor * voltage;
        token = strtok(NULL, ",");
    }

    // If there are fewer readings than the maximum allowed, fill the rest with zeros
    for (int i = idx; i < MAX_CHANNELS; i++) {
        pressures[i] = 0;
    }

    // Store the readings in the database
    StoreReadingsInDatabase(pressures, idx, address, configCommand);

    // Close the VISA sessions
    viClose(vi);
    viClose(defaultRM);

    return idx;  // Return the number of readings obtained
}
