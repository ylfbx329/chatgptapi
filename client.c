#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define FINISH_FLAG "finish"

typedef struct
{
    char foodName[30];
    int num;
} FOOD;

typedef struct foodNode
{
    FOOD data;
    struct foodNode *next;
} LfoodNode;

void menu(void);
void exitmenu(void);
SOCKET startClient(void);
void Lfree(LfoodNode *H);
int inputFoodname(FOOD *ptmp);
void addOrder(SOCKET clientSocket);
void chooseFood(SOCKET clientSocket);

int main()
{
    int mode;
    SOCKET clientSocket;
    menu();
    while (1)
    {
        printf("请输入指令\n");
        scanf("%d", &mode);
        switch (mode)
        {
        case 1:
            clientSocket = startClient();
            chooseFood(clientSocket);
            addOrder(clientSocket);
            closesocket(clientSocket);
            break;
        case 2:
            WSACleanup();
            exitmenu();
            exit(EXIT_SUCCESS);
            break;
        default:
            menu();
            printf("指令错误，请重新输入\n");
            break;
        }
    }
    return 0;
}

void menu(void) // OK
{
    system("cls");
    printf("*************************************************\n"
           "*\t欢迎使用森林之家订餐系统（客户端）\t*\n"
           "*************************************************\n"
           "*\t1.选择菜品\t\t\t\t*\n"
           "*\t2.退出系统\t\t\t\t*\n"
           "*************************************************\n");
}

SOCKET startClient(void) // OK
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    //创建套接字
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //绑定服务端地址
    struct sockaddr_in socketaddr;
    ZeroMemory(&socketaddr, sizeof(socketaddr));
    socketaddr.sin_family = AF_INET;
    socketaddr.sin_addr.S_un.S_addr = inet_addr("10.191.81.110");
    socketaddr.sin_port = htons(1234);
    connect(clientSocket, (struct sockaddr *)&socketaddr, sizeof(socketaddr));
    return clientSocket;
}

void chooseFood(SOCKET clientSocket) // OK
{
    int foodNum = 0;
    char name[30];
    char num[4];
    char flag[] = FINISH_FLAG;
    FOOD tmp;
    LfoodNode *foodListHead, *p;
    foodListHead = (LfoodNode *)malloc(sizeof(LfoodNode));
    foodListHead->next = NULL;
    p = foodListHead;
    while (1)
    {
        memset(name, 0, sizeof(name));
        recv(clientSocket, name, sizeof(name), 0); //菜名
        if (strcmp(name, flag) == 0)
        {
            break;
        }
        memset(num, 0, sizeof(num));
        recv(clientSocket, num, sizeof(num), 0); //数量
        strcpy(tmp.foodName, name);
        tmp.num = atoi(num);
        p->next = (LfoodNode *)malloc(sizeof(LfoodNode));
        p = p->next;
        p->data = tmp;
        p->next = NULL;
        foodNum++;
    }
    system("cls");
    printf("*********************************\n");
    printf("*\t小店今日有 %d 件菜品\t*\n", foodNum);
    printf("*   菜品名称\t*    剩余份数\t*\n");
    printf("*********************************\n");
    p = foodListHead->next;
    while (p != NULL)
    {
        if (p->data.num)
            printf("*   %s\t*\t%d\t*\n", p->data.foodName, p->data.num);
        else
            printf("*   %s\t*\t已售完\t*\n", p->data.foodName);
        p = p->next;
    }
    printf("*********************************\n");
    free(foodListHead);
}

void addOrder(SOCKET clientSocket) // OK
{
    int foodNum = 0;
    char num[4];
    char address[100];
    FOOD tmp;
    LfoodNode *foodOrderHead, *p;
    foodOrderHead = (LfoodNode *)malloc(sizeof(LfoodNode));
    foodOrderHead->next = NULL;
    p = foodOrderHead;
    printf("请输入下单菜品和数量（ESC键结束下单）\n");
    while (inputFoodname(&tmp))
    {
        scanf("%d", &tmp.num);
        p->next = (LfoodNode *)malloc(sizeof(LfoodNode));
        p = p->next;
        p->data = tmp;
        p->next = NULL;
        foodNum++;
    }
    system("cls");
    printf("请输入地址信息\n");
    scanf("%s", address);
    printf("*********************************\n");
    printf("*\t已选则 %d 件菜品\t\t*\n", foodNum);
    printf("*   菜品名称\t*    剩余份数\t*\n");
    printf("*********************************\n");
    p = foodOrderHead->next;
    while (p != NULL)
    {
        printf("*   %s\t*\t%d\t*\n", p->data.foodName, p->data.num);
        p = p->next;
    }
    printf("*********************************\n");
    printf("按 1 确认订单，按 0 取消订单\n");
    int orderMode;
    scanf("%d", &orderMode);
    if (orderMode == 1)
    {
        char ok[] = "ok";
        char flag[] = FINISH_FLAG;
        p = foodOrderHead->next;
        send(clientSocket, ok, strlen(ok), 0);
        Sleep(10);
        while (p != NULL)
        {
            tmp = p->data;
            send(clientSocket, tmp.foodName, strlen(tmp.foodName), 0);
            Sleep(10);
            memset(num, 0, sizeof(num));
            itoa(tmp.num, num, 10);
            send(clientSocket, num, strlen(num), 0);
            Sleep(10);
            p = p->next;
        }
        send(clientSocket, flag, strlen(flag), 0);
        Sleep(10);
        send(clientSocket, address, strlen(address), 0);
        Lfree(foodOrderHead);
        menu();
        printf("订单已发送\n");
        return;
    }
    else if (orderMode == 0)
    {
        char no[] = "no";
        send(clientSocket, no, strlen(no), 0);
        Sleep(10);
        free(foodOrderHead);
        menu();
        printf("订单已取消\n");
    }
    else
    {
        printf("输入错误，重新输入\n");
        scanf("%d", &orderMode);
    }
}

int inputFoodname(FOOD *ptmp) // OK
{
    int bufferIndex = 0;
    char inputChar;
    while (1)
    {
        inputChar = _getch();
        if (inputChar == 27)
        {
            return 0;
        }
        else if (inputChar == ' ')
        {
            ptmp->foodName[bufferIndex] = '\0';
            putchar(inputChar);
            return 1;
        }
        else if (inputChar == '\b')
        {
            if (bufferIndex > 0)
            {
                char prechar = ptmp->foodName[bufferIndex - 1];
                if ((prechar >= 'A' && prechar <= 'Z') || (prechar >= 'a' && prechar <= 'z') || (prechar >= '0' && prechar <= '9'))
                {
                    printf("\b \b");
                    bufferIndex--;
                }
                else //汉字处理两字符
                {
                    printf("\b\b  \b\b");
                    bufferIndex -= 2;
                }
            }
        }
        else
        {
            ptmp->foodName[bufferIndex++] = inputChar;
            putchar(inputChar);
        }
    }
}

void Lfree(LfoodNode *H) // OK
{
    LfoodNode *p;
    while (H != NULL)
    {
        p = H->next;
        free(H);
        H = p;
    }
    return;
}

void exitmenu() // OK
{
    system("cls");
    printf("*****************************************\n"
           "*\t感谢使用，祝您生活愉快\t\t*\n"
           "*****************************************\n");
    Sleep(3000);
}
