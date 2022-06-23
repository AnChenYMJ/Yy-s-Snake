#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <malloc.h>
#include <time.h>
#include <stdbool.h>	//bool 要识别必须加
#include <share.h>		//_fsopen

//全局变量
#define X 20		//游戏区域的长宽(行列)
#define Y 20
char g_Back[X][Y];	//游戏区域的背景数组
enum Dir { enum_east, enum_south, enum_west, enum_north};	//枚举，用于操作蛇的移动方向（东南西北）默认从0 1 2 3开始
struct Node* g_pHead = NULL;								//蛇身链表的头节点
struct Node* g_pEnd = NULL;									//尾节点
int g_quit = 1;												//控制Esc键退出游戏
BOOL g_stop = false;										//控制暂停/启动
int g_speed = 500;											//小蛇移动的速度
BOOL g_save = true;											//一局游戏可被保存的标志

//界面相关
void Head(void);	//打印操作提示信息
void Frame(void);	//打印游戏区域（游戏的边框、边界）
void GotoXY(unsigned short hang, unsigned short lie);	//指定光标位置
void DrawSnake(void);									//画出游戏开始时的初始小蛇
void Drop(void);										//消除小蛇移动时，尾巴的残留
void Food(void);										//产生食物


		//当前节点的坐标与移动方向
struct Snake
{
	int iX;				//坐标
	int iY;
	enum Dir emdir;		//蛇的移动方向（东南西北）
};
		//蛇的节点
struct Node
{
							//写命名时的规范，如下：指针的名字前加p int类型前加i st则是结构体
	struct Snake stSnake;	//当前节点的坐标与移动方向
	int iCount;				//计数，记录有多少节点
	struct Node* pPre;		//上一个节点地址
	struct Node* pNext;		//下一个节点地址
};

//操作相关
void CreateSnake(int iCount);	//游戏最开始生成蛇,参数是生成几个节点
void Free(void);				//释放申请的空间
void Move(void);				//让小蛇开始移动
void KeyState(void);			//检测按键
void Appand(void);				//小蛇吃完食物长大（添加新节点）
bool Eat(void);					//吃食物
bool IsDie(void);				//判断小蛇死亡

//其它游戏功能
void SetSocre(void);			//打印小蛇长度
void NewGame(void);				//重新开始游戏，按键检测中调用
void ChangeSpeed(void);			//切换游戏难度

//保存数据
struct Record					//存储数据的结构体
{
	time_t t;					//记录时间
	int socre;					//记录长度
};			
void Save(void);				//保存到文件
void Read(void);				//读取文件数据

int main(void)
{
	g_save = true;							//初始化，此局能被保存
	srand((unsigned int)time(NULL));		//随机数种子，用当前系统时间触发，需要time头文件,强转是为了去警告
	//打印提示信息
	Head();
	//打印游戏区域的边框及背景
	Frame();
	//开始生成蛇的节点,参数是需要生成几个节点
	CreateSnake(3);
	//打印初始的小蛇
	DrawSnake();
	//生成食物
	Food();
	//打印小蛇长度
	SetSocre();

	while (1)
	{
		//检测按键	（要想更好游戏体验，就必须在Move之前检测按键，否则要移动一步才转向）
		KeyState();
		//判断是否退出游戏
		if (0 == g_quit)
		{
			GotoXY(20, 45);
			printf("已退出游戏,请按任意键继续");
			break;
		}
			
		//判断游戏 在 启动 或 暂停
		if (true == g_stop)
		{
			//判断是否吃到食物
			if (true == Eat())
			{
				SetSocre();	//打印小蛇长度
				Food();		//重新生成食物
			}

			//判断是否游戏结束
			if (true == IsDie())
			{
				if (true == g_save)							//一局游戏只能保存一次，然后其他方式就不能保存了
				{
					Save();									
					g_save = false;
				}
				system("pause>0");	//卡住一下
				continue;			//返回循环开始，进行按键判断，如Q重新开始 Esc退出游戏
			}

			//消除小蛇移动的残留 （必须在移动之前消除，移动后尾节点的坐标会变化，原来的残留还是没清除，可以尝试放到不同位置有什么变化)
			Drop();
			//小蛇移动
			Move();
			//刷新画出小蛇	(这三个函数配套出现，顺序固定)
			DrawSnake();
		}
		
		//延迟一点来画
		Sleep(g_speed);
	}

	//游戏结束，或者小蛇死亡，释放掉申请的空间●
	Free();

	system("pause>0");
	return 0;
}

void Head(void)
{
	printf("\n\n");
	printf("\t\t\t\t\t※※※※※>>>>贪吃蛇<<<<※※※※※\n");
	printf("\t\t\t\t\t※>>>     Enter 启动/暂停    <<<※\n");
	printf("\t\t\t\t\t※>>>   ↑←↓→ 控制方向    <<<※\n");
	printf("\t\t\t\t\t※>>>     1 查看历史纪录     <<<※\n");
	printf("\t\t\t\t\t※>>>     Q 重新开始游戏     <<<※\n");
	printf("\t\t\t\t\t※>>>   Tab 切换难度：简单   <<<※\n");
	printf("\t\t\t\t\t※>>>      当前长度：        <<<※\n");
	printf("\t\t\t\t\t※>>>      Esc 退出游戏      <<<※\n");
	printf("\t\t\t\t\t※※※※※※※※※※※※※※※※※\n");
}
void Frame(void)
{
	//先给背景数组赋值，为1代表实心，为0代表空心（空格）
	for (int i = 0; i < X; i++)
	{
		for (int j = 0; j < Y; j++)
		{
			if(0 == i || 0 == j || (X - 1) == i || (Y - 1) == j)	//这四个条件正好是边框的位置
				g_Back[i][j] = 1;									//边框的背景数组赋值1
			else
				g_Back[i][j] = 0;									//游戏区域非边框赋值0
		}
	}
	//打印边框及游戏区域
	for (int i = 0; i < X; i++)
	{
		GotoXY(i + 11, 37);			//为了让框对整齐，在打印框之前先调整光标位置
		for (int j = 0; j < Y; j++)
		{
			if (1 == g_Back[i][j])	//对应数组元素为1，则是实心
				printf("■");
			else
				printf("  ");		//对应数组元素为0，打印俩空格
		}
		putchar('\n');		//先打印第一行，然后此处换行切换到下一行
	}
}
void GotoXY(unsigned short hang, unsigned short lie)	//此函数的参数是short类型，因为下面的函数只支持short类型
{	
	//此结构体为了下面函数而产生使用的
	COORD cd = { lie , hang };										//注意参数顺序，是X,Y  也就是lie、hang X对应行 Y对应列
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cd);	//参1是命令行窗口句柄（句柄就是ID，每个都唯一的）	需要<windows.h>
																	//此函数绑定所传句柄的窗口，可指定光标位置
}
void CreateSnake(int iCount)
{
	//参数合法性检测
	if (0 >= iCount)	//没有节点数，就不成立
		return;
	//创建节点（C语言用到malloc，要头文件）
	for (int i = 0; i < iCount; i++)
	{
		//申请节点（用了malloc，就必须要释放）
		struct Node* pT = (struct Node*)malloc(sizeof(struct Node));
		if (NULL == pT)		//申请失败处理
			return;
		//新节点赋值(初始化)
		pT->iCount = 0;
		pT->pPre = NULL;				//这俩NULL最重要，必须初始化成NULL
		pT->pNext = NULL;
		pT->stSnake.emdir = enum_east;	//让新节点一开始向东移动
		pT->stSnake.iX = 0;
		pT->stSnake.iY = 0;
		//新节点关联到链表上
		if (NULL == g_pHead)			//链表还没有节点时
		{
			g_pHead = pT;				//让当前节点加入链表，此时该节点同时是头尾节点
			g_pEnd = pT;
			g_pHead->iCount = 1;
			pT->stSnake.iX = rand() % (X - 2) + 1;			//这俩是该节点游戏开始生成的坐标，用随机数来决定
			pT->stSnake.iY = rand() % (Y - iCount - 1) + 1 + 2;	//这俩的边界条件，看笔记，我用了东方向，所以左侧可能会突破边界，-在外+2即可，让范围在3~19之间，这样从东向西生成时，不会有越界
		}
		else							//链表已经有了节点
		{
			g_pEnd->pNext = pT;			//新节点进行链表尾添加
			pT->pPre = g_pEnd;
			g_pEnd = pT;				//注意逻辑顺序，此节点已成为链表新的尾节点


			g_pHead->iCount += 1;		//节点数量由头节点来计数

			//由于是尾添加，链表已经有节点了，所以必须对照上一个节点，不能再用随机数生成刷新坐标
			g_pEnd->stSnake.iX = g_pEnd->pPre->stSnake.iX;		//要看你希望方块朝哪个方向开始，以后可修改
			g_pEnd->stSnake.iY = g_pEnd->pPre->stSnake.iY - 1;	//这样做让当前节点接在上一节点左边（一开始弄错了，修改即可）
			
		}
	}
	
}
//释放申请的空间
void Free(void)
{
	//参数合法性检测
	if (NULL == g_pHead)
		return;
	//记录头节点，不要直接使用g_pHead，因为是全局变量，很可能出错
	struct Node* pT = g_pHead;
	while (NULL != pT)
	{
		struct Node* pp = pT;				//记录要被释放的节点
		pT = pT->pNext;						//指针走向下一个节点
		free(pp);							//释放节点
	}
	//头尾指针赋空别忘了
	g_pHead = NULL;
	g_pEnd = NULL;

}
void DrawSnake(void)
{
	//参数合法性检测
	if (NULL == g_pHead)
		return; 
	//记录头对象
	struct Node* pT = g_pHead;
	while (pT != NULL)
	{
		//指定光标到达对应节点坐标，画出蛇的身体（节点），*2是因为一个字符占两格且stSnake是从1开始的，假设stSnake.iY坐标为1，那么将会从37+2==39位置开始画，刚好不会碰到墙壁
		GotoXY(11 + pT->stSnake.iX , 37 + pT->stSnake.iY*2);		//具体坐标请推算,参考了Frame函数中的边框打印
		printf("●");					
		pT = pT->pNext;
	}

}
void Move(void)
{
	//参数合法性检测
	if(NULL == g_pHead)
		return;
	//记录尾节点（本次移动，采用从后向前遍历）
	struct Node* pT = g_pEnd;
	//循环遍历（非头节点）
	while(pT != g_pHead)					//妈的循环我居然用if 傻逼了我
	{
		pT->stSnake = pT->pPre->stSnake;	//从尾开始，前一个的坐标移动到后一个
		pT = pT->pPre;
	}
	//头节点的特殊处理（头决定向哪个方向走）
	switch (pT->stSnake.emdir) 
	{
	case enum_east:			//东
		g_pHead->stSnake.iY++;			//这个iX iY 究竟代表什么我还不清晰，iY是列，列的变化导致左右移动 iX是行，行的变化导致上下移动
		break;
	case enum_south:		//南
		g_pHead->stSnake.iX++;
		break;	
	case enum_west:			//西
		g_pHead->stSnake.iY--;
		break;
	case enum_north:		//北
		g_pHead->stSnake.iX--;
		break;
	}
}
void Drop(void)
{
	//坐标移动到为链表尾节点上
	GotoXY(11 + g_pEnd->stSnake.iX, 37 + g_pEnd->stSnake.iY*2);
	//尾节点的位置，用俩空格消除
	printf("  ");
}
void KeyState(void)
{
	//限定 上下左右键，在暂停时无法操作
	if (true == g_stop)
	{
		if (GetAsyncKeyState(VK_UP) & 0x01)		//↑
		{
			if (g_pHead->stSnake.emdir == enum_south)		//防止原地掉头，让原地掉头失效
				g_pHead->stSnake.emdir = enum_south;
			else if (g_pHead->stSnake.emdir == enum_north)	//当此已经朝此方向移动，再按下方向键就相当于一次加速
			{
				//判断是否吃到食物	//加速时也必须判断食物，否则就吃不到食物了
				if (true == Eat())
					Food();
				if (true == IsDie())//判断游戏是否结束
				{
					if (true == g_save)						
					{
						Save();						
						g_save = false;
					}
					g_stop = false;
					KeyState();
				}
				Drop();			//这仨配套出现：清除痕迹、移动、画新形状
				Move();			//只有移动的话，虽然加速了，但还是有残留
				DrawSnake();
			}
			else				//移动方向
			{
				g_pHead->stSnake.emdir = enum_north;
			}
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x01)	//↓
		{
			if (g_pHead->stSnake.emdir == enum_north)		//防止原地掉头，让原地掉头失效
				g_pHead->stSnake.emdir = enum_north;
			else if (g_pHead->stSnake.emdir == enum_south)	//再按下方向键就相当于一次加速
			{
				if (true == Eat())
					Food();
				if (true == IsDie())//判断游戏是否结束
				{
					if (true == g_save)						//一局游戏只能保存一次，然后其他方式就不能保存了
					{
						Save();								//保存数据
						g_save = false;
					}
					g_stop = false;	//暂停
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
		else if (GetAsyncKeyState(VK_LEFT) & 0x01)	//←
		{
			if (g_pHead->stSnake.emdir == enum_east)
				g_pHead->stSnake.emdir = enum_east;
			else if (g_pHead->stSnake.emdir == enum_west)
			{
				if (true == Eat())
					Food();
				if (true == IsDie())//判断游戏是否结束
				{
					if (true == g_save)
					{
						Save();
						g_save = false;
					}
					g_stop = false;	//暂停
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
		else if (GetAsyncKeyState(VK_RIGHT) & 0x01)//→
		{
			if (g_pHead->stSnake.emdir == enum_west)
				g_pHead->stSnake.emdir = enum_west;
			else if (g_pHead->stSnake.emdir == enum_east)
			{
				if (true == Eat())
					Food();
				if (true == IsDie())//判断游戏是否结束
				{
					if (true == g_save)
					{
						Save();
						g_save = false;
					}
					g_stop = false;	//暂停
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
	
	if (GetAsyncKeyState('Q'))			//Q键 注意调用时，一定是单引号加大写
	{
		if (true == g_save)
		{
			Save();									//保存这局数据
			g_save = false;				
		}
		NewGame();						//重新开始
	}
	else if (GetAsyncKeyState(VK_ESCAPE) & 0x01)	//Esc键 退出游戏，用一个全局变量来控制是否退出循环
	{
		if (true == g_save)							//一局游戏只能保存一次，然后其他方式就不能保存了
		{
			Save();									//保存数据
			g_save = false;
		}
		g_quit = 0;
	}
	else if (GetAsyncKeyState(VK_RETURN) & 0x01)	//ENTER键，控制启动/暂停
	{
		g_stop = !g_stop;							//取反
	}
	else if (GetAsyncKeyState(VK_TAB) & 0x01)
	{
		ChangeSpeed();
	}
	else if (GetAsyncKeyState('1') & 0x01)
	{
		Read();						//读取数据文件
	}
}
void Food(void)
{
	while (1)
	{
		//产生一组坐标
		int x = rand() % (X - 2) + 1;	//让坐标在1~19之间，就不会与墙壁重合了
		int y = rand() % (Y - 2) + 1;
		//判断该坐标是否刷在蛇身上
		struct Node* pT = g_pHead;
		while (pT != NULL)
		{
			if (x == pT->stSnake.iX && y == pT->stSnake.iY)
				break;					//一旦食物坐标，与蛇节点坐标重合，退出当前循环，此时pT记录着该节点，如果没有任何重合，最终pT == NULL
			pT = pT->pNext;
		}
		//如果刷在了蛇身上，就重新循环一次，生成新坐标
		if (pT != NULL)					//如果食物坐标与蛇节点坐标有重合，pT会记录那个节点地址，没有重合的话，pT经历过循环最终会是NULL
			continue;					//重新经历循环，重新刷新食物坐标
		//不在蛇身上，则是合理的坐标
		if (pT == NULL)
		{
			//给食物的坐标位置数组背景赋值
			g_Back[x][y] = 2;
			//在该坐标画出食物
			GotoXY(11 + x, 37 + y * 2);
			printf("□");
			//跳出循环
			break;
		}
	}
}
void Appand(void)
{
	//参数合法性检测
	if (NULL == g_pHead)
		return;
	//小蛇吃食物本质上是链表的尾添加
	//创建新节点
	struct Node* pTemp = (struct Node*)malloc(sizeof(struct Node));
	if (NULL == pTemp)
		return;
	//新节点成员赋值
	g_pHead->iCount++;		//计数交给头节点来计数
	pTemp->pNext = NULL;
	pTemp->pPre = NULL;
	pTemp->iCount = 0;
	pTemp->stSnake.emdir = g_pEnd->stSnake.emdir;
	switch (g_pEnd->stSnake.emdir)
	{
	case enum_east:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX;
		pTemp->stSnake.iY = g_pEnd->stSnake.iY - 1;		//如果尾节点是朝东，那么就会在尾节点左边接上新节点
		break;
	case enum_south:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX - 1;		//如果尾节点是朝南，那么就会在尾节点上方接上新节点
		pTemp->stSnake.iY = g_pEnd->stSnake.iY;
		break;
	case enum_west:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX;
		pTemp->stSnake.iY = g_pEnd->stSnake.iY + 1;		//如果尾节点是朝西，那么就会在尾节点右侧接上新节点
		break;
	case enum_north:
		pTemp->stSnake.iX = g_pEnd->stSnake.iX + 1;		//如果尾节点是朝北，那么就会在尾节点下方接上新节点
		pTemp->stSnake.iY = g_pEnd->stSnake.iY;
		break;
	}
	//新节点添加到尾巴上
	pTemp->pPre = g_pEnd;
	g_pEnd->pNext = pTemp;
	g_pEnd = pTemp;
}
bool Eat(void)
{
	//利用之前的背景数组的设定特殊值来判断头节点是否碰到食物
	if (2 == g_Back[g_pHead->stSnake.iX][g_pHead->stSnake.iY])
	{
		g_Back[g_pHead->stSnake.iX][g_pHead->stSnake.iY] = 0;	//被吃后背景数组归0
		Appand();												//蛇身长大
		return true;
	}
	else
		return false;
}
bool IsDie(void)
{
	//当蛇头触碰到边界时就Game Over!! 也可以自定义，比如蛇头与自己身体碰撞
	if (1 == g_Back[g_pHead->stSnake.iX][g_pHead->stSnake.iY])
	{
		GotoXY(20, 52);
		printf("Game Over!!");
		return true;
	}
	//蛇触碰自身也会导致死亡（利用遍历，但有没有更高效的方法呢，肯定有，比如给节点成员添加一个值来标识，或者用背景数组）
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
	system("cls");		//清屏		一定要清屏，否则会出现覆盖问题
	Free();				//释放原来小蛇的链表

	//重新初始化界面与游戏，由于在按键检测中调用，故执行完此函数，仍然在循环中继续循环，就达到了重新开始的效果
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
	case 0:			//简单
		GotoXY(7, 62);
		printf("简单");
		g_speed = 500;
		break;
	case 1:			//一般
		GotoXY(7, 62);
		printf("一般");
		g_speed = 350;
		break;
	case 2:			//困难
		GotoXY(7, 62);
		printf("困难");
		g_speed = 200;
		break;
	case 3:			//噩梦
		GotoXY(7, 62);
		printf("噩梦");
		g_speed = 50;
		break;
	}
}
void Save(void)
{
	//此函数在3个位置调用，游戏结束、退出游戏、重新开始
	struct Record re;
	re.socre = g_pHead->iCount;
	re.t = time(NULL);			//当前的系统时间（这个格式读取时要用特定格式）
	//写入文件
	FILE* pFile = NULL;										
	errno_t res = fopen_s(&pFile, "record.dat", "a");		//打开文件，"a" 是接着写， w是覆盖写，a的话多次写入会接着上一次的内容写，w就会将原先的覆盖掉
	if (NULL == pFile || 0 != res)
		return;
	fwrite(&re, sizeof(re), 1, pFile);						//正式写入，我们打开文件夹能观察到record.dat内是乱码的，原因是存储格式与记事本的打开格式是不同的
	fclose(pFile);											//我们用fread就能读取这些信息了
}
void Read(void)
{
	//打开文件
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, "record.dat", "r");
	if (0 != res || NULL == pFile)
		return;
	//读取文件
	int hang = 11;
	struct Record re;					//接收数据的结构体
	while (0 != fread(&re, sizeof(re), 1, pFile))
	{
		GotoXY(hang++, 78);				//这样每次循环，都会自动转到下一行打印了
		char str[50] = { 0 };
		ctime_s(str, 50, &re.t);		//让时间的格式能方便识别
		printf("长度：%d , 时间：%s ", re.socre, str);
	}
	fclose(pFile);						//fopen_s打开一个文件一定要用fclose，否则下次fopen_s打开文件就会返回13错误码
}