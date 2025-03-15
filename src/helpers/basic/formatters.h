#ifndef FORMATTERS_H
#define FORMATTERS_H

typedef enum {
    B,
    KB,
    MB,
    GB
} SizeMeasurement;

static inline float bytes_to_measurement(int byteCount, SizeMeasurement measurement) {
    switch (measurement) {
        case B:
            return (float)byteCount;
        case KB:
            return (float)byteCount / 1024;
        case MB:
            return (float)byteCount / (1024 * 1024);
        case GB:
            return (float)byteCount / (1024 * 1024 * 1024);
        default:
            return (float)byteCount;  // fallback to bytes if unrecognized
    }
}

#endif