/**
 * Sorts array of n values.
 C language.
 Written by : Ahmad Dadoush.
 */
void sort(int values[], int n)
{
    // selection sort.
    int key;
    int min;
    int j;
    for (int i = 0; i < n - 1; i++)
    {
        key = values[i];
        min = i;
        for (j = i+1 ; j < n ; j++)
        {
            if (values[j] < values[min])
            {
            min = j;
            }
        }
        values[i] = values[min];
        values[min] = key;
    }
}
