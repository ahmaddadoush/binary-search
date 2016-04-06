/*
C language.
 Written by : Ahmad Dadoush.
 */

bool search(int value, int values[], int n)
{
    // Binary search.
    int start = 0;
    int end = n - 1;
    int list = end;
    int middle = (start + end) / 2;
    while(list > 0)
    {
        if(values[middle] == value)
        {
            return true;
        }
        else if (values[middle] > value)
        {
            list = end - middle;
            end = middle -1;
            middle = (start + end) / 2;
        }
        else if (values[middle] < value)
        {
            list = end - middle;
            start = middle +1;
            middle = (start + end) / 2;
        }
    }
    return false;
}
