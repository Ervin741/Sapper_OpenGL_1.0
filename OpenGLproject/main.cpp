#include <windows.h>
#include <gl/gl.h>
#include <tchar.h>
#include <time.h>

#pragma comment(lib, "opengl32.lib")

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

#define mapW 10
#define mapH 10

typedef struct {
	BOOL mine;
	BOOL flag;
	BOOL open;
	int cntAround;
} TCell;

TCell map[mapW][mapH];
int mines;
int closedCell;
BOOL failed;

void Line(float x1, float y1, float x2, float y2) 
{
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

BOOL IsCellInMap(int x, int y) {
	return (x >= 0) && (y >= 0) && (x < mapW) && (y < mapH);
}

void ScreeToOpenGL(HWND hwnd, int x, int y, float* ox, float* oy) {
	RECT rct;
	GetClientRect(hwnd, &rct);
	*ox = x / (float)rct.right * mapW;
	*oy = mapH - y / (float)rct.bottom * mapH;

}

void ShowCount(int a) {

	glLineWidth(3);
		glColor3f(1, 1, 0);
		if ((a != 1) && (a != 4))
			Line(0.3, 0.85, 0.7, 0.85);
		if ((a != 0) && (a != 1) && (a != 7))
			Line(0.3, 0.5, 0.7, 0.5);
		if ((a != 1) && (a != 4) && (a != 7))
			Line(0.3, 0.15, 0.7, 0.15);

		if ((a != 5) && (a != 6))
			Line(0.7, 0.5, 0.7, 0.85);
		if ((a != 2))
			Line(0.7, 0.5, 0.7, 0.15);
	
		if ((a != 1) && (a != 2) && (a != 3) && (a != 7))
			Line(0.3, 0.5, 0.3, 0.85);
		if ((a == 0) || (a == 2) || (a == 6) || (a == 8))
			Line(0.3, 0.5, 0.3, 0.15);
}

void Game_New(){
	srand(time(NULL));
	memset(map, 0, sizeof(map));

	mines = 20;
	closedCell = mapW * mapH;
	failed = false;
	for (int i = 0; i < mines; i++) {
		int x = rand() % mapW;
		int y = rand() % mapH;
		if (map[x][y].mine) i--;
		else {
			map[x][y].mine = TRUE;

			for (int dx = -1; dx < 2; dx++)
				for (int dy = -1; dy < 2; dy++)
					if (IsCellInMap(x + dx, y + dy))
						map[x + dx][y + dy].cntAround += 1;
			
		}
	}

}

void ShowMine() {
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(0, 0, 0);
		glVertex2f(0.3, 0.3);
		glVertex2f(0.7, 0.3);
		glVertex2f(0.7, 0.7);
		glVertex2f(0.3, 0.7);
	glEnd();
}

void ShowField() {
	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.8f, 0.8f, 0.8f); glVertex2f(0.0f, 1.0f);
		glColor3f(0.7f, 0.7f, 0.7f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f); glVertex2f(1.0f, 0.0f);	
	glEnd();
}

void ShowFieldOpen() {
	glBegin(GL_TRIANGLE_STRIP);
	glColor3f(0.3f, 0.7f, 0.3f); glVertex2f(0.0f, 1.0f);
	glColor3f(0.3f, 0.6f, 0.3f); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 0.0f);
	glColor3f(0.3f, 0.5f, 0.3f); glVertex2f(1.0f, 0.0f);
	glEnd();
}

void ShowFlag() {
	glBegin(GL_TRIANGLES);
		glColor3f(1, 0, 0);
		glVertex2f(0.25f, 0.75f);
		glVertex2f(0.85f, 0.5f);
		glVertex2f(0.25f, 0.25f);
	glEnd();

	glLineWidth(5);

	glBegin(GL_LINES);
		glColor3f(0, 0, 0);
		glVertex2f(0.25, 0.75);
		glVertex2f(0.25, 0);
	glEnd();
}

void OpenFields(int x, int y) {
	if (!IsCellInMap(x, y) || map[x][y].open) return;
	map[x][y].open = TRUE;
	closedCell--; 
	if (map[x][y].cntAround == 0)
		for (int dx = -1; dx < 2; dx++)
			for (int dy = -1; dy < 2; dy++)
				OpenFields(x + dx, y + dy);
	if (map[x][y].mine) {
		failed = TRUE;
		for (int j = 0; j < mapH; j++)
			for (int i = 0; i < mapW; i++)
				map[i][j].open = TRUE;
	}
}

void GameShow() {
	glLoadIdentity();
	glScalef(2.0 / mapW, 2.0 / mapH, 1);
	glTranslatef(-mapW * 0.5, -mapH * 0.5, 0);
	
	for (int j = 0; j < mapH; j++)
		for (int i = 0; i < mapW; i++){
		{
			glPushMatrix();
			glTranslatef(i, j, 0);
			if (map[i][j].open) 
			{
				ShowFieldOpen();
				if (map[i][j].mine)
					ShowMine();
				else if (map[i][j].cntAround > 0) 
					ShowCount(map[i][j].cntAround);
			}
			else {
				ShowField();
				if (map[i][j].flag) ShowFlag();
			}
			glPopMatrix();
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;
	float theta = 0.0f;

	/* register window class */
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "GLSample";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


	if (!RegisterClassEx(&wcex))
		return 0;


	/* create main window */
	hwnd = CreateWindowEx(0,
		"GLSample",
		"OpenGL Sample",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		600,
		600,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

	Game_New();

	/* program main loop */
	while (!bQuit)
	{
		/* check for messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			/* handle or dispatch messages */
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			/* OpenGL animation code goes here */

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			
			if (mines == closedCell)
				Game_New();
			
			GameShow();

			SwapBuffers(hDC);

			Sleep(1);
		}
	}

	/* shutdown OpenGL */
	DisableOpenGL(hwnd, hDC, hRC);

	/* destroy the window explicitly */
	DestroyWindow(hwnd);

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
	{
		POINTFLOAT pf;
		ScreeToOpenGL(hwnd, LOWORD(lParam), HIWORD(lParam), &pf.x, &pf.y);
		int x = (int)pf.x;
		int y = (int)pf.y;
		if (IsCellInMap(x, y) && !map[x][y].flag)
			OpenFields(x, y);
			/*map[x][y].open = TRUE;*/
	}break;

	case WM_RBUTTONDOWN:
	{
		POINTFLOAT pf;
		ScreeToOpenGL(hwnd, LOWORD(lParam), HIWORD(lParam), &pf.x, &pf.y);
		int x = (int)pf.x;
		int y = (int)pf.y;
		if (IsCellInMap(x, y))
			map[x][y].flag = !map[x][y].flag;
	}break;

	case WM_DESTROY:
		return 0;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}
	break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*hDC, &pfd);

	SetPixelFormat(*hDC, iFormat, &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}

