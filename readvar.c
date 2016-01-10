// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "config.h"
#include "optical_eye_utils.h"

typedef struct _id2str {
    unsigned int id;
    char const *description;
} id2str;

#define unknown_variable_description "Meter response seen, variable unknown"

// Units, provided by Erik Jensen. Unit 51 was empty, I made the guess that
// it is used for variable length integer values, for things that can be
// counted.

static char* units[] = {
    "Unit0",                                          // 0 (Was empty).
    "Wh",       "kWh",      "MWh",      "GWh",        // 1-4 Power.
    "j",        "kj",       "Mj",       "Gj",         // 5-8 Energy.
    "Cal",      "kCal",     "Mcal",     "Gcal",       // 9-12 Heat energy.
    "varh",     "kvarh",    "Mvarh",    "Gvarh",      // 13-16 Reactive energy.
    "VAh",      "kVAh",     "MVAh",     "GVAh",       // 17-20 Energy.
    "kW",       "kW",       "MW",       "GW",         // 21-24 Power.
    "kvar",     "kvar",     "Mvar",     "Gvar",       // 25-28 Reactive power.
    "VA",       "kVA",      "MVA",      "GVA",        // 29-32 Power.
    "V",        "A",        "kV",       "kA",         // 33-36 Voltage/Current.
    "C",        "K",                                  // 37-38 Temperature.
    "l",        "m3",                                 // 39-40 Volume.
    "l/h",      "m3/h",                               // 41-42 Flow of volume.
    "m3xC",                                           // 43 ?
    "ton",                                            // 44 Mass.
    "ton/h",                                          // 45 Flow of mass.
    "h",                                              // 46 Time.
    "hh,mm,ss", "yy,mm,dd", "yyyy,mm,dd", "mm,dd",    // 47-50 Composite time.
    "int",                                            // 51 Counts (was empty).
    "bar",                                            // 52 Pressure.
    "RTC",                                            // 53 Composite time.
    "ASCII",                                          // 54 Textual data.
    "m3 x 10", "ton x 10", "GJ x 10",                 // 55-57 "10x units".
    "minutes",                                        // 58 Time.
    "Bitfield",                                       // 59 Binary data.
    "s",        "ms",       "days",                   // 60-62 Time.
    "RTC-Q",    "Datetime"                            // 63-64 Composite time.
};
static int units_length = 65;

typedef enum _UNIT_REPRESENTATION {
    UR_UNKNOWN, UR_INT, UR_FLOAT, UR_BYTE, UR_TIME,
    UR_DATE2, UR_DATE3, UR_DATE4,
    UR_ASCII, UR_BITS, UR_RTC, UR_RTCQ, UR_DATETIME,
    UR_VARINT
} UNIT_REPRESENTATION;

static UNIT_REPRESENTATION unit_representation[] = {
    UR_UNKNOWN,                                       // 0
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 1-4 Power.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 5-8 Energy.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 9-12 Heat energy.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 13-16 Reactive energy.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 17-20 Energy.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 21-24 Power.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 25-28 Reactive power.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 29-32 Power.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,   UR_FLOAT,     // 33-36 Voltage/Current.
    UR_FLOAT,   UR_FLOAT,                             // 37-38 Temperature.
    UR_FLOAT,   UR_FLOAT,                             // 39-40 Volume.
    UR_FLOAT,   UR_FLOAT,                             // 41-42 Flow of volume.
    UR_FLOAT,                                         // 43 ?
    UR_FLOAT,                                         // 44 Mass.
    UR_FLOAT,                                         // 45 Flow of mass.
    UR_BYTE,                                          // 46 Time.
    UR_TIME,    UR_DATE3,   UR_DATE4,   UR_DATE2,     // 47-50 Composite time.
    UR_VARINT,                                        // 51
    UR_FLOAT,                                         // 52 Pressure.
    UR_RTC,                                           // 53 Composite time.
    UR_ASCII,                                         // 54 Textual data.
    UR_FLOAT,   UR_FLOAT,   UR_FLOAT,                 // 55-57 "10x units".
    UR_BYTE,                                          // 58 Time.
    UR_BITS,                                          // 59 Binary data.
    UR_BYTE,    UR_INT,     UR_INT,                   // 60-62 Time.
    UR_RTCQ,    UR_DATETIME                           // 63-64 Composite time.
};

// Variable identifiers and descriptions, provided by Kim Djernaes.

static id2str var_data[] = {
    {   1, "Active energy A14"},
    {   2, "Active energy A23"},
    {   3, "Reactive energy R12"},
    {   4, "Reactive energy R34"},
    {   5, "Reactive energy R1"},
    {   6, "Reactive energy R4"},
    {  13, "Active energy A14, test xxx.xxxx"},
    {  14, "Active energy A23, test xxx.xxxx"},
    {  15, "Reactive energy R12, test xxx.xxxx"},
    {  16, "Reactive energy R34, test xxx.xxxx"},
    {  17, "Resettable counter A14"},
    {  18, "Resettable counter A23"},
    {  19, "Active energy A14 Tariff 1"},
    {  20, "Active energy A23 Tariff 1"},
    {  21, "Reactive energy R12 Tariff 1"},
    {  22, "Reactive energy R34 Tariff 1"},
    {  23, "Active energy A14 Tariff 2"},
    {  24, "Active energy A23 Tariff 2"},
    {  25, "Reactive energy R12 Tariff 2"},
    {  26, "Reactive energy R34 Tariff 2"},
    {  27, "Active energy A14 Tariff 3"},
    {  28, "Active energy A23 Tariff 3"},
    {  29, "Reactive energy R12 Tariff 3"},
    {  30, "Reactive energy R34 Tariff 3"},
    {  31, "Active energy A14 Tariff 4"},
    {  32, "Active energy A23 Tariff 4"},
    {  33, "Reactive energy R12 Tariff 4"},
    {  34, "Reactive energy R34 Tariff 4"},
    {  35, unknown_variable_description},
    {  36, unknown_variable_description},
    {  37, unknown_variable_description},
    {  38, unknown_variable_description},
    {  39, "Max power P14"},
    {  40, "Max power P23"},
    {  41, "Max power Q12"},
    {  42, "Max power Q34"},
    {  43, "Accumulated max power P14"},
    {  44, "Accumulated max power P23"},
    {  45, "Accumulated max power Q12"},
    {  46, "Accumulated max power Q34"},
    {  47, "Number of debiting periods"},
    {  50, "Meter status"},
    {  51, "Meter number 1"},
    {  52, "Meter number 2"},
    {  53, "Meter number 3"},
    {  54, "Configurations number 1"},
    {  55, "Configurations number 2"},
    {  56, "Configurations number 3"},
    {  57, "Special Data 1"},
    {  58, "Pulse input"},
    { 199, unknown_variable_description},
    { 222, unknown_variable_description},
    {1001, "Serial number"},
    {1002, "Clock"},
    {1003, "Date"},
    {1004, "Hour counter"},
    {1005, "Software revision"},
    {1010, "Total meter number"},
    {1021, "Special Data 2"},
    {1023, "Actual power P14"},
    {1024, "Actual power P23"},
    {1025, "Actual power Q12"},
    {1026, "Actual power Q34"},
    {1027, unknown_variable_description},
    {1028, unknown_variable_description},
    {1029, "Configurations number 4"},
    {1030, unknown_variable_description},
    {1031, "Active energy A1234"},
    {1032, "Operation mode"},
    {1033, "Max power P14 Tariff 1"},
    {1034, unknown_variable_description},
    {1035, unknown_variable_description},
    {1036, "Max power P14 Tariff 2"},
    {1037, unknown_variable_description},
    {1038, unknown_variable_description},
    {1039, "Power threshold value"},
    {1040, "Power threshold counter"},
    // {1045, "RTC status"}, Meter responds as unknown.
    {1046, "VCOPCO status"},
    {1047, "RTC"},
    {1049, "Max power P14 RTC"},
    {1050, "Max power P14 Tariff 1 RTC"},
    {1051, "Max power P14 Tariff 2 RTC"},
    {1054, "Voltage L1"},
    {1055, "Voltage L2"},
    {1056, "Voltage L3"},
    {1058, "Type number"},
    {1059, "Active energy A14 Tariff 5"},
    {1060, "Active energy A14 Tariff 6"},
    {1061, "Active energy A14 Tariff 7"},
    {1062, "Active energy A14 Tariff 8"},
    {1063, "Active energy A23 Tariff 5"},
    {1064, "Active energy A23 Tariff 6"},
    {1065, "Active energy A23 Tariff 7"},
    {1066, "Active energy A23 Tariff 8"},
    {1067, "Reactive energy R12 Tariff 5"},
    {1068, "Reactive energy R12 Tariff 6"},
    {1069, "Reactive energy R12 Tariff 7"},
    {1070, "Reactive energy R12 Tariff 8"},
    {1071, "Reactive energy R34 Tariff 5"},
    {1072, "Reactive energy R34 Tariff 6"},
    {1073, "Reactive energy R34 Tariff 7"},
    {1074, "Reactive energy R34 Tariff 8"},
    {1075, "Configurations number 5"},
    {1076, "Current L1"},
    {1077, "Current L2"},
    {1078, "Current L3"},
    {1079, unknown_variable_description},
    {1080, "Actual power P14 L1"},
    {1081, "Actual power P14 L2"},
    {1082, "Actual power P14 L3"},
    {1083, "ROM checksum"},
    // {1084, "Voltage extremity"}, Meter responds as unknown.
    // {1085, "Voltage event"}, Meter responds as unknown.
    {1086, "Logger status"},
    {1087, "Connection status"},
    {1088, "Connection feedback"},
    {1089, unknown_variable_description},
    {1090, unknown_variable_description},
    {1091, unknown_variable_description},
    {1092, unknown_variable_description},
    {1101, unknown_variable_description},
    {1102, "Module port I/O configuration"},
    {1108, unknown_variable_description},
    {1109, unknown_variable_description},
    {1110, unknown_variable_description},
    {1111, unknown_variable_description},
    {1112, unknown_variable_description},
    {1113, unknown_variable_description},
    {1114, unknown_variable_description},
    {1115, unknown_variable_description},
    {1116, unknown_variable_description},
    {1117, unknown_variable_description},
    {1118, unknown_variable_description},
    {1119, unknown_variable_description},
    {1120, unknown_variable_description},
    {1121, unknown_variable_description},
    {1122, unknown_variable_description},
    {1126, unknown_variable_description},
    {1175, unknown_variable_description},
    {1181, unknown_variable_description},
    {1189, unknown_variable_description},
    {1190, unknown_variable_description},
    {1191, unknown_variable_description},
    {1192, unknown_variable_description},
    {1193, unknown_variable_description},
    {1194, unknown_variable_description},
    {1195, unknown_variable_description},
    {1196, unknown_variable_description},
    {1197, unknown_variable_description},
    {1198, unknown_variable_description},
    {1199, unknown_variable_description},
    {1200, unknown_variable_description},
    {1201, unknown_variable_description},
    {1206, unknown_variable_description},
    {1207, unknown_variable_description},
    {1208, unknown_variable_description},
    {1209, unknown_variable_description},
    {1210, unknown_variable_description},
    {1211, unknown_variable_description},
    {1212, unknown_variable_description},
    {1213, unknown_variable_description},
    {1214, unknown_variable_description},
    {1215, unknown_variable_description},
    {1216, unknown_variable_description},
    {1217, unknown_variable_description},
    {1218, unknown_variable_description},
    {1219, unknown_variable_description},
    {1220, unknown_variable_description},
    {1221, unknown_variable_description},
    {1223, unknown_variable_description},
    {1224, unknown_variable_description},
    {1225, unknown_variable_description},
    {1226, unknown_variable_description},
    {1227, unknown_variable_description},
    {1228, unknown_variable_description},
    {1229, unknown_variable_description},
    {1230, unknown_variable_description},
    {1231, unknown_variable_description},
    {1232, unknown_variable_description},
    {1233, unknown_variable_description},
    {1234, unknown_variable_description},
    {1235, unknown_variable_description},
    {1236, unknown_variable_description},
    {1237, unknown_variable_description},
    {1238, unknown_variable_description},
    {1239, unknown_variable_description},
    {1240, unknown_variable_description},
    {1241, unknown_variable_description},
    {1243, unknown_variable_description},
    {1244, unknown_variable_description},
    {1245, unknown_variable_description},
    {1246, unknown_variable_description},
    {1247, unknown_variable_description},
    {1248, unknown_variable_description},
    {1249, unknown_variable_description},
    {1255, unknown_variable_description},
    {1256, unknown_variable_description},
    {1257, unknown_variable_description},
    {1258, unknown_variable_description},
    {1259, unknown_variable_description},
    {1260, unknown_variable_description},
    {1266, unknown_variable_description},
    {1267, unknown_variable_description},
    {1268, unknown_variable_description},
    {1269, unknown_variable_description},
    {1270, unknown_variable_description},
    {1271, unknown_variable_description},
    {1272, unknown_variable_description},
    {2010, "Active tariff"},
    {2011, unknown_variable_description},
    {   0, NULL } // End marker.
};

char const *var_name_of_id(int var_id) {
    char const *name = "Undefined";
    id2str *p;
    for (p = var_data; p->description; p++) {
        if (p->id == var_id) name = p->description;
    }
    return name;
}

unsigned int var_id_of_partial_name(char const *partial_name) {
    unsigned int var_id = 0;
    id2str *p;
    for (p = var_data; p->description; p++) {
        if (strstr(p->description, partial_name) != NULL) var_id = p->id;
    }
    return var_id;
}

void show_package_hex(unsigned char const *buffer, int length) {
    int index;
    for (index = 0; index < length; index++) {
        printf(" %02x", buffer[index]);
    }
}

void show_package_ascii(unsigned char const *buffer, int length) {
    int index;
    for (index = 0; index < length; index++) {
        show_char(buffer[index]);
    }
}

void show_package_named_char(unsigned char const *buffer, int length) {
    int index;
    for (index = 0; index < length; index++) {
        show_char(buffer[index]);
        if (buffer[index] == '\r') printf("\n");
    }
}

static unsigned char readvar_unknown_response[] = {
    '\x40',            // Start of response token.
    '\x3f',            // Address of receiver unit in meter.
    '\x10',            // Response to
    '\x07', '\x9a',    // CRC
    '\x0d'             // End of response token.
};
static int readvar_unknown_response_length = 6;

double decode_float_value(unsigned char length,
                          unsigned char *representation) {
    int exponent = representation[0] & 0x3f;
    if (representation[0] & 0x40) exponent = -exponent;

    unsigned char const *mantissa_bytes = representation + 1;
    unsigned long mantissa = 0;
    int index;
    for (index = 0; index < length; index++) {
        mantissa <<= 8;
        mantissa |= mantissa_bytes[index];
    }
    double factor = pow(10.0, (double)exponent);
    double value = ((double)mantissa) * factor;
    if (representation[0] & 0x80) value = -value;
    return value;
}

void show_unsupported_value(char const *kind, unsigned char const *buffer,
                            int const length, char const *unit) {
    printf("%s not yet supported, ", kind);
    show_package_hex(buffer, length);
    printf(" [%s]\n", unit);
}

void show_time_value(unsigned char const *buffer, int const length,
                     char const *unit) {
    int hhmmss = 16777216 * buffer[2] + 65536 * buffer[3] +
        256 * buffer[4] + buffer[5];
    int ss = hhmmss % 100;
    int hhmm = hhmmss / 100;
    int mm = hhmm % 100;
    int hh = hhmm / 100;
    printf("%02d:%02d:%02d [%s", hh, mm, ss, unit);
    if (buffer[0] != 4 || buffer[1] != 0) {
        printf(", unexpected data length: %d]\n", buffer[0] + 256 * buffer[1]);
    } else {
        printf("]\n");
    }
}

char* const month_name[] = {
    "(Undefined month: zero)",
    "Jan", "Feb", "Mar", "Apr",
    "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec"
};

void show_date3_value(unsigned char const *buffer, int const length,
                      char const *unit) {
    int yymmdd = 16777216 * buffer[2] + 65536 * buffer[3] +
        256 * buffer[4] + buffer[5];
    int dd = yymmdd % 100;
    int yymm = yymmdd / 100;
    int mm = yymm % 100;
    int yy = yymm / 100;
    printf("%02d, %s, %02d [%s", yy, month_name[mm], dd, unit);
    if (buffer[0] != 4 || buffer[1] != 0) {
        printf(", unexpected data length: %d]\n", buffer[0] + 256 * buffer[1]);
    } else {
        printf("]\n");
    }
}

void show_ascii_value(unsigned char const *buffer, int const length,
                      char const *unit) {
    // Apparently, the length is encoded twice for ASCII data:
    // One time in the general data format, and one more time in
    // the data area itself. We use the former to decide on the
    // amount of text shown, and print the latter, such that the
    // user can see both (they seem to follow each other, but if
    // they sometimes differ the user will at least get a hint).
    int embedded_length = buffer[0] + 256 * buffer[1];
    putchar('"');
    show_package_ascii(buffer + 2, length - 2);
    printf("\" [%s, length %d]\n", unit, embedded_length);
}

void show_varint_value(unsigned char const *buffer, int const length,
                       char const *unit) {
    // This is about unit 51 which is currently undocumented; guessing
    // that it contains an variable length unsigned integer number in
    // big-endian format which may be used for counting things, we show
    // it in a decimal format. We use the embedded length to decide on
    // the number of bytes to include, and give a hint in the case where
    // the specified total buffer length differs.
    int embedded_length = buffer[0] + 256 * buffer[1];
    if (embedded_length > length - 2) {
        // The value seems to occupy more bytes than the buffer contains.
        // We cannot interpret data beyond the data area that we have
        // received, so we drop out entirely here, and show the raw data.
        show_unsupported_value("Malformed INT", buffer, length, unit);
    } else {
        int index;
        unsigned long int value = 0;
        for (index = 0; index < embedded_length; index++) {
            value <<= 8;
            value |= buffer[index + 2];
        }
        if (embedded_length == length - 2) {
            // Data area used exactly, as expected.
            printf("%lu [%s, length %d]\n", value, unit, embedded_length);
        } else {
            // embedded_length < length - 2:
            printf("[%s, length %d]\n", unit, embedded_length);
        }
    }
}

void show_package(unsigned char *buffer, int length, int var_id) {
    if (!memcmp(buffer, readvar_unknown_response,
                readvar_unknown_response_length)) {
        printf("No value returned.\n");
        return;
    }
    int done = 0; // Set if and when all the data has been shown.
    unsigned short crc_expected = crc16(buffer + 1, length - 4);
    unsigned short crc_found = (buffer[length - 3] << 8) | buffer[length - 2];
    if (crc_expected != crc_found) {
        printf("Warning: Wrong CRC, found 0x%04X, expected 0x%04X\n",
               crc_found, crc_expected);
    }
    if (buffer[1] != '\x3f') {
        printf("Unexpected meter unit address: found 0x%02X, expected 0x3F\n",
               buffer[1]);
        show_package_named_char(buffer, length);
        return;
    }
    if (buffer[2] != '\x10') {
        printf("Unexpected type of response: found 0x%02X, expected 0x10\n",
               buffer[2]);
        show_package_named_char(buffer, length);
        return;
    }
    if ((buffer[3] << 8 | buffer[4]) != var_id) {
        printf("Unexpected variable id: found 0x%04X, expected 0x%04X\n",
               buffer[3] << 8 | buffer[4], var_id);
        show_package_named_char(buffer, length);
        return;
    }
    char const *unit = "UnknownUnit";
    UNIT_REPRESENTATION representation = UR_UNKNOWN;
    void const *data = buffer + 6;
    unsigned int const data_length = length - 9;
    if (buffer[5] < units_length) {
        representation = buffer[5];
        unit = units[representation];

        switch (unit_representation[representation]) {
            case UR_FLOAT: {
                double value = decode_float_value(buffer[6], buffer + 7);
                if (length == 15) {
                    printf("%0.4f %s\n", value, unit);
                    done = 1;
                } else if (length == 13) {
                    printf("%0.3f %s\n", value, unit);
                    done = 1;
                }
                break;
            }
            case UR_INT:
                show_unsupported_value("INT", data, data_length, unit);
                done = 1;
                break;
            case UR_BYTE:
                show_unsupported_value("BYTE", data, data_length, unit);
                done = 1;
                break;
            case UR_TIME:
                show_time_value(data, data_length, unit);
                done = 1;
                break;
            case UR_DATE2:
                show_unsupported_value("DATE2", data, data_length, unit);
                done = 1;
                break;
            case UR_DATE3:
                show_date3_value(data, data_length, unit);
                done = 1;
                break;
            case UR_DATE4:
                show_unsupported_value("DATE4", data, data_length, unit);
                done = 1;
                break;
            case UR_ASCII:
                show_ascii_value(data, data_length, unit);
                done = 1;
                break;
            case UR_BITS:
                show_unsupported_value("BITS", data, data_length, unit);
                done = 1;
                break;
            case UR_RTC:
                show_unsupported_value("RTC", data, data_length, unit);
                done = 1;
                break;
            case UR_RTCQ:
                show_unsupported_value("RTCQ", data, data_length, unit);
                done = 1;
                break;
            case UR_DATETIME:
                show_unsupported_value("DATETIME", data, data_length, unit);
                done = 1;
                break;
            case UR_VARINT:
                show_varint_value(data, data_length, unit);
                done = 1;
                break;
            case UR_UNKNOWN:
                printf("Raw data:");
                show_package_hex(data, data_length);
                if (buffer[5] >= units_length || *units[buffer[5]]) {
                    printf(" [no unit: %d]\n", buffer[5]);
                } else {
                    printf(" [%s]\n", unit);
                }
                done = 1;
                break;
            default:
                perror("Bug please report: unexpected unit representation");
                exit(-1);
        }
    } else {

        printf("Unexpected unit: found 0x%02X, expected 0x00..0x%02X\n",
               buffer[5], units_length);
    }
    if (!done) {
        show_package_hex(buffer, length);
        printf("\n");
    }
}

static unsigned char readvar_request[] = {
    '\x80',            // Start of request token.
    '\x3f',            // Address of receiver unit in meter.
    '\x10', '\x01',    // Read variable command.
    '\x00', '\x00',    // Variable identifier (to be modified).
    '\x00', '\x00',    // CRC (to be modified).
    '\x0d'             // End of request token.
};

static int const timeout = 3; // Seconds.
static int readvar_request_length = 9;
static unsigned int* const readvar_request_varid = (int*)(readvar_request + 4);
static unsigned int* const readvar_request_crc = (int*)(readvar_request + 6);

void usage(char *self) {
    printf("Usage: %s (var_id|partial_var_name) [device [baudrate]]\n", self);
    exit(0);
}

int main(int argc, char *argv[])
{
    char *device = DEVICE;
    int baudrate = BAUDRATE;
    int var_id = 1023; // Default variable identifier: Actual Power P14.

    if (argc < 2 || argc > 4) usage(argv[0]);
    if (argc > 1) {
        var_id = atoi(argv[1]);
        if (var_id == 0) {
            // Arg not a number, assumed to be a partial variable name.
            var_id = var_id_of_partial_name(argv[1]);
            if (var_id == 0) usage(argv[0]);
        }
    }
    if (argc > 2) {
        device = argv[1];
        printf("%s: Using device %s\n", argv[0], device);
    }
    if (argc > 3) {
        baudrate = baudrate_of(argv[0], argv[2]);
        printf("%s: Using baudrate %s\n", argv[0], argv[2]);
    }

    int optical_eye_fd = setup_optical_eye(device, baudrate, IS_8N2);
    readvar_request[4] = (char)(var_id >> 8);
    readvar_request[5] = (char)(var_id & 0xff);
    unsigned short crc = crc16(readvar_request + 1, 5);
    readvar_request[6] = (char)(crc >> 8);
    readvar_request[7] = (char)(crc & 0xff);
    optical_eye_write(optical_eye_fd, readvar_request, readvar_request_length);

    // Wait for data: we may need to skip an echo first, so we wait
    // for the response packet start byte 0x40.
    char buffer[BUFFER_LENGTH];
    int received_total = 0, received_cr = 0;
    char c = '\0';
    time_t starting_time = time(NULL);
    while (time(NULL) < starting_time + timeout) {
        int received = read(optical_eye_fd, &c, sizeof(c));
        if (c == '\x40') {
            buffer[received_total] = c;
            received_total++;
            break;
        }
    }

    // Now read the actual response.
    starting_time = time(NULL);
    while (time(NULL) < starting_time + timeout && received_cr < 1 &&
           received_total < BUFFER_LENGTH) {
        int received = read(optical_eye_fd, &c, sizeof(c));
        if (c == '\r') received_cr++;
        if (received == 1) {
            buffer[received_total] = c;
            received_total++;
        }
        ioctl(optical_eye_fd, I_FLUSH, FLUSHW);
    }
    received_total = descape_package(buffer, received_total);
    printf("%s (id %i): ", var_name_of_id(var_id), var_id);
    show_package(buffer, received_total, var_id);
    return 0;
}
