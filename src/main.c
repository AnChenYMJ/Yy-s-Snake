#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <malloc.h>
#include <time.h>
#include <stdbool.h>	//bool Ҫʶ������
#include <share.h>		//_fsopen

//ȫ�ֱ���
#define X 20		//��Ϸ����ĳ���(����)
#define Y 20
char g_Back[X][Y];	//��Ϸ����ı�������
enum Dir { enum_east, enum_south, enum_west, enum_north};	//ö�٣����ڲ����ߵ��ƶ����򣨶���������Ĭ�ϴ�0 1 2 3��ʼ
struct Node* g_pHead = NULL;								//���������ͷ�ڵ�
struct Node* g_pEnd = NULL;									//β�ڵ�
int g_quit = 1;												//����Esc���˳���Ϸ
BOOL g_stop = false;										//������ͣ/����
int g_speed = 500;											//С���ƶ����ٶ�
BOOL g_save = true;											//һ����Ϸ�ɱ�����ı�־

//�������
void Head(void);	//��ӡ������ʾ��Ϣ
void Frame(void);	//��ӡ��Ϸ������Ϸ�ı߿򡢱߽磩
void GotoXY(unsigned short hang, unsigned short lie);	//ָ�����λ��
void DrawSnake(void);									//������Ϸ��ʼʱ�ĳ�ʼС��
void Drop(void);										//����С���ƶ�ʱ��β�͵Ĳ���
void Food(void);										//����ʳ��


		//��ǰ�ڵ���������ƶ�����
struct Snake
{
	int iX;				//����
	int iY;
	enum Dir emdir;		//�ߵ��ƶ����򣨶���������
};
		//�ߵĽڵ�
struct Node
{
							//д����ʱ�Ĺ淶�����£�ָ�������ǰ��p int����ǰ��i st���ǽṹ��
	struct Snake stSnake;	//��ǰ�ڵ���������ƶ�����
	int iCount;				//��������¼�ж��ٽڵ�
	struct Node* pPre;		//��һ���ڵ��ַ
	struct Node* pNext;		//��һ���ڵ��ַ
};

//�������
void CreateSnake(int iCount);	//��Ϸ�ʼ������,���������ɼ����ڵ�
void Free(void);				//�ͷ�����Ŀռ�
void Move(void);				//��С�߿�ʼ�ƶ�
void KeyState(void);			//��ⰴ��
void Appand(void);				//С�߳���ʳ�ﳤ������½ڵ㣩
bool Eat(void);					//��ʳ��
bool IsDie(void);				//�ж�С������

//������Ϸ����
void SetSocre(void);			//��ӡС�߳���
void NewGame(void);				//���¿�ʼ��Ϸ����������е���
void ChangeSpeed(void);			//�л���Ϸ�Ѷ�

//��������
struct Record					//�洢���ݵĽṹ��
{
	time_t t;					//��¼ʱ��
	int socre;					//��¼����
};			
void Save(void);				//���浽�ļ�
void Read(void);				//��ȡ�ļ�����

int main(void)
{
	g_save = true;							//��ʼ�����˾��ܱ�����
	srand((unsigned int)time(NULL));		//��������ӣ��õ�ǰϵͳʱ�䴥������Ҫtimeͷ�ļ�,ǿת��Ϊ��ȥ����
	//��ӡ��ʾ��Ϣ
	Head();
	//��ӡ��Ϸ����ı߿򼰱���
	Frame();
	//��ʼ�����ߵĽڵ�,��������Ҫ���ɼ����ڵ�
	CreateSnake(3);
	//��ӡ��ʼ��С��
	DrawSnake();
	//����ʳ��
	Food();
	//��ӡС�߳���
	SetSocre();

	while (1)
	{
		//��ⰴ��	��Ҫ�������Ϸ���飬�ͱ�����Move֮ǰ��ⰴ��������Ҫ�ƶ�һ����ת��
		KeyState();
		//�ж��Ƿ��˳���Ϸ
		if (0 == g_quit)
		{
			GotoXY(20, 45);
			printf("���˳���Ϸ,�밴���������");
			break;
		}
			
		//�ж���Ϸ �� ���� �� ��ͣ
		if (true == g_stop)
		{
			//�ж��Ƿ�Ե�ʳ��
			if (true == Eat())
			{
				SetSocre();	//��ӡС�߳���
				Food();		//��������ʳ��
			}

			//�ж��Ƿ���Ϸ����
			if (true == IsDie())
			{
				if (true == g_save)							//һ����Ϸֻ�ܱ���һ�Σ�Ȼ��������ʽ�Ͳ��ܱ�����
				{
					Save();									
					g_save = false;
				}
				system("pause>0");	//��סһ��
				continue;			//����ѭ����ʼ�����а����жϣ���Q���¿�ʼ Esc�˳���Ϸ
			}

			//����С���ƶ��Ĳ��� ���������ƶ�֮ǰ�������ƶ���β�ڵ�������仯��ԭ���Ĳ�������û��������Գ��Էŵ���ͬλ����ʲô�仯)
			Drop();
			//С���ƶ�
			Move();
			//ˢ�»���С��	(�������������׳��֣�˳��̶�)
			DrawSnake();
		}
		
		//�ӳ�һ������
		Sleep(g_speed);
	}

	//��Ϸ����������С���������ͷŵ�����Ŀռ��
	Free();

	system("pause>0");
	return 0;
}

void Head(void)
{
	printf("\n\n");
	printf("\t\t\t\t\t����������>>>>̰����<<<<����������\n");
	printf("\t\t\t\t\t��>>>     Enter ����/��ͣ    <<<��\n");
	printf("\t\t\t\t\t��>>>   �������� ���Ʒ���    <<<��\n");
	printf("\t\t\t\t\t��>>>     1 �鿴��ʷ��¼     <<<��\n");
	printf("\t\t\t\t\t��>>>     Q ���¿�ʼ��Ϸ     <<<��\n");
	printf("\t\t\t\t\t��>>>   Tab �л��Ѷȣ���   <<<��\n");
	printf("\t\t\t\t\t��>>>      ��ǰ���ȣ�        <<<��\n");
	printf("\t\t\t\t\t��>>>      Esc �˳���Ϸ      <<<��\n");
	printf("\t\t\t\t\t����������������������������������\n");
}
void Frame(void)
{
	//�ȸ��������鸳ֵ��Ϊ1����ʵ�ģ�Ϊ0������ģ��ո�
	for (int i = 0; i < X; i++)
	{
		for (int j = 0; j < Y; j++)
		{
			if(0 == i || 0 == j || (X - 1) == i || (Y - 1) == j)	//���ĸ����������Ǳ߿��λ��
				g_Back[i][j] = 1;									//�߿�ı������鸳ֵ1
			else
				g_Back[i][j] = 0;									//��Ϸ����Ǳ߿�ֵ0
		}
	}
	//��ӡ�߿���Ϸ����
	for (int i = 0; i < X; i++)
	{
		GotoXY(i + 11, 37);			//Ϊ���ÿ�����룬�ڴ�ӡ��֮ǰ�ȵ������λ��
		for (int j = 0; j < Y; j++)
		{
			if (1 == g_Back[i][j])	//��Ӧ����Ԫ��Ϊ1������ʵ��
				printf("��");
			else
				printf("  ");		//��Ӧ����Ԫ��Ϊ0����ӡ���ո�
		}
		putchar('\n');		//�ȴ�ӡ��һ�У�Ȼ��˴������л�����һ��
	}
}
void GotoXY(unsigned short hang, unsigned short lie)	//�˺����Ĳ�����short���ͣ���Ϊ����ĺ���ֻ֧��short����
{	
	//�˽ṹ��Ϊ�����溯��������ʹ�õ�
	COORD cd = { lie , hang };										//ע�����˳����X,Y  Ҳ����lie��hang X��Ӧ�� Y��Ӧ��
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cd);	//��1�������д��ھ�����������ID��ÿ����Ψһ�ģ�	��Ҫ<windows.h>
																	//�˺�������������Ĵ��ڣ���ָ�����λ��
}
void CreateSnake(int iCount)
{
	//�����Ϸ��Լ��
	if (0 >= iCount)	//û�нڵ������Ͳ�����
		return;
	//�����ڵ㣨C�����õ�malloc��Ҫͷ�ļ���
	for (int i = 0; i < iCount; i++)
	{
		//����ڵ㣨����malloc���ͱ���Ҫ�ͷţ�
		struct Node* pT = (struct Node*)malloc(sizeof(struct Node));
		if (NULL == pT)		//����ʧ�ܴ���
			return;
		//�½ڵ㸳ֵ(��ʼ��)
		pT->iCount = 0;
		pT->pPre = NULL;				//����NULL����Ҫ�������ʼ����NULL
		pT->pNext = NULL;
		pT->stSnake.emdir = enum_east;	//���½ڵ�һ��ʼ���ƶ�
		pT->stSnake.iX = 0;
		pT->stSnake.iY = 0;
		//�½ڵ������������
		if (NULL == g_pHead)			//����û�нڵ�ʱ
		{
			g_pHead = pT;				//�õ�ǰ�ڵ����������ʱ�ýڵ�ͬʱ��ͷβ�ڵ�
			g_pEnd = pT;
			g_pHead->iCount = 1;
			pT->stSnake.iX = rand() % (X - 2) + 1;			//�����Ǹýڵ���Ϸ��ʼ���ɵ����꣬�������������
			pT->stSnake.iY = rand() % (Y - iCount - 1) + 1 + 2;	//�����ı߽����������ʼǣ������˶��������������ܻ�ͻ�Ʊ߽磬-����+2���ɣ��÷�Χ��3~19֮�䣬�����Ӷ���������ʱ��������Խ��
		}
		else							//�����Ѿ����˽ڵ�
		{
			g_pEnd->pNext = pT;			//�½ڵ��������β���
			pT->pPre = g_pEnd;
			g_pEnd = pT;				//ע���߼�˳�򣬴˽ڵ��ѳ�Ϊ�����µ�β�ڵ�


			g_pHead->iCount += 1;		//�ڵ�������ͷ�ڵ�������

			//������β��ӣ������Ѿ��нڵ��ˣ����Ա��������һ���ڵ㣬�����������������ˢ������
			g_pEnd->stSnake.iX = g_pEnd->pPre->stSnake.iX;		//Ҫ����ϣ�����鳯�ĸ�����ʼ���Ժ���޸�
			g_pEnd->stSnake.iY = g_pEnd->pPre->stSnake.iY - 1;	//�������õ�ǰ�ڵ������һ�ڵ���ߣ�һ��ʼŪ���ˣ��޸ļ��ɣ�
			
		}
	}
	
}
//�ͷ�����Ŀռ�
void Free(void)
{
	//�����Ϸ��Լ��
	if (NULL == g_pHead)
		return;
	//��¼ͷ�ڵ㣬��Ҫֱ��ʹ��g_pHead����Ϊ��ȫ�ֱ������ܿ��ܳ���
	struct Node* pT = g_pHead;
	while (NULL != pT)
	{
		struct Node* pp = pT;				//��¼Ҫ���ͷŵĽڵ�
		pT = pT->pNext;						//ָ��������һ���ڵ�
		free(pp);							//�ͷŽڵ�
	}
	//ͷβָ�븳�ձ�����
	g_pHead = NULL;
	g_pEnd = NULL;

}
void DrawSnake(void)
{
	//�����Ϸ��Լ��
	if (NULL == g_pHead)
		return; 
	//��¼ͷ����
	struct Node* pT = g_pHead;
	while (pT != NULL)
	{
		//ָ����굽���Ӧ�ڵ����꣬�����ߵ����壨�ڵ㣩��*2����Ϊһ���ַ�ռ������stSnake�Ǵ�1��ʼ�ģ�����stSnake.iY����Ϊ1����ô�����37+2==39λ�ÿ�ʼ�����պò�������ǽ��
		GotoXY(11 + pT->stSnake.iX , 37 + pT->stSnake.iY*2);		//��������������,�ο���Frame�����еı߿��ӡ
		printf("��");					
		pT = pT->pNext;
	}

}
void Move(void)
{
	//�����Ϸ��Լ��
	if(NULL == g_pHead)
		return;
	//��¼β�ڵ㣨�����ƶ������ôӺ���ǰ������
	struct Node* pT = g_pEnd;
	//ѭ����������ͷ�ڵ㣩
	while(pT != g_pHead)					//���ѭ���Ҿ�Ȼ��if ɵ������
	{
		pT->stSnake = pT->pPre->stSnake;	//��β��ʼ��ǰһ���������ƶ�����һ��
		pT = pT->pPre;
	}
	//ͷ�ڵ�����⴦��ͷ�������ĸ������ߣ�
	switch (pT->stSnake.emdir) 
	{
	case enum_east:			//��
		g_pHead->stSnake.iY++;			//���iX iY ��������ʲô�һ���������iY���У��еı仯���������ƶ� iX���У��еı仯���������ƶ�
		break;
	case enum_south:		//��
		g_pHead->stSnake.iX++;
		break;	
	case enum_west:			//��
		g_pHead->stSnake.iY--;
		break;
	case enum_north:		//��
		g_pHead->stSnake.iX--;
		break;
	}
}
void Drop(void)
{
	//�����ƶ���Ϊ����β�ڵ���
	GotoXY(11 + g_pEnd->stSnake.iX, 37 + g_pEnd->stSnake.iY*2);
	//β�ڵ��λ�ã������ո�����
	printf("  ");
}
void KeyState(void)
{
	//�޶� �������Ҽ�������ͣʱ�޷�����
	if (true == g_stop)
	{
		if (GetAsyncKeyState(VK_UP) & 0x01)		//��
		{
			if (g_pHead->stSnake.emdir == enum_south)		//��ֹԭ�ص�ͷ����ԭ�ص�ͷʧЧ
				g_pHead->stSnake.emdir = enum_south;
			else if (g_pHead->stSnake.emdir == enum_north)	//�����Ѿ����˷����ƶ����ٰ��·�������൱��һ�μ���
			{
				//�ж��Ƿ�Ե�ʳ��	//����ʱҲ�����ж�ʳ�����ͳԲ���ʳ����
				if (true == Eat())
					Food();
				if (true == IsDie())//�ж���Ϸ�Ƿ����
				{
					if (true == g_save)						
					{
						Save();						
						g_save = false;
					}
					g_stop = false;
					KeyState();
				}
				Drop();			//�������׳��֣�����ۼ����ƶ���������״
				Move();			//ֻ���ƶ��Ļ�����Ȼ�����ˣ��������в���
				DrawSnake();
			}
			else				//�ƶ�����
			{
				g_pHead->stSnake.emdir = enum_north;
			}
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x01)	//��
		{
			if (g_pHead->stSnake.emdir == enum_north)		//��ֹԭ�ص�ͷ����ԭ�ص�ͷʧЧ
				g_pHead->stSnake.emdir = enum_north;
			else if (g_pHead->stSnake.emdir == enum_south)	//�ٰ��·�������൱��һ�μ���
			{
				if (true == Eat())
					Food();
				if (true == IsDie())//�ж���Ϸ�Ƿ����
				{
					if (true == g_save)						//һ����Ϸֻ�ܱ���һ�Σ�Ȼ��������ʽ�Ͳ��ܱ�����
					{
						Save();								//��������
						g_save = false;
					}
					g_stop = false;	//��ͣ
					KeyState();
				}
				Drop();
				Move();
				DrawSnake();
			}
			else
			{
				g_pHead->stSnake.emdir = enum_south;
			}
		}
		else if (GetAsyncKeyState(VK_LEFT) & 0x01)	//��
		{
			if (g_pHead->stSnake.emdir == enum_east)
				g_pHead->stSnake.emdir = enum_east;
			else if (g_pHead->stSnake.emdir == enum_west)
			{
				if (true == Eat())
					Food();
				if (true == IsDie())//�ж���Ϸ�Ƿ����
				{
					if (true == g_save)
					{
						Save();
						g_save = false;
					}
					g_stop = false;	//��ͣ
					KeyState();
				}
				Drop();
				Move();
				DrawSnake();
			}
			else
			{
				g_pHead->stSnake.emdir = enum_west;
			}

		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x01)//��
		{
			if (g_pHead->stSnake.emdir == enum_west)
				g_pHead->stSnake.emdir = enum_west;
			else if (g_pHead->stSnake.emdir == enum_east)
			{
				if (true == Eat())
					Food();
				if (true == IsDie())//�ж���Ϸ�Ƿ����
				{
					if (true == g_save)
					{
						Save();
						g_save = false;
					}
					g_stop = false;	//��ͣ
					KeyState();
				}
				Drop();
				Move();
				DrawSnake();
			}
			else
			{
				g_pHead->stSnake.emdir = enum_east;
			}
		}
	}
	
	if (GetAsyncKeyState('Q'))			//Q�� ע�����ʱ��һ���ǵ����żӴ�д
	{
		if (true == g_save)
		{
			Save();									//�����������
			g_save = false;				
		}
		NewGame();						//���¿�ʼ
	}
	else if (GetAsyncKeyState(VK_ESCAPE) & 0x01)	//Esc�� �˳���Ϸ����һ��ȫ�ֱ����������Ƿ��˳�ѭ��
	{
		if (true == g_save)							//һ����Ϸֻ�ܱ���һ�Σ�Ȼ��������ʽ�Ͳ��ܱ�����
		{
			Save();									//��������
			g_save = false;
		}
		g_quit = 0;
	}
	else if (GetAsyncKeyState(VK_RETURN) & 0x01)	//ENTER������������/��ͣ
	{
		g_stop = !g_stop;							//ȡ��
	}
	else if (GetAsyncKeyState(VK_TAB) & 0x01)
	{
		ChangeSpeed();
	}
	else if (GetAsyncKeyState('1') & 0x01)
	{
		Read();						//��ȡ�����ļ�
	}
}
void Food(void)
{
	while (1)
	{
		//����һ������
		int x = rand() % (X - 2) + 1;	//��������1~19֮�䣬�Ͳ�����ǽ���غ���
		int y = rand() % (Y - 2) + 1;
		//�жϸ������Ƿ�ˢ��������
		struct Node* pT = g_pHead;
		while (pT != NULL)
		{
			if (x == pT->stSnake.iX && y == pT->stSnake.iY)
				break;					//һ��ʳ�����꣬���߽ڵ������غϣ��˳���ǰѭ������ʱpT��¼�Ÿýڵ㣬���û���κ��غϣ�����pT == NULL
			pT = pT->pNext;
		}
		//���ˢ���������ϣ�������ѭ��һ�Σ�����������
		if (pT != NULL)					//���ʳ���������߽ڵ��������غϣ�pT���¼�Ǹ��ڵ��ַ��û���غϵĻ���pT������ѭ�����ջ���NULL
			continue;					//���¾���ѭ��������ˢ��ʳ������
		//���������ϣ����Ǻ��������
		if (pT == NULL)
		{
			//��ʳ�������λ�����鱳����ֵ
			g_Back[x][y] = 2;
			//�ڸ����껭��ʳ��
			GotoXY(11 + x, 37 + y * 2);
			printf("��");
			//����ѭ��
			break;
		}
	}
}
void Appand(void)
{
	//�����Ϸ��Լ��
	if (NULL == g_pHead)
		return;
	//С�߳�ʳ�ﱾ�����������β���
	//�����½ڵ�
	struct Node* pTemp = (struct Node*)malloc(sizeof(struct Node));
	if (NULL == pTemp)
		return;
	//�½ڵ��Ա��ֵ
	g_pHead->iCount++;		//��������ͷ�ڵ�������
	pTemp->pNext = NULL;
	pTemp->pPre = NULL;
	pTemp->iCount = 0;
	pTemp->stSnake.emdir = g_pEnd->stSnake.emdir;
	switch (g_pEnd->stSnake.emdir)
	{
	case enum_east:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX;
		pTemp->stSnake.iY = g_pEnd->stSnake.iY - 1;		//���β�ڵ��ǳ�������ô�ͻ���β�ڵ���߽����½ڵ�
		break;
	case enum_south:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX - 1;		//���β�ڵ��ǳ��ϣ���ô�ͻ���β�ڵ��Ϸ������½ڵ�
		pTemp->stSnake.iY = g_pEnd->stSnake.iY;
		break;
	case enum_west:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX;
		pTemp->stSnake.iY = g_pEnd->stSnake.iY + 1;		//���β�ڵ��ǳ�������ô�ͻ���β�ڵ��Ҳ�����½ڵ�
		break;
	case enum_north:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX + 1;		//���β�ڵ��ǳ�������ô�ͻ���β�ڵ��·������½ڵ�
		pTemp->stSnake.iY = g_pEnd->stSnake.iY;
		break;
	}
	//�½ڵ���ӵ�β����
	pTemp->pPre = g_pEnd;
	g_pEnd->pNext = pTemp;
	g_pEnd = pTemp;
}
bool Eat(void)
{
	//����֮ǰ�ı���������趨����ֵ���ж�ͷ�ڵ��Ƿ�����ʳ��
	if (2 == g_Back[g_pHead->stSnake.iX][g_pHead->stSnake.iY])
	{
		g_Back[g_pHead->stSnake.iX][g_pHead->stSnake.iY] = 0;	//���Ժ󱳾������0
		Appand();												//������
		return true;
	}
	else
		return false;
}
bool IsDie(void)
{
	//����ͷ�������߽�ʱ��Game Over!! Ҳ�����Զ��壬������ͷ���Լ�������ײ
	if (1 == g_Back[g_pHead->stSnake.iX][g_pHead->stSnake.iY])
	{
		GotoXY(20, 52);
		printf("Game Over!!");
		return true;
	}
	//�ߴ�������Ҳ�ᵼ�����������ñ���������û�и���Ч�ķ����أ��϶��У�������ڵ��Ա���һ��ֵ����ʶ�������ñ������飩
	struct Node* pT = g_pHead->pNext;
	while (pT != NULL)
	{
		if (pT->stSnake.iX == g_pHead->stSnake.iX && pT->stSnake.iY == g_pHead->stSnake.iY)
		{
			GotoXY(20, 52);
			printf("Game Over!!");
			return true;
		}
		pT = pT->pNext;
	}
	return false;
}
void SetSocre(void)
{
	GotoXY(8, 61);
	printf("%d", g_pHead->iCount);
}
void NewGame(void)
{
	system("cls");		//����		һ��Ҫ�������������ָ�������
	Free();				//�ͷ�ԭ��С�ߵ�����

	//���³�ʼ����������Ϸ�������ڰ�������е��ã���ִ����˺�������Ȼ��ѭ���м���ѭ�����ʹﵽ�����¿�ʼ��Ч��
	g_save = true;
	Head();
	Frame();
	CreateSnake(3);
	DrawSnake();
	Food();
	SetSocre();
}
void ChangeSpeed(void)
{
	static int g = 0;
	g++;
	switch (g%4)
	{
	case 0:			//��
		GotoXY(7, 62);
		printf("��");
		g_speed = 500;
		break;
	case 1:			//һ��
		GotoXY(7, 62);
		printf("һ��");
		g_speed = 350;
		break;
	case 2:			//����
		GotoXY(7, 62);
		printf("����");
		g_speed = 200;
		break;
	case 3:			//ج��
		GotoXY(7, 62);
		printf("ج��");
		g_speed = 50;
		break;
	}
}
void Save(void)
{
	//�˺�����3��λ�õ��ã���Ϸ�������˳���Ϸ�����¿�ʼ
	struct Record re;
	re.socre = g_pHead->iCount;
	re.t = time(NULL);			//��ǰ��ϵͳʱ�䣨�����ʽ��ȡʱҪ���ض���ʽ��
	//д���ļ�
	FILE* pFile = NULL;										
	errno_t res = fopen_s(&pFile, "record.dat", "a");		//���ļ���"a" �ǽ���д�� w�Ǹ���д��a�Ļ����д��������һ�ε�����д��w�ͻὫԭ�ȵĸ��ǵ�
	if (NULL == pFile || 0 != res)
		return;
	fwrite(&re, sizeof(re), 1, pFile);						//��ʽд�룬���Ǵ��ļ����ܹ۲쵽record.dat��������ģ�ԭ���Ǵ洢��ʽ����±��Ĵ򿪸�ʽ�ǲ�ͬ��
	fclose(pFile);											//������fread���ܶ�ȡ��Щ��Ϣ��
}
void Read(void)
{
	//���ļ�
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, "record.dat", "r");
	if (0 != res || NULL == pFile)
		return;
	//��ȡ�ļ�
	int hang = 11;
	struct Record re;					//�������ݵĽṹ��
	while (0 != fread(&re, sizeof(re), 1, pFile))
	{
		GotoXY(hang++, 78);				//����ÿ��ѭ���������Զ�ת����һ�д�ӡ��
		char str[50] = { 0 };
		ctime_s(str, 50, &re.t);		//��ʱ��ĸ�ʽ�ܷ���ʶ��
		printf("���ȣ�%d , ʱ�䣺%s ", re.socre, str);
	}
	fclose(pFile);						//fopen_s��һ���ļ�һ��Ҫ��fclose�������´�fopen_s���ļ��ͻ᷵��13������
}