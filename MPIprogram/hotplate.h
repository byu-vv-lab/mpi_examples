#pragma once
#ifndef HOTPLATE_H
#define HOTPLATE_H

double now();

void initializeFloatArray(float** array, int length, int size);

void initializeBoolArray(float** array, int length, int size);

void init();

bool validCell(int x, int y);

void lockCell(int x, int y, float temp);

void initPlate(float** plate);

//void swapPlate(float* oldPlate, float* newPlate);

bool isStable(float** plate);

bool update(float** oldPlate, float** newPlate);

void end();

int main(int argc, char* argv[]);

#endif

