/*
C language.
 Written by : Ahmad Dadoush.
 */
#include <cs50.h>
#include <stdio.h>
#include <math.h>
#include "helpers.h"

/**
 * Returns true if value is in array of n values, else false.
 */
bool search(int value, int values[], int n)
{
    // Binary search.
    int start = 0;
    int end = n - 1;
    int middle = (start + end) / 2;
    while(start <= end)
    {
        if(values[middle] == value)
        {
            return true;
        }
        else if (values[middle] > value)
        {
            end = middle -1;
            middle = (start + end) / 2;
        }
        else if (values[middle] < value)
        {
            start = middle +1;
            middle = (start + end) / 2;
        }
        
    }
    return false;
}
