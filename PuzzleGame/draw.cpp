#include "stdafx.h"

#include "draw.h"

#include "auto.h"
#include "game.h"
#include "window.h"
#include "resource.h"

float DPIScale::scale = 1.0f;

ID2D1Factory			*g_pID2D1Factory;
ID2D1HwndRenderTarget	*g_pRenderTarget;
ID2D1SolidColorBrush	*g_pBrush;
ID2D1Bitmap             *g_pBitmap;
IWICImagingFactory      *g_pIWICFactory;

// 棋盘距边缘距离/min(窗口长, 窗口宽)
constexpr float BORDER_GAP = 0.1f;
// 绘制矩形长宽比
constexpr float RECT_SCALE = 16.0f / 9.0f;
// 按钮垂直间隔/按钮高度
constexpr float BUTTON_GAP = 1.0f / 4.0f;
// 绘制矩形
RECT g_paintRect;
// 棋盘左上角坐标（DIP）
D2D1_POINT_2F g_boardLTPoint;
// 单格边长（DIP）
float g_pieceWidth, g_pieceHeight;
// 按钮长宽
int g_buttonWidth, g_buttonHeight;
// 位图左上角坐标
D2D1_POINT_2F g_bmpLTPoint;
// 位图较短边长度
float g_bmpSideLength;
// 是否为预览模式
bool g_isPreview = false;

void CalculateLayout()
{
	if (g_pRenderTarget != NULL)
	{
		// 获取客户区大小
		GetClientRect(g_hWnd, &g_paintRect);

		// 根据长宽比裁剪大小
		int width = static_cast<int>(g_paintRect.bottom * RECT_SCALE);
		int height;

		if (width < g_paintRect.right)
		{
			g_paintRect.left = (g_paintRect.right - width) / 2;
			g_paintRect.right = (g_paintRect.right + width) / 2;
			height = g_paintRect.bottom;
		}
		else
		{
			width = g_paintRect.right;
			height = static_cast<int>(g_paintRect.right / RECT_SCALE);
			g_paintRect.top = (g_paintRect.bottom - height) / 2;
			g_paintRect.bottom = (g_paintRect.bottom + height) / 2;
		}

		// 切掉边缘留白
		const int horizontalGap = static_cast<int>(width * BORDER_GAP);
		const int verticalGap = static_cast<int>(height * BORDER_GAP);

		g_paintRect.left += horizontalGap;
		g_paintRect.top += verticalGap;
		g_paintRect.right -= horizontalGap;
		g_paintRect.bottom -= verticalGap;
		width = g_paintRect.right - g_paintRect.left;
		height = g_paintRect.bottom - g_paintRect.top;

		// 计算棋盘位置
		g_boardLTPoint = DPIScale::PixelsToDips(g_paintRect.right - height, g_paintRect.top);
		g_pieceWidth = DPIScale::PixelsToDips(height) / g_boardSize;
		g_pieceHeight = DPIScale::PixelsToDips(height) / g_boardSize;

		// 计算按钮位置
		g_buttonWidth = width - height - horizontalGap;
		g_buttonHeight = static_cast<int>(height / (4 + (4 - 1) * BUTTON_GAP));
	}
}

HRESULT CreateGraphicsResources()
{
	HRESULT hr = S_OK;
	if (g_pRenderTarget == NULL)
	{
		RECT rc;
		GetClientRect(g_hWnd, &rc);

		hr = g_pID2D1Factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				g_hWnd,
				D2D1::SizeU(rc.right, rc.bottom)
			),
			&g_pRenderTarget
		);

		if (SUCCEEDED(hr))
		{
			hr = g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), &g_pBrush);

			if (SUCCEEDED(hr))
			{
				hr = LoadResourceBitmap();

				if (SUCCEEDED(hr))
				{
					CalculateLayout();
				}
			}
		}
	}
	return hr;
}

HRESULT CreateMyBitmapFromWicBitmap(IWICBitmapSource *pIWICBitmapSource)
{

	ID2D1Bitmap *tpBitmap;
	// Create a Direct2D bitmap from the WIC bitmap.
	HRESULT hr = g_pRenderTarget->CreateBitmapFromWicBitmap(
		pIWICBitmapSource,
		NULL,
		&tpBitmap
	);

	if (SUCCEEDED(hr))
	{
		SafeRelease(g_pBitmap);
		g_pBitmap = tpBitmap;

		D2D1_SIZE_F size = g_pBitmap->GetSize();

		if (size.width < size.height)
		{
			g_bmpLTPoint.x = 0.0f;
			g_bmpLTPoint.y = (size.height - size.width) / 2;
			g_bmpSideLength = size.width;
		}
		else
		{
			g_bmpLTPoint.y = 0.0f;
			g_bmpLTPoint.x = (size.width - size.height) / 2;
			g_bmpSideLength = size.height;
		}

		InvalidateRect(g_hWnd, NULL, FALSE);
	}

	return hr;
}

void DiscardGraphicsResources()
{
	SafeRelease(g_pRenderTarget);
	SafeRelease(g_pBrush);
	SafeRelease(g_pBitmap);
}

HRESULT LoadResourceBitmap()
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void *pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Locate the resource.
	imageResHandle = FindResource(NULL, MAKEINTRESOURCE(IDR_IMAGE1), _T("Image"));
	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		imageResDataHandle = LoadResource(NULL, imageResHandle);

		hr = imageResDataHandle ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImageFile = LockResource(imageResDataHandle);

		hr = pImageFile ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		imageFileSize = SizeofResource(NULL, imageResHandle);

		hr = imageFileSize ? S_OK : E_FAIL;

	}
	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory.
		hr = g_pIWICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(
			reinterpret_cast<BYTE*>(pImageFile),
			imageFileSize
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = g_pIWICFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = g_pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr))
	{
		//create a Direct2D bitmap from the WIC bitmap.
		hr = CreateMyBitmapFromWicBitmap(pConverter);
	}

	SafeRelease(pDecoder);
	SafeRelease(pSource);
	SafeRelease(pStream);
	SafeRelease(pConverter);

	return hr;
}

void OnLButtonDown(int pixelX, int pixelY, DWORD flags)
{
	const int boardLeft = g_paintRect.right - (g_paintRect.bottom - g_paintRect.top);
	const int boardTop = g_paintRect.top;
	const int pieceWidth = (g_paintRect.right - boardLeft) / g_boardSize;
	const int pieceHeight = (g_paintRect.bottom - boardTop) / g_boardSize;
	int posX = (pixelX - boardLeft) < 0 ? -1 : (pixelX - boardLeft) / pieceWidth;
	int posY = (pixelY - boardTop) < 0 ? -1 : (pixelY - boardTop) / pieceHeight;
	PosInfo pos = { posY, posX };
	bool res;
	if (g_boardSize == 3) res = g_board3.click(pos);
	else if (g_boardSize == 4) res = g_board4.click(pos);
	else res = g_board5.click(pos);
	if (res) InvalidateRect(g_hWnd, NULL, FALSE);
}

void OnLButtonUp(int pixelX, int pixelY, DWORD flags)
{
	
}

void OnMove(MoveInfo mov)
{
	bool res;
	if (g_boardSize == 3) res = g_board3.move(mov);
	else if (g_boardSize == 4) res = g_board4.move(mov);
	else res = g_board5.move(mov);
	if (res)
	{
		InvalidateRect(g_hWnd, NULL, FALSE);
	}
}

void OnPaint()
{
	HRESULT hr = CreateGraphicsResources();
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(g_hWnd, &ps);

		g_pRenderTarget->BeginDraw();

		g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
		PaintBoard();

		hr = g_pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}

		PaintButton();

		EndPaint(g_hWnd, &ps);
	}
}

void PaintBoard()
{
	const float boardBottom = g_boardLTPoint.y + g_pieceHeight * g_boardSize;
	const float boardRight = g_boardLTPoint.x + g_pieceWidth * g_boardSize;

	if (g_isPreview)
	{
		g_pRenderTarget->DrawBitmap(
			g_pBitmap,
			D2D1::RectF(
				g_boardLTPoint.x,
				g_boardLTPoint.y,
				boardRight,
				boardBottom
			),
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			D2D1::RectF(
				g_bmpLTPoint.x,
				g_bmpLTPoint.y,
				g_bmpLTPoint.x + g_bmpSideLength,
				g_bmpLTPoint.y + g_bmpSideLength
			)
		);

		g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		g_pRenderTarget->DrawRectangle(
			D2D1::RectF(
				g_boardLTPoint.x,
				g_boardLTPoint.y,
				boardRight,
				boardBottom
			),
			g_pBrush
		);
	}
	else
	{
		float bmpPieceSideLength = g_bmpSideLength / g_boardSize;
		PosInfo pos;
		float tmp;

		for (int row = 0; row < g_boardSize; ++row)
		{
			for (int col = 0; col < g_boardSize; ++col)
			{
				if (g_boardSize == 3) pos = g_board3.getPiecePos({ row, col });
				else if (g_boardSize == 4) pos = g_board4.getPiecePos({ row, col });
				else pos = g_board5.getPiecePos({ row, col });
				if (pos.row != g_boardSize - 1 || pos.col != g_boardSize - 1)
				{
					g_pRenderTarget->DrawBitmap(
						g_pBitmap,
						D2D1::RectF(
							g_boardLTPoint.x + col * g_pieceWidth,
							g_boardLTPoint.y + row * g_pieceHeight,
							g_boardLTPoint.x + (col + 1) * g_pieceWidth,
							g_boardLTPoint.y + (row + 1) * g_pieceHeight
						),
						1.0f,
						D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
						D2D1::RectF(
							g_bmpLTPoint.x + pos.col * bmpPieceSideLength,
							g_bmpLTPoint.y + pos.row * bmpPieceSideLength,
							g_bmpLTPoint.x + (pos.col + 1) * bmpPieceSideLength,
							g_bmpLTPoint.y + (pos.row + 1) * bmpPieceSideLength
						)
					);
				}
			}
		}

		g_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

		// 绘制横线
		for (int i = 0; i <= g_boardSize; ++i)
		{
			tmp = g_boardLTPoint.y + i * g_pieceHeight;

			g_pRenderTarget->DrawLine(
				{ g_boardLTPoint.x, tmp },
				{ boardRight, tmp },
				g_pBrush
			);
		}
		// 绘制竖线
		for (int i = 0; i <= g_boardSize; ++i)
		{
			tmp = g_boardLTPoint.x + i * g_pieceWidth;

			g_pRenderTarget->DrawLine(
				{ tmp, g_boardLTPoint.y },
				{ tmp, boardBottom },
				g_pBrush
			);
		}
	}
}

void PaintButton()
{
	int cury = g_paintRect.top;
	int dy = static_cast<int>(g_buttonHeight * (1 + BUTTON_GAP));
	bool isSolved;

	MoveWindow(g_hBtnDifficulty, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
	InvalidateRect(g_hBtnDifficulty, NULL, FALSE);

	cury += dy;
	if (g_boardSize == 3) isSolved = g_board3.isFinished();
	else if (g_boardSize == 4) isSolved = g_board4.isFinished();
	else isSolved = g_board5.isFinished();
	if (isSolved)
	{
		ShowWindow(g_hBtnRandom, SW_SHOW);
		ShowWindow(g_hBtnSolve, SW_HIDE);
		MoveWindow(g_hBtnRandom, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
		InvalidateRect(g_hBtnRandom, NULL, FALSE);
	}
	else
	{
		ShowWindow(g_hBtnSolve, SW_SHOW);
		ShowWindow(g_hBtnRandom, SW_HIDE);
		MoveWindow(g_hBtnSolve, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
		InvalidateRect(g_hBtnSolve, NULL, FALSE);
	}

	cury += dy;
	if (g_threadRunning)
	{
		ShowWindow(g_hBtnStop, SW_SHOW);
		ShowWindow(g_hBtnAuto, SW_HIDE);
		MoveWindow(g_hBtnStop, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
		InvalidateRect(g_hBtnStop, NULL, FALSE);
	}
	else
	{
		ShowWindow(g_hBtnAuto, SW_SHOW);
		ShowWindow(g_hBtnStop, SW_HIDE);
		MoveWindow(g_hBtnAuto, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
		InvalidateRect(g_hBtnAuto, NULL, FALSE);
	}

	cury += dy;
	MoveWindow(g_hBtnImage, g_paintRect.left, cury, g_buttonWidth, g_buttonHeight, FALSE);
	InvalidateRect(g_hBtnImage, NULL, FALSE);

	// UNDONE: 添加其余按钮
}

void Resize()
{
	if (g_pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(g_hWnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		g_pRenderTarget->Resize(size);
		CalculateLayout();
		InvalidateRect(g_hWnd, NULL, FALSE);
	}
}

HRESULT SetImageFile(PCTSTR fileName)
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICFormatConverter *pConverter = NULL;

	hr = g_pIWICFactory->CreateDecoderFromFilename(
		fileName,
		NULL,
		GENERIC_READ, 
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = g_pIWICFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}

	if (SUCCEEDED(hr))
	{
		hr = CreateMyBitmapFromWicBitmap(pConverter);
	}

	SafeRelease(pDecoder);
	SafeRelease(pSource);
	SafeRelease(pConverter);

	if (FAILED(hr))
	{
		MessageBox(g_hWnd, _T("无法读取该文件。\n无效的位图文件或不支持文件的格式。"), NULL, MB_OK | MB_ICONWARNING);
	}

	return hr;
}