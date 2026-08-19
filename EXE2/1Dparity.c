#include <stdio.h>
#include <string.h>

void charTo7Bit(char ch, char bin[])
{
    int i;
    for(i = 6; i >= 0; i--)
    {
        if(ch % 2 == 0)
            bin[i] = '0';
        else
            bin[i] = '1';

        ch = ch / 2;
    }
    bin[7] = '\0';
}

char getEvenParity(char bin[])
{
    int i;
    int ones = 0;

    for(i = 0; i < 7; i++)
    {
        if(bin[i] == '1')
            ones++;
    }

    if(ones % 2 == 0)
        return '0';
    else
        return '1';
}

int countOnes(char data[])
{
    int i;
    int count = 0;

    for(i = 0; i < 8; i++)
    {
        if(data[i] == '1')
            count++;
    }

    return count;
}

int main()
{
    char message[100];
    char blocks[100][9];
    int i, j;

    printf("Enter a message: ");
    fgets(message, sizeof(message), stdin);

    int len = strlen(message);
    if(len > 0 && message[len - 1] == '\n')
    {
        message[len - 1] = '\0';
        len--;
    }

    printf("\n--- Binary Table ---\n\n");
    printf("Block\tChar\t7-Bit Data\tParity Added\tFull Block\n");
    printf("-----\t----\t----------\t------------\t----------\n");

    for(i = 0; i < len; i++)
    {
        char sevenBit[8];

        charTo7Bit(message[i], sevenBit);

        for(j = 0; j < 7; j++)
        {
            blocks[i][j] = sevenBit[j];
        }

        char parityBit = getEvenParity(sevenBit);
        blocks[i][7] = parityBit;
        blocks[i][8] = '\0';

        printf("  %d\t %c\t%s\t\t%c\t\t%s\n", i + 1, message[i], sevenBit, parityBit, blocks[i]);
    }

    char choice;
    printf("\nDo you want to change any bits? (Y/N): ");
    scanf(" %c", &choice);

    if(choice == 'Y' || choice == 'y')
    {
        int bitsToModify;
        printf("How many bits do you want to change in each block?: ");
        scanf("%d", &bitsToModify);

        for(i = 0; i < len; i++)
        {
            printf("\n--- Block %d ('%c') ---\n", i + 1, message[i]);

            for(j = 0; j < bitsToModify; j++)
            {
                int bitPos;
                char newValue;

                printf("Enter position (1 to 8): ");
                scanf("%d", &bitPos);

                printf("Enter new value (0 or 1): ");
                scanf(" %c", &newValue);

                blocks[i][bitPos - 1] = newValue;
            }
        }

        printf("\n--- Updated Blocks ---\n");
        for(i = 0; i < len; i++)
        {
            printf("Block %d: %s\n", i + 1, blocks[i]);
        }

        printf("\n--- Checking for Errors ---\n");
        int hasError = 0;

        for(i = 0; i < len; i++)
        {
            int totalOnes = countOnes(blocks[i]);

            if(totalOnes % 2 != 0)
            {
                printf("Block %d: Error found!\n", i + 1);
                hasError = 1;
            }
            else
            {
                printf("Block %d: No error found.\n", i + 1);
            }
        }

        if(hasError == 0)
        {
            printf("\nAll blocks are fine.\n");
        }
    }
    else
    {
        printf("\nNo changes made.\n");
    }

    return 0;
}
