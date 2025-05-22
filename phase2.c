#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Initialize
struct Machine {
    char mem[300][4]; //memory
    char IR[4]; //instruction register ( holds the current instruction)
    int C; //toggel
    int IC; //holds next inst address
    char R[4]; //register 4 bytes
    int SI; //system calls
    int PI; //interrupt
    int TI; //interuppt
  } M;

struct PCB //process
{
    char JID[5];
    int TTL;
    int TLL;
    int TTC;
    int LLC;
};

struct PCB info;
int flag = 0;
int currentLine;


int PTR; //page table register
int generated_number[100];
int generated_no_index;
int Program_card_couter;
int count = 0;

int key[30];
int value[30];
int key_index;

int gen_address;
int message;
char buffer[41];

int T;


void MOS();
void EXECUTE();
void INIT();
void EXECUTE_USER_PROGRAM();
void GD();
void PD();
void H();
void LR();
void SR();
void BT();
void CR();
void simulation();
int Allocate();
int Address_Map(int IC);
int map(int add);
void print_status();
void print_memory_block();
void Load();
void ER(int msg);

int num(char IR[4]) {
    return (IR[2]-'0')*10+(IR[3]-'0'); //convert char to number
  }

void print_status(FILE* fp)
{

    fprintf(fp,"\nPI : %d\n", M.PI);
    fprintf(fp,"TI : %d\n", M.TI);
    fprintf(fp, "IC : %d\n", M.IC);
    fprintf(fp, "IR : %.4s\n", M.IR);
    fprintf(fp, "TTC : %d\n", info.TTC);
    fprintf(fp, "TTL : %d\n", info.TTL);
    fprintf(fp, "LLC : %d\n", info.LLC);
    fprintf(fp, "TLL : %d\n\n", info.TLL);
}


int Allocate()
{
    srand(time(NULL));
    int page_frame_no;
    int l = 0;
    while (1)
    {
        page_frame_no = rand() % 30;
        for (int i = 0; i <= generated_no_index; i++)
        {
            if (generated_number[i] == page_frame_no)
            {
                l = 1;
                break;
            }
        }
        if (l == 0)
        {
            break;
        }
        else
        {
            l = 0;
        }
    }

    generated_no_index++;
    generated_number[generated_no_index] = page_frame_no;
    return page_frame_no;
}

void print_memory_block() //used for debugging purpose
{
    printf("Memory block is:\n\n");
    int i, j;
    for (i = 0; i < 300; i++)
    {
        if (i >= 100) printf("M[%d]  ", i);
        else if (i >= 10 && i < 100) printf("M[%d]   ", i);
        else printf("M[%d]    ", i);

        for (j = 0; j < 4; j++)
        {
            printf("%c", M.mem[i][j]);
        }
        printf("\n");
    }
}

int Address_Map(int IC)
{
    if (IC % 10 == 0 && IC != 0)
    {
        count++;
    }
    int PTE = (M.mem[PTR + count][2] - '0') * 10 + (M.mem[PTR + count][3] - '0');
    int RA = PTE * 10 + IC % 10;
    return RA;
}

int map(int add)
{
    if (M.IR[0] == 'H' && M.IR[1] == ' ' && M.IR[2] == ' ' && M.IR[3] == ' ')
        return;

    if (M.IR[0] == 'B' && M.IR[1] == 'T')
        return;

    if (M.IR[2] - '0' < 0 || M.IR[2] - '0' > 9 || M.IR[3] - '0' < 0 || M.IR[3] - '0' > 9)
    {
        M.PI = 2;
        MOS();
        return;
    }

    for (int i = 0; i <= key_index; i++)
    {
        if (key[i] == (add / 10) * 10)
        {
            return (value[i]) * 10 + add % 10;
        }
    }

    M.PI = 3;
    MOS();
    return;
}

void ER(int msg)
{
    T = 1;
    FILE *fp = NULL;
    fp = fopen("output2.txt", "a");

    fprintf(fp, "\nJob ID:%s\n", info.JID);

    switch (msg)
    {
    case 0: fputs("  No Error\n", fp); break;
    case 1: fputs("  OUT OF DATA\n", fp); break;
    case 2: fputs("  LINE LIMIT EXCEEDED\n", fp); break;
    case 3: fputs("  TIME LIMIT EXCEEDED\n", fp); break;
    case 4: fputs("  OPERATION CODE ERROR\n", fp); break;
    case 5: fputs("  OPERAND ERROR\n", fp); break;
    case 6: fputs("  INVALID PAGE FAULT\n", fp); break;
    }

    print_status(fp);


}

void Load()
{
    FILE *fp = NULL;
    fp = fopen("input2.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening input file!\n");
        exit(1);
    }

    while (fgets(buffer, 41, fp) != NULL)
    {
        if (buffer[0] == '$' && buffer[1] == 'A' && buffer[2] == 'M' && buffer[3] == 'J')
        {
            PTR = Allocate() * 10;
            strncpy(info.JID, buffer+4, 4);
            info.JID[4] = '\0';

            info.TTL = (buffer[8] - '0') * 1000 + (buffer[9] - '0') * 100 + (buffer[10] - '0') * 10 + (buffer[11] - '0');
            info.TLL = (buffer[12] - '0') * 1000 + (buffer[13] - '0') * 100 + (buffer[14] - '0') * 10 + (buffer[15] - '0');
            info.TTC = 0;
            info.LLC = 0;

            for (int i = PTR; i < PTR + 10; i++)
            {
                M.mem[i][0] = '0';
                M.mem[i][2] = '*';
                M.mem[i][3] = '*';
            }

            int col = 0;
            int temp, temp1;

            while (1)
            {
                fgets(buffer, 41, fp);
                if ((buffer[0] == '$' && buffer[1] == 'D' && buffer[2] == 'T' && buffer[3] == 'A'))
                    break;

                temp = Allocate();
                temp1 = temp * 10;

                M.mem[PTR + Program_card_couter][0] = '1';
                M.mem[PTR + Program_card_couter][3] = temp % 10 + '0';
                M.mem[PTR + Program_card_couter][2] = temp / 10 + '0';
                Program_card_couter++;

                col = 0;
                for (int i = 0; i < strlen(buffer) - 1; i++)
                {
                    if (buffer[i] == ' ') break;
                    M.mem[temp1][col] = buffer[i];
                    col++;
                    if (col == 4)
                    {
                        temp1++;
                        col = 0;
                    }
                }
            }
            currentLine = ftell(fp);
        }

        if ((buffer[0] == '$' && buffer[1] == 'D' && buffer[2] == 'T' && buffer[3] == 'A'))
        {
            EXECUTE();
        }

        if ((buffer[0] == '$' && buffer[1] == 'E' && buffer[2] == 'N' && buffer[3] == 'D'))
        {
            // print_memory_block();

            if (message == 1)
            {
                fseek(fp, ftell(fp) + 2, SEEK_SET);
            }

            INIT();
        }
    }
    fclose(fp);
}

void INIT()
{
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            M.mem[i][j] = ' ';
        }
    }
    for (int i = 0; i < 4; i++)
    {
        M.IR[i] = ' ';
        M.R[i] = ' ';
    }
    M.IC = 0;
    M.SI = 0;
    M.C = 0;
    T = 0;
    currentLine = 0;
    M.PI = 0;
    M.TI = 0;
    message = 0;

    generated_no_index = -1;
    Program_card_couter = 0;
    key_index = -1;
    count = 0;
}

void EXECUTE()
{
    M.IC = 0;
    EXECUTE_USER_PROGRAM();
}

void EXECUTE_USER_PROGRAM()
{
    while (1)
    {
        int RA = Address_Map(M.IC);
        for (int i = 0; i < 4; i++)
        {
            M.IR[i] = M.mem[RA][i];
        }
        M.IC++;

        simulation();
        gen_address = map(num(M.IR));

        if (T == 1) break;

        if (!((M.IR[0] == 'G' && M.IR[1] == 'D') || (M.IR[0] == 'P' && M.IR[1] == 'D') ||
             (M.IR[0] == 'H' && M.IR[1] == ' ') || (M.IR[0] == 'L' && M.IR[1] == 'R') ||
             (M.IR[0] == 'S' && M.IR[1] == 'R') || (M.IR[0] == 'C' && M.IR[1] == 'R') ||
             (M.IR[0] == 'B' && M.IR[1] == 'T')))
        {
            M.PI = 1;
            MOS();
            break;
        }

        if (M.TI == 2)
        {
            MOS();
            break;
        }
        else if (M.IR[0] == 'G' && M.IR[1] == 'D')
        {
            M.SI = 1;
            MOS();
            if (T == 1) break;
        }
        else if (M.IR[0] == 'P' && M.IR[1] == 'D')
        {
            M.SI = 2;
            MOS();
            if (T == 1) break;
        }
        else if (M.IR[0] == 'H' && M.IR[1] == ' ')
        {
            M.SI = 3;
            MOS();
            break;
        }
        else if (M.IR[0] == 'L' && M.IR[1] == 'R')
        {
            LR();
        }
        else if (M.IR[0] == 'S' && M.IR[1] == 'R')
        {
            SR();
        }
        else if (M.IR[0] == 'B' && M.IR[1] == 'T')
        {
            BT();
        }
        else if (M.IR[0] == 'C' && M.IR[1] == 'R')
        {
            CR();
        }
    }
}

void simulation()
{
    if ((M.IR[0] == 'G' && M.IR[1] == 'D') || (M.IR[0] == 'S' && M.IR[1] == 'R'))
    {
        info.TTC += 2;
    }
    else
    {
        info.TTC += 1;
    }
    if (info.TTC > info.TTL)
    {
        M.TI = 2;
        ER(3);
    }
}

void MOS()
{
    if (M.TI == 0)
    {
        if (M.SI == 1)
        {
            GD();
        }
        else if (M.SI == 2)
        {
            PD();
        }
        else if (M.SI == 3)
        {
            H();
        }
        else if (M.PI == 1)
        {
            ER(4);
        }
        else if (M.PI == 2)
        {
            ER(5);
        }
        else if (M.PI == 3)
        {
            if ((M.IR[0] == 'G' && M.IR[1] == 'D') || (M.IR[0] == 'S' && M.IR[1] == 'R'))
            {
                key_index++;
                key[key_index] = (M.IR[2] - '0') * 10 + (M.IR[3] - '0');
                value[key_index] = Allocate();
                int temp = value[key_index];

                M.mem[PTR + Program_card_couter][0] = '1';
                M.mem[PTR + Program_card_couter][3] = temp % 10 + '0';
                M.mem[PTR + Program_card_couter][2] = temp / 10 + '0';
                Program_card_couter++;
                M.PI = 0;
                return value[key_index] * 10;
            }
            else
            {
                ER(6);
            }
        }
    }
    else if (M.TI == 2)
    {
        if (M.PI == 0)
        {
            if (M.IR[0] == 'P' && M.IR[1] == 'D')
            {
                PD();
            }
            else if (M.IR[0] == 'H' && M.IR[1] == ' ')
            {
                H();
                return;
            }
            ER(3);
        }
        else if (M.PI == 1)
        {
            ER(7);
        }
        else if (M.PI == 2)
        {
            ER(8);
        }
        else if (M.PI == 3)
        {
            ER(3);
        }
    }
}

void GD()
{
    int row = map(num(M.IR));
    int col = 0;

    FILE *fp = NULL;
    fp = fopen("input2.txt", "r");
    fseek(fp, currentLine, SEEK_SET);
    fgets(buffer, 41, fp);

    if ((buffer[0] == '$' && buffer[1] == 'E' && buffer[2] == 'N' && buffer[3] == 'D'))
    {
        message = 1;
        ER(1);
        fclose(fp);
        return;
    }

    for (int i = 0; i < strlen(buffer) - 1; i++)
    {
        M.mem[row][col] = buffer[i];
        col++;
        if (col == 4)
        {
            row++;
            col = 0;
        }
    }
    currentLine = ftell(fp);
    fclose(fp);
    M.SI = 0;
}

void PD()
{
    FILE *fp = NULL;
    info.LLC++;
    if (info.LLC > info.TLL)
    {
        ER(2);
        return;
    }

    fp = fopen("output2.txt", "a");
    if (flag == 0)
        flag++;
    else
        fprintf(fp, "\n");

    int row = map(num(M.IR));
    int col = 0;

    for (int i = row; i < row + 10; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            fputc(M.mem[i][j], fp);
        }
    }
    fclose(fp);
    M.SI = 0;
}

void H()
{
    ER(0);
}

void LR()
{
    int row = map(num(M.IR));
    for (int i = 0; i < 4; i++)
    {
        M.R[i] = M.mem[row][i];
    }
}

void SR()
{
    int row = map(num(M.IR));
    for (int i = 0; i < 4; i++)
    {
        M.mem[row][i] = M.R[i];
    }
}

void BT()
{
    if (M.C == 1)
    {
        M.IC = num(M.IR);
        M.C = 0;
    }
}

void CR()
{
    int row = map(num(M.IR));
    for (int i = 0; i < 4; i++)
    {
        if (M.R[i] != M.mem[row][i])
        {
            M.C = 0;
            break;
        }
        else
        {
            M.C = 1;
        }
    }
}

int main()
{

    FILE *f = NULL;
    char ch;
    f = fopen("input2.txt", "r");
    if (f == NULL)
    {
        printf("not found!!");
        exit(1);
    }

    fclose(f);

    INIT();
    Load();

    return 0;
}
