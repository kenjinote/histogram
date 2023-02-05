#define UNICODE
#pragma comment(linker,"/opt:nowin98")
#pragma comment(lib,"gdiplus.lib")
#include<windows.h>
#include<gdiplus.h>

using namespace Gdiplus;

TCHAR szClassName[]=TEXT("Window");
extern "C" int _fltused = 0x9875;

HBITMAP LoadPictureAsBitmap(LPCTSTR pszFileName)
{
    Color c;
    c.SetFromCOLORREF(RGB(0,0,0));
    Bitmap b(pszFileName);
    HBITMAP hbm=0;
    b.GetHBITMAP(c, &hbm);
    return hbm;
}

UINT GetHistogram(LPCTSTR szFileName,UINT* pHistogram)
{
	BITMAP  bm;
	HBITMAP hBitmap=LoadPictureAsBitmap(szFileName);
	if( hBitmap == NULL ) return 0;
	GetObject(hBitmap, sizeof(BITMAP), &bm );
	HDC hMemDC1 = CreateCompatibleDC( NULL );
	HBITMAP hOldBitmap = (HBITMAP)SelectObject( hMemDC1, hBitmap );
	UINT nMax = 0;
	for(int y=0;y<bm.bmWidth;y++)
	{
		for(int x=0;x<bm.bmWidth;x++)
		{
			const COLORREF color = GetPixel(hMemDC1,x,y);
			const double r = GetRValue(color);
			const double g = GetGValue(color);
			const double b = GetBValue(color);
			const int nIndex = (int)( 0.5 + 0.298912 * r + 0.586611 * g + 0.114478 * b );
			pHistogram[nIndex]++;
		}
	}
	for(int i=0;i<256;i++)
	{
		if(nMax<pHistogram[i])nMax=pHistogram[i];
	}
	SelectObject( hMemDC1, hOldBitmap );
	DeleteDC( hMemDC1 );
	DeleteObject( hBitmap );
	return nMax;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static UINT uHistogram[256];
	static UINT nMax;
	switch(msg)
	{
	case WM_CREATE:
		DragAcceptFiles(hWnd,TRUE);
		break;
	case WM_DROPFILES:
		{
			TCHAR szFileName[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			UINT uFileNo = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			for(UINT i = 0; i < (int)uFileNo; i++)
			{
				ZeroMemory(uHistogram,sizeof(UINT)*256);
				DragQueryFile(hDrop, i, szFileName, sizeof(szFileName));
				nMax=GetHistogram(szFileName,uHistogram);
			}
			DragFinish(hDrop);
			InvalidateRect(hWnd,0,1);
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc=BeginPaint(hWnd,&ps);
			if(nMax)
			{
				HPEN hPen=CreatePen(PS_SOLID,1,RGB(192,192,192));
				HPEN hOldPen=(HPEN)SelectObject(hdc,hPen);
				RECT rect;
				RECT rectFill;
				GetClientRect(hWnd,&rect);
				for(int i=0;i<256;i++)
				{
					SetRect(&rectFill,
						(int)((double)i*((double)rect.right/(double)256)),
						(int)((double)rect.bottom*(1.0-(double)uHistogram[i]/(double)nMax)),
						(int)((double)(i+1)*((double)rect.right/(double)256))-1,
						rect.bottom);
					FillRect(hdc,&rectFill,(HBRUSH)GetStockObject(GRAY_BRUSH));
				}
				SelectObject(hdc,hOldPen);
				DeleteObject(hPen);
			}
			EndPaint(hWnd,&ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return 0;
}

EXTERN_C void __cdecl WinMainCRTStartup()
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	MSG msg;
	HINSTANCE hInstance=GetModuleHandle(0);
	WNDCLASS wndclass={
		CS_HREDRAW|CS_VREDRAW,
			WndProc,
			0,
			0,
			hInstance,
			0,
			LoadCursor(0,IDC_ARROW),
			(HBRUSH)(COLOR_WINDOW+1),
			0,
			szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd=CreateWindow(
		szClassName,
		TEXT("ƒqƒXƒgƒOƒ‰ƒ€"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    GdiplusShutdown(gdiplusToken);
	ExitProcess(msg.wParam);
}

#if _DEBUG
void main(){}
#endif
