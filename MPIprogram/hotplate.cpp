#include "hotplate.h"
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

const int plateHeight = 1024;
const int plateWidth = 1024;

float** plate1;
float** plate2;
bool** lockedCells;
float** lockedValues;

/* Return the current time in seconds, using a double precision number. */
double now() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}

void initializeFloatArray(float** array, int length, int size) {
    for (int i = 0; i < length; i++) {
        array[i] = new float[size];
        memset(array[i], 0.0, size * sizeof(float));
    }
}

void initializeBoolArray(bool** array, int length, int size) {
    for (int i = 0; i < length; i++) {
        array[i] = new bool[size];
        for (int j = 0; j < size; j++) {
            array[i][j] = false;
        }
        //memset(array[i], 0.0, size * sizeof(bool));
    }
}

void init() {
    // The double buffers for the hot plate
    plate1 = new float*[plateHeight];
    initializeFloatArray(plate1, plateHeight, plateWidth);
    plate2 = new float*[plateHeight];
    initializeFloatArray(plate2, plateHeight, plateWidth);
    
    lockedCells = new bool*[plateHeight];
    initializeBoolArray(lockedCells, plateHeight, plateWidth);
    lockedValues = new float*[plateHeight];
    initializeFloatArray(lockedValues, plateHeight, plateWidth);

    for (int x = 0; x < plateWidth; x++) {
        // Lock the bottom row at 0 degrees
        lockCell(x, 0, 100);
        // Lock the top row at 100 degrees
        lockCell(x, plateHeight-1, 0);
    }
    for (int y = 0; y < plateHeight; y++) {
        // Lock the left row at 0 degrees
        lockCell(0, y, 0);
        // Lock the right row at 0 degrees
        lockCell(plateWidth-1, y, 0);
    }
    for (int x = 0; x <= 330; x++) {
        // Lock this row at 100 degrees
        lockCell(x, 400, 100);
    }
    // Lock the random cell
    lockCell(500, 200, 100);
}

bool validCell(int x, int y) {
    return x >= 0 && x < plateWidth && y >= 0 && y < plateHeight;
}

void lockCell(int x, int y, float temp) {
    if (validCell(x, y)) {
        lockedCells[y][x] = true;
        lockedValues[y][x] = temp;
    }
}

void initPlate(float** plate) {
    for (int y = 0; y < plateHeight; y++) {
        for (int x = 0; x < plateWidth; x++) {
            if (lockedCells[y][x]) {
                // Temperature the cell was locked to
                plate[y][x] = lockedValues[y][x];
            } else {
                // Default cell starting temperature
                plate[y][x] = 50.0;
            }
        }
    }
}

//void swapPlate(float* oldPlate, float* newPlate) {
//    memcpy((void*)newPlate, (void*)oldPlate, plateWidth * plateHeight * sizeof(float));
//}


bool isStable(float** plate) {
    // Only scan the inside section (not the outside columns and rows)
    // This way, each iteration doesn't need a range check.
    for (int y = 0; y < plateHeight; y++) {
        //printf("Entering at (x, %i)\n", y);
        for (int x = 0; x < plateWidth; x++) {
            // If the cell is locked, then it is skipped while determining stability
            if (!lockedCells[y][x]) {
                float diff = fabs(plate[y][x] - (plate[y][x-1] + plate[y-1][x] + plate[y][x+1] + plate[y+1][x]) * 0.25);
                if (!(diff < 0.1)) {
                    /*
                    printf("Exiting at (%i, %i)\n", x, y);
                    printf("%2.0f %2.0f %2.0f\n%2.0f %2.0f %2.0f\n%2.0f %2.0f %2.0f\n",
                        plate[cellRef(x-1, y-1)], plate[cellRef(x, y-1)], plate[cellRef(x+1, y-1)], 
                        plate[cellRef(x-1, y)], plate[cellRef(x, y)], plate[cellRef(x+1, y)], 
                        plate[cellRef(x-1, y+1)], plate[cellRef(x, y+1)], plate[cellRef(x+1, y+1)]);
                    
                    printf("Ending diff: %2.3f\n", diff);
                    */
                    return false;
                }
            }
        }
    }
    return true;
}

// Changed: This no longer serves dual purposes.
// This function serves the purpose of both updating the new plate,
// and also determining whether or not the update was even needed.
// If the current status was stable as it was, then true is returned,
// and the process should not be repeated. Otherwise, false is returned.
bool update(float** oldPlate, float** newPlate) {
    for (int y = 0; y < plateHeight; y++) {
        for (int x = 0; x < plateWidth; x++) {
            if (!lockedCells[y][x]) {
                // The borders where this would grab invalid data will always be locked
                // so we don't need to check here for that.
                newPlate[y][x] = (oldPlate[y][x-1] + oldPlate[y-1][x] + oldPlate[y][x+1] + oldPlate[y+1][x] + (4.0 * oldPlate[y][x])) * 0.125;
            } else {
                newPlate[y][x] = oldPlate[y][x];
            }
        }
    }
    return false;
}

void end() {
    for (int i = 0; i < plateHeight; i++) {
        delete[] plate1[i];
    }
    delete[] plate1;
    plate1 = NULL;
    
    for (int i = 0; i < plateHeight; i++) {
        delete[] plate2[i];
    }
    delete[] plate2;
    plate2 = NULL;
    
    for (int i = 0; i < plateHeight; i++) {
        delete[] lockedCells[i];
    }
    delete[] lockedCells;
    lockedCells = NULL;
    
    for (int i = 0; i < plateHeight; i++) {
        delete[] lockedValues[i];
    }
    delete[] lockedValues;
    lockedValues = NULL;
}

int main(int argc, char* argv[]) {
    // Start the timing
    double start = now();
    printf("Start: %.4f\n", start);

    // Initialize the locked cells
    init();
    // Initialize the plate using the locked cells
    initPlate(plate1);
    initPlate(plate2);

    float** plate = plate1;
    float** otherPlate = plate2;
    float** tmp;
    
    bool done = false;
    int count = 0;
    while (true) {
        //printf("\n\n");
        
        // Run a single step
        // Swap the plates
        tmp = otherPlate;
        otherPlate = plate;
        plate = tmp;
        // Now update back into the regular plate
        update(otherPlate, plate);
        count++;
        
        done = isStable(plate);
        if (done) {
            break;
        } else {
            //printf("Not Stable\n");
        }

        //for (int y = 0; y < plateHeight; y++) {
        //    for (int x = 0; x < plateWidth; x++) {
        //        printf("%3.0f ", plate[cellRef(x, y)]);
        //    }
        //    printf("\n");
        //}

        //sleep(1);
    }
    printf("Iterations required: %i\n", count);
    int highTempCount = 0;
    int strictlyHigherTempCount = 0;
    for (int y = 0; y < plateWidth; y++) {
        for (int x = 0; x < plateHeight; x++) {
            //printf("%6.2f\t", plate[y][x]);
            
            if (plate[y][x] >= 50) {
                highTempCount++;
                if (plate[y][x] > 50) {
                    strictlyHigherTempCount++;
                }
            }
        }
        //printf("\n");
    }
    printf("Cells >= 50 degrees: %i\n", highTempCount);
    printf("Cells > 50 degrees: %i\n", strictlyHigherTempCount);

    // Clear up the allocated memory
    // Including the locked cells arrays
    end();

    // Finish up with the timing
    double end = now();
    printf("End: %.4f\n", end);
    printf("Elapsed: %.4f\n", end - start);
    
    return 0;
}

