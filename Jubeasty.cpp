#include <windows.h>
#include <mmsystem.h>
#include <io.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <fmod.h>
#include "./FMOD/inc/fmod.hpp"
#include "./FMOD/inc/fmod_errors.h"

#define EPSILON 0.000001

#define Screen_Width 900
#define Screen_Height 785 //770
#define TimeLine_Frame 150

#define Block_Size 150

#define VK_Z 90
#define VK_X 88
#define BackSpace 8

#define BACKCOLOR RGB(0,255,0)

// FMOD
#ifdef _DEBUG
	#pragma comment(lib, "./FMOD/lib/fmodexL_vc")
#else
#pragma comment(lib, "./FMOD/lib/fmodex_vc") 
#endif


HINSTANCE g_hInst;
RECT crt;
int Score;
int FPS;

char* Music;
int BPM;
int Song_Length;
double Cell;
double S_Scale;

int isMouseDown=0;
int isMusicPlay=0;

HBITMAP Note;
HBITMAP BNote;
HBITMAP BackGround;
HBITMAP Back_TimeLine;
HBITMAP Back_SideMenu;
HBITMAP ANIME;

HFONT Font1;
HFONT Font2;


	FMOD::System     *g_system;
	FMOD::Sound      *sound1;
	FMOD::Channel    *channel = 0;
	FMOD_RESULT       result;

	unsigned int      version;
	unsigned long     lCount=0;



class Block{
public:
	int Index;// 0 ~ 15
	int Time; // MinTime ~ MaxTime
	Block(int index=-1, int time=-1){
		Index=index;
		Time=time;
	}
	~Block(){
	}
};
Block* Blocks[10000];
int Bcount=0;

class TimeLine{
public:
	double OffSet;
	double CurrentTime;
	double TimeNode;
	int MinTime;
	int MaxTime;
	int Dtime;
	TimeLine(int MinTime_t=0, int MaxTime_t=Song_Length){
		MinTime=MinTime_t;
		MaxTime=MaxTime_t;
		CurrentTime=0;
		Dtime=-1;
		OffSet=0;
	}
	void Counting(double scale=1){
		int t=GetTickCount();
		if(Dtime==-1)	Dtime=t;
		CurrentTime+=scale*(double)(t-Dtime)/1000;
		Dtime=t;
		if(MaxTime<CurrentTime)	CurrentTime=MaxTime, isMusicPlay=0;
	}
	~TimeLine(){
	}
};

TimeLine* TLine;

int Load(){
	if(access("Music.jbt",04))	return 0;	// ������ ���� ����
	FILE *in=fopen("Music.jbt","r");

	Music=new char[1000];
	fscanf(in,"%s\n",Music);

	fscanf(in,"%d\n",&BPM);
	Cell=(double)60/BPM/4;

	fscanf(in,"%d\n",&Song_Length);

	//��� �Է�
	fscanf(in,"%d\n",&Bcount);
	for(int i=0;i<Bcount;i++){
		Blocks[i] = new Block();
		fscanf(in,"%d %d\n",&Blocks[i]->Index, &Blocks[i]->Time);
	}


	return 1;
}

void Initialize(){

	Note = (HBITMAP)LoadImage(g_hInst, "Image\\type1.bmp", NULL, 100, 100, LR_LOADFROMFILE);
	BNote = (HBITMAP)LoadImage(g_hInst, "Image\\none.bmp", NULL, 100, 100, LR_LOADFROMFILE);
	BackGround = (HBITMAP)LoadImage(g_hInst, "Image\\ba ckground.bmp", NULL, 600, 750, LR_LOADFROMFILE);
	Back_TimeLine = (HBITMAP)LoadImage(g_hInst, "Image\\TimeLine.bmp", NULL, 650, 150, LR_LOADFROMFILE);
	Back_SideMenu = (HBITMAP)LoadImage(g_hInst, "Image\\SideMenu.bmp", NULL, 300, 750, LR_LOADFROMFILE);
	ANIME = (HBITMAP)LoadImage(g_hInst, "Image\\Animation.bmp", NULL, 400, 400, LR_LOADFROMFILE);

	Font1 = CreateFont(13,0,0,0,400,0,0,0,HANGEUL_CHARSET,0,0,0,0,TEXT("����"));
	Font2 = CreateFont(30,0,0,0,500,0,0,0,HANGEUL_CHARSET,0,0,0,0,TEXT("���� ���"));

	if(Load()){
	}else{
		Music = TEXT("Music\\Area7.mp3");
		Song_Length=100;// second
		BPM=130;
		Cell=(double)60/BPM/4;
	}
	
	TLine = new TimeLine();
	S_Scale=(double)(Block_Size*4-50)/(TLine->MaxTime-TLine->MinTime);
	TLine->OffSet=200;
	isMusicPlay=0;

	Score=0;

	//���� ���
	result=FMOD::System_Create(&g_system); //FMOD �ý��� ����
	if (result != FMOD_OK)
        {
                printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
                return;
        }
	g_system->getVersion(&version); //���� Ȯ��
	g_system->init(32, FMOD_INIT_NORMAL, 0); //���� ä�� 32��
	g_system->createSound(Music, FMOD_HARDWARE | FMOD_3D_WORLDRELATIVE, 0, &sound1); //���ҽ� ���� ����
	sound1->setMode(FMOD_LOOP_OFF); //���� ����
	if(isMusicPlay)	g_system->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel);//���
	else			g_system->playSound(FMOD_CHANNEL_FREE, sound1, true, &channel); //����

	channel->setPosition(TLine->OffSet,true);

}

int count=0;
int Ftime=-1;

void Timer(){
	if(Ftime==-1)	count=0,Ftime=GetTickCount();
	while(count>(GetTickCount()-Ftime)*60/1000);
	if(GetTickCount()-Ftime>=1000)	Ftime=GetTickCount(), FPS=count, count=0;

	count++;

	int num=(TLine->CurrentTime+EPSILON)/Cell;
	TLine->TimeNode=(double)num*Cell;

	if(!isMusicPlay){
		TLine->Dtime=GetTickCount();
		TLine->CurrentTime=TLine->TimeNode;
	}
	if(isMusicPlay)		TLine->Counting(1);

	bool Playing;
	channel->isPlaying(&Playing);
	if(!Playing){
			if(isMusicPlay)	g_system->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel);//���
			else			g_system->playSound(FMOD_CHANNEL_FREE, sound1, true, &channel); //����

	}
}

void DrawDetail(HDC hMemDC){
	HDC hMemDC2 = CreateCompatibleDC(hMemDC);
	HBITMAP OldBitmap;
    HFONT OldFont;

	static char LStr[128];


	//��� ���
	OldBitmap = (HBITMAP)SelectObject(hMemDC2,BackGround);
	BitBlt(hMemDC,0,0,Block_Size*4,750,hMemDC2,0,0,SRCCOPY);
	SelectObject(hMemDC2,OldBitmap);

	//���̵� �޴� ���

	SetBkMode(hMemDC, 1);
	OldFont = (HFONT)SelectObject(hMemDC,Font2);
	OldBitmap = (HBITMAP)SelectObject(hMemDC2,Back_SideMenu);
	BitBlt(hMemDC,Block_Size*4,0,300,750,hMemDC2,0,0,SRCCOPY);
	SelectObject(hMemDC2,OldBitmap);

	sprintf(LStr,"MUSIC:%s",Music);//MUSIC
	TextOutA(hMemDC,Block_Size*4+25,50,LStr,lstrlenA(LStr));

	sprintf(LStr,"BPM:%d",BPM);//BPM
	TextOutA(hMemDC,Block_Size*4+25,100,LStr,lstrlenA(LStr));

	sprintf(LStr,"OffSet:%d",(int)TLine->OffSet);//OFFSET
	TextOutA(hMemDC,Block_Size*4+25,150,LStr,lstrlenA(LStr));

	sprintf(LStr,"Number of Block:%d",Bcount);//Bcount
	TextOutA(hMemDC,Block_Size*4+25,200,LStr,lstrlenA(LStr));

	sprintf(LStr,"Score per Block:%d",(int)((double)1000000/Bcount));//Bcount
	TextOutA(hMemDC,Block_Size*4+25,250,LStr,lstrlenA(LStr));

	SelectObject(hMemDC,OldFont);

	//��� ���


	int i,j;
	for(i=0;i<16;i++){
		for(j=0;j<Bcount;j++){
			if(Blocks[j]->Index==i && 0>=((int)((TLine->TimeNode+EPSILON)/Cell)-Blocks[j]->Time)){//��Ʈ
				int number=((int)((TLine->TimeNode+EPSILON)/Cell)-Blocks[j]->Time)+15;
				OldBitmap=(HBITMAP)SelectObject(hMemDC2,ANIME);
				TransparentBlt(hMemDC, Block_Size*(i%4), Block_Size*(i/4), Block_Size, Block_Size, hMemDC2, 100*(int)(number%4), 100*(int)(number/4), 100, 100, RGB(0,255,0));
				SelectObject(hMemDC2,OldBitmap);
			}
		}
		if(j==Bcount){//��ĭ
			OldBitmap = (HBITMAP)SelectObject(hMemDC2,BNote);
			TransparentBlt(hMemDC, Block_Size*(i%4), Block_Size*(i/4), Block_Size, Block_Size, hMemDC2, 0, 0, 100, 100, BACKCOLOR);
			SelectObject(hMemDC2,OldBitmap);
		}
	}

	//Ÿ�Ӷ��� ���
	int CL=TLine->TimeNode*S_Scale;
	OldBitmap = (HBITMAP)SelectObject(hMemDC2,Back_TimeLine);
	TransparentBlt(hMemDC,0,Block_Size*4,Block_Size*4,TimeLine_Frame,hMemDC2, 0, 0, 650, 150, BACKCOLOR);
	SelectObject(hMemDC2,OldBitmap);

	SetBkMode(hMemDC, 2);
	sprintf(LStr,"%d",TLine->MinTime);//������
	TextOutA(hMemDC,28,Block_Size*4+TimeLine_Frame-30,LStr,lstrlenA(LStr));

	sprintf(LStr,"%d",TLine->MaxTime);//����
	TextOutA(hMemDC,Block_Size*4-30-lstrlenA(LStr)*7,Block_Size*4+TimeLine_Frame-30,LStr,lstrlenA(LStr));

/*	sprintf(LStr,"%.3f",TLine->TimeNode);//������
	TextOutA(hMemDC,28+CL,Block_Size*4+TimeLine_Frame-43,LStr,lstrlenA(LStr));
	sprintf(LStr,"%.3f",TLine->CurrentTime);//������
	TextOutA(hMemDC,28+CL,Block_Size*4+TimeLine_Frame-30,LStr,lstrlenA(LStr));
*/	
	Rectangle(hMemDC,23+CL,Block_Size*4+5,27+CL,Block_Size*4+TimeLine_Frame-5);//Ÿ�Ӷ���

	i=0;
	while(i*Cell<TLine->MaxTime){//���� ���
		MoveToEx(hMemDC,23+i*Cell*S_Scale,Block_Size*4+10,NULL);
		LineTo(hMemDC,23+i*Cell*S_Scale,Block_Size*4+TimeLine_Frame-10);
		i+=16;
	}

	sprintf(LStr,"%d/16",(int)((TLine->TimeNode+EPSILON)/Cell)%16);//�������� ������ ��ġ
//	TextOutA(hMemDC,28+CL,Block_Size*4+TimeLine_Frame-56,LStr,lstrlenA(LStr));
	TextOutA(hMemDC,28+CL,Block_Size*4+TimeLine_Frame-43,LStr,lstrlenA(LStr));

	DeleteDC(hMemDC2);
}

void MSG_PAINT(HWND hWnd)
{
	char LStr[128];

	PAINTSTRUCT ps;
	HDC hdc, hMemDC;
	HBITMAP hBitmap, OldBitmap;
	HBRUSH BGBrush;
    HFONT OldFont;

	hdc = BeginPaint(hWnd,&ps);
	hMemDC = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	OldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	OldFont = (HFONT)SelectObject(hMemDC,Font1);

	SetBkMode(hMemDC, 1);
	SetTextColor(hMemDC, 0x000000);	// ��Ʈ��
	SetBkColor(hMemDC, 0xffffff);	// ����

//��ü ��� �ʱ�ȭ
		BGBrush = CreateSolidBrush(0xffffff);
		FillRect(hMemDC, &crt, BGBrush);
		DeleteObject(BGBrush);

		DrawDetail(hMemDC); // ȭ�� ���

		SetBkMode(hMemDC, 2);
		wsprintf(LStr,"FPS : %d",FPS);
		TextOutA(hMemDC,10,10,LStr,lstrlenA(LStr));
		wsprintf(LStr,"BPM : %d",BPM);
		TextOutA(hMemDC,10,23,LStr,lstrlenA(LStr));

	BitBlt(hdc, 0, 0, crt.right, crt.bottom, hMemDC, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(hMemDC, OldFont));
	DeleteObject(SelectObject(hMemDC, OldBitmap));
	DeleteDC(hMemDC);
	DeleteDC(hdc);

	EndPaint(hWnd,&ps);
}

int x, y;

void LBUTTONDOWN( WPARAM wParam, LPARAM lParam ){
	x=LOWORD(lParam);
	y=HIWORD(lParam);

	if(Block_Size*4+10<=y && y<=Block_Size*4+TimeLine_Frame-10){
		if(x<=25)	x=25;
		if(x>=Block_Size*4-25)	x=Block_Size*4-25;
		TLine->CurrentTime=(double)(x-25)*(TLine->MaxTime-TLine->MinTime)/(Block_Size*4-50);
		channel->setPosition(TLine->OffSet+TLine->CurrentTime*1000,true);

		isMouseDown=1;
	}
	if(Block_Size*4+10>y){
		if(0<=x && x<=Block_Size*4){
			int sw=0;
			for(int i=0;i<Bcount;i++){
				if(Blocks[i]->Index==(y/Block_Size)*4+(x/Block_Size)){
					if(fabs((TLine->TimeNode+EPSILON)/Cell-Blocks[i]->Time)<Cell){
//						(void*)(Blocks[i]); //Block �Ҵ� ����
						for(int j=i;j<Bcount-1;j++)
							Blocks[j]=Blocks[j+1];
						Blocks[Bcount-1]=NULL;
						Bcount--;
						sw=1;
					}
				}
			}
			if(sw==0){
				Blocks[Bcount] = new Block((y/Block_Size)*4+(x/Block_Size), (TLine->TimeNode+EPSILON)/Cell);
				Bcount++;
			}
		}
		isMouseDown=1;
	}
}

void LBUTTONMOVE( WPARAM wParam, LPARAM lParam ){
	int isSameBlock=0;
	if(isMouseDown){
		if(x/Block_Size==LOWORD(lParam)/Block_Size)
			if(y/Block_Size==HIWORD(lParam)/Block_Size)
				isSameBlock=1;

		x=LOWORD(lParam);
		y=HIWORD(lParam);

		if(Block_Size*4+10<=y && y<=Block_Size*4+TimeLine_Frame-10){
			if(x<=25)	x=25;
			if(x>=Block_Size*4-25)	x=Block_Size*4-25;
			TLine->CurrentTime=(double)(x-25)*(TLine->MaxTime-TLine->MinTime)/(Block_Size*4-50);
			channel->setPosition(TLine->OffSet+TLine->CurrentTime*1000,true);
		}

		if(isSameBlock)
			return;
		if(Block_Size*4+10>y){
			if(0<=x && x<=Block_Size*4){
				int sw=0;
				for(int i=0;i<Bcount;i++){
					if(Blocks[i]->Index==(y/Block_Size)*4+(x/Block_Size)){
						if(fabs((TLine->TimeNode+EPSILON)/Cell-Blocks[i]->Time)<Cell){
	//						(void*)(Blocks[i]); //Block �Ҵ� ����
							for(int j=i;j<Bcount-1;j++)
								Blocks[j]=Blocks[j+1];
							Blocks[Bcount-1]=NULL;
							Bcount--;
							sw=1;
						}
					}
				}
				if(sw==0){
					Blocks[Bcount] = new Block((y/Block_Size)*4+(x/Block_Size), (TLine->TimeNode+EPSILON)/Cell);
					Bcount++;
				}
			}
		}

	}
}

void KEYBOARDDOWN( WPARAM wParam, LPARAM lParam ){
	char key = wParam;
	switch(key){
	case VK_RIGHT:
		TLine->CurrentTime+=Cell+EPSILON;
		if(TLine->CurrentTime>TLine->MaxTime)	TLine->CurrentTime=TLine->MaxTime;
		channel->setPosition(TLine->OffSet+TLine->CurrentTime*1000,true);
		break;
	case VK_LEFT:
		TLine->CurrentTime-=Cell-EPSILON;
		if(TLine->CurrentTime<TLine->MinTime)	TLine->CurrentTime=TLine->MinTime;
		channel->setPosition(TLine->OffSet+TLine->CurrentTime*1000,true);
		break;
	case 32:
		isMusicPlay = 1 - isMusicPlay;
		if(isMusicPlay){
			channel->setPosition(TLine->OffSet+TLine->CurrentTime*1000,true);
			channel->setPaused(false);
		}else{
			channel->setPaused(true);
		}
		break;
	case 27:
		PostQuitMessage(0);
		break;
	}
}

void SaveFile(){
	FILE *out=fopen("Music.jbt","w");
	fprintf(out,"%s\n",Music);
	fprintf(out,"%d\n",BPM);
	fprintf(out,"%d\n",Song_Length);
	//��� ���
	fprintf(out,"%d\n",Bcount);
	for(int i=0;i<Bcount;i++)
		fprintf(out,"%d %d\n",Blocks[i]->Index, Blocks[i]->Time);
	fclose(out);
}

void QUIT(){
	sound1->release();
	g_system->close();
	g_system->release();

}

LRESULT CALLBACK WndProc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam )
{
	switch(iMessage) {
	case WM_TIMER:		Timer();
						InvalidateRect(hWnd, NULL, FALSE);	break;
	case WM_PAINT:		MSG_PAINT(hWnd);					break;
	case WM_LBUTTONDOWN:	LBUTTONDOWN(wParam,lParam);		break;
	case WM_MOUSEMOVE:		LBUTTONMOVE(wParam,lParam);		break;
	case WM_LBUTTONUP:		isMouseDown=0;					break;
	case WM_KEYDOWN:		KEYBOARDDOWN(wParam,lParam);	break;
	case WM_CREATE:		
						Initialize();
						GetClientRect(hWnd, &crt);
						SetTimer(hWnd,0,1,NULL);			break;
	case WM_DESTROY:	PostQuitMessage(0);					break;
	}
	return DefWindowProc( hWnd, iMessage, wParam, lParam );
}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow )
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = CreateSolidBrush(RGB( 255,255,255 ) );
	WndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	WndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = ( WNDPROC )WndProc;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = TEXT( "TEST" );
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass( &WndClass );

	hWnd = CreateWindow( TEXT( "TEST" ), TEXT( "C++ Exemple" ),
	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 150, 0,
	Screen_Width, Screen_Height, NULL, (HMENU)NULL, hInstance, NULL );

	g_hInst=hInstance;
 
	ShowWindow( hWnd, nCmdShow );

	while( GetMessage( &Message, NULL, 0, 0 ) ){
		TranslateMessage( &Message );
		DispatchMessage( &Message );
	}

//	SaveFile();
	QUIT();

	return (int)Message.wParam;
}





















/*


// FMODTest1.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

#include <windows.h>

#include "./FMOD/inc/fmod.hpp"
#include "./FMOD/inc/fmod_errors.h"


// FMOD
#ifdef _DEBUG
#pragma comment(lib, "./FMOD/lib/fmodexL_vc")
#else
#pragma comment(lib, "./FMOD/lib/fmodex_vc") 
#endif


// ������ ������ݴϴ�.
void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	FMOD::System     *system;
	FMOD::Sound      *sound1;
	FMOD::Channel    *channel = 0;
	FMOD_RESULT       result;

	unsigned int      version;
	unsigned long     lCount=0;


	result = FMOD::System_Create(&system);
	ERRCHECK(result);

	result = system->getVersion(&version);
	ERRCHECK(result);


	// ����üũ�� �ʼ�, ���߽� ���� �ٸ������� ����ϸ� dll���� ������ �˴ϴ�.
	if (version < FMOD_VERSION)
	{
		printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return 0;
	}

	// ���ÿ� ����Ǵ� �����, 32ä���� ���� �⺻���Դϴ�.
	result = system->init(32, FMOD_INIT_NORMAL, 0);
	ERRCHECK(result);

	// ���ҽ� ��������
	result = system->createSound("drumloop.wav", FMOD_HARDWARE, 0, &sound1);
	ERRCHECK(result);

	// �⺻���� ������, 1ȸ����� Loop�� ���ξ�� �մϴ�.
	result = sound1->setMode(FMOD_LOOP_OFF); 
	ERRCHECK(result); 


	// ���ڵ��ؼ� �޸𸮿� �÷��Ӵϴ�. �׸���, ��ä�ο� ����մϴ�.
	// �׸���, ����ϴ� ä���� �����͸� �޽��ϴ�.
	result = system->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel);
	ERRCHECK(result);


	// ä���� ���ؼ� ���, �Ͻ�����, �������� �Ҽ� �ֽ��ϴ�.
	bool         playing = 0;
	bool         paused = 0;
	unsigned int ms = 0;
	unsigned int lenms = 0;
	FMOD::Sound *currentsound = 0;
	int          channelsplaying = 0;


	// ������ΰ�, �Ͻ����������ΰ�?, �����ġ, ä�ο� ����ϴ� ����
	result = channel->isPlaying(&playing);
	if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
	{
		ERRCHECK(result);
	}
	result = channel->getPaused(&paused);
	if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
	{
		ERRCHECK(result);
	}
	result = channel->getPosition(&ms, FMOD_TIMEUNIT_MS);
	if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
	{
		ERRCHECK(result);
	}


	// ä���� ���� ���� ����
	//channel->stop();


	// �ش� ä�ο� ������� ������ �����͸� ����ϴ�.
	// �̰��� ���ؼ� ������ ����϶�� �� ���尡 ����� �������� �˼� �ֽ��ϴ�.
	result = channel->getCurrentSound(&currentsound);
	if (currentsound)
	{
		result = currentsound->getLength(&lenms, FMOD_TIMEUNIT_MS);
		if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
		{
			ERRCHECK(result);
		}
	}


	// ���� ����Ǵ� ä�μ� 
	system->getChannelsPlaying(&channelsplaying);


	printf("ESCŰ ������ ����~\n");

	// �� �����Ӹ��� ������Ʈ�� �� �Ǿ�� �մϴ�.
	while(1)
	{
		system->update();
		result = channel->isPlaying(&playing);
		if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
		{
			ERRCHECK(result);
		}else if (playing==FALSE)
		{
			printf("%8d�� �ݺ�\r",++lCount);
			result = system->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel);
			ERRCHECK(result);
		}

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			break;
		}

		Sleep(10);
	}

	// �����, �������Ͽ� �޸𸮿��� �����մϴ�.
	result = sound1->release();
	ERRCHECK(result);
	result = system->close();
	ERRCHECK(result);
	result = system->release();
	ERRCHECK(result);

	return 0;
}
*/