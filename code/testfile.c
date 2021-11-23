#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char *file1 = argv[1];
    char *file2 = argv[2];

    FILE *fp1 = fopen(file1, "rb");
    FILE *fp2 = fopen(file2, "rb");

    if (fp1 == NULL || fp2 == NULL)
    {
        printf("Error opening file\n");
        return 1;
    }

    while(!feof(fp1) && !feof(fp2))
    {
        char c1 = fgetc(fp1);
        char c2 = fgetc(fp2);

        if (c1 != c2)
        {
            printf("Files are not equal\n");
            return 1;
        }
    }
    printf("Files are equal\n");
}