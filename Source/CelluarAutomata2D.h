#pragma once

#include <windows.h>
#include <cstdint>
#include <vector>

typedef uint8_t     u8;
typedef int32_t     s32;
typedef uint32_t    u32;
typedef float       f32;

namespace Automaton
{
    struct Cell
    {
        bool alive = false;
    };

    // signed point coordinates are useful only for neighbour offsets, but inside grid, when converting from point coordinates to index or the other way around, the should be non-negative
    struct Point
    {
        s32 X;
        s32 Y;
    };

    namespace Grid
    {
        const u32 Width = 128;
        const u32 Height = Width;
        const auto Size = Width * Height;

        inline u32 CalcIndex(Point point)
        {
            return point.X + point.Y * Width;
        }

        inline Point CalcPoint(u32 Index)
        {
            s32 X = Index % Width;
            s32 Y = (Index - X) / Width;
            return Point{ X, Y };
        }

        Point Neighbours[] = { 
            Point{-1, -1},
            Point{ 0, -1},
            Point{ 1, -1},

            Point{-1,  0},
            // Cell itself
            Point{ 1,  0},

            Point{-1,  1},
            Point{ 0,  1},
            Point{ 1,  1}, 
        };

        std::vector<Cell> CurrentGenCells(Size);
        std::vector<Cell> NextGenCells(Size);
    };

};


namespace Bitmap
{
    const u32 SourceWidth = Automaton::Grid::Width;
    const u32 SourceHeight = Automaton::Grid::Height;

    const u32 PixelSize = 6;
    const u32 DestinationWidth = SourceWidth * PixelSize;
    const u32 DestinationHeight = SourceHeight * PixelSize;
    const u32 BytesPerPixel = 4;	

     BITMAPINFO Info;
     void* Memory;

    const u32 MemorySize = SourceWidth * SourceHeight * BytesPerPixel;
	const u32 Pitch = SourceWidth * BytesPerPixel;


    inline u32 Pixel_RGB(u8 R, u8 G, u8 B)
    {
        // B G R 0, so reversed when bit or-ing: 0 R G B
        return (0 << 24) | (R << 16) | (G << 8) | B;
    }
};

namespace WinApp
{
    void RenderSourceBitmap()
    {
        if (Bitmap::Memory)
            VirtualFree(Bitmap::Memory, 0, MEM_RELEASE);

        Bitmap::Info.bmiHeader.biSize = sizeof(Bitmap::Info.bmiHeader);
        Bitmap::Info.bmiHeader.biWidth = Bitmap::SourceWidth;
        // this should be negative height https://docs.microsoft.com/en-us/previous-versions/dd183376(v=vs.85) to make "The rows are written out from top to bottom" true, but whatever!
        Bitmap::Info.bmiHeader.biHeight = Bitmap::SourceHeight; 
        Bitmap::Info.bmiHeader.biPlanes = 1;
        Bitmap::Info.bmiHeader.biBitCount = 32;
        Bitmap::Info.bmiHeader.biCompression = BI_RGB;
        
        Bitmap::Memory = VirtualAlloc(0, Bitmap::MemorySize, MEM_COMMIT, PAGE_READWRITE);
        
        u8 *Row = (u8 *)Bitmap::Memory;
        for (s32 Y = 0; Y < Bitmap::SourceHeight; Y++)
        {
            u32 *Pixel = (u32 *)Row;
            for (s32 X = 0; X < Bitmap::SourceWidth; X++)
            {
                u32 RGB_Value;

                if (X % 2)
                {
                    if (Y % 2)
                        RGB_Value = Bitmap::Pixel_RGB(0, 255, 255);
                    else
                        RGB_Value = Bitmap::Pixel_RGB(128, 255, 0);
                }
                else
                {
                    if (Y % 2)
                        RGB_Value = Bitmap::Pixel_RGB(255, 0, 255);
                    else
                        RGB_Value = Bitmap::Pixel_RGB(255, 255, 0);
                }

                *Pixel++ = RGB_Value;
            }

            Row += Bitmap::Pitch;
        }
    }

    void OutputDestinationBitmapToWindow(HDC DeviceContext)
    {
        StretchDIBits(
            DeviceContext,
            0, 0, Bitmap::DestinationWidth, Bitmap::DestinationHeight, // Destination
            0, 0, Bitmap::SourceWidth, Bitmap::SourceHeight, // Source
            Bitmap::Memory,
            &Bitmap::Info,
            DIB_RGB_COLORS,
            SRCCOPY
        );
    }

    LRESULT CALLBACK MainWindowCallback(HWND Window, u32 Message, WPARAM WParam, LPARAM LParam)
    {
        LRESULT Result = 0;
        switch (Message)
        {
        
            case WM_CLOSE:
            {
                OutputDebugStringA("WM_CLOSE\n");
                DestroyWindow(Window);
            } break;

            case WM_DESTROY:
            {
                OutputDebugStringA("WM_DESTROY\n");
                PostQuitMessage(0);
                Result = 0;
            } break;

            case WM_PAINT:
            {
                RenderSourceBitmap();
                PAINTSTRUCT Pas32;
                HDC DeviceContext = BeginPaint(Window, &Pas32);

                RECT ClientRect;
                GetClientRect(Window, &ClientRect);

                OutputDestinationBitmapToWindow(DeviceContext);
                EndPaint(Window, &Pas32);
            } break;

            default:
            {
                OutputDebugStringA("default\n");
                Result = DefWindowProc(Window, Message, WParam, LParam);
            }
        }

        return Result;
    }

    void WINAPI Start()
    {
        WNDCLASS WindowClass = {};
        
        WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        WindowClass.lpfnWndProc = MainWindowCallback;
        WindowClass.hInstance = GetModuleHandle(NULL);
        WindowClass.lpszClassName = "Win32Window";

        if (RegisterClass(&WindowClass))
        {
            // Not the window itself, but the area to pas32 to inside window.
            RECT WindowRect;
            WindowRect.top = 0;
            WindowRect.left = 0;
            WindowRect.bottom = Bitmap::DestinationHeight;
            WindowRect.right = Bitmap::DestinationWidth;

            DWORD WindowStyle = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE;

            // convert area to pas32 s32o actual window dimensions
            if (!AdjustWindowRect(&WindowRect, WindowStyle, false))
            {
                OutputDebugStringA("AdjustWindowRect Error!\n");
                PostQuitMessage(WM_QUIT);
            }

            u32 WindowWidth = WindowRect.right - WindowRect.left;
            u32 WindowHeight = WindowRect.bottom - WindowRect.top;

            HWND WindowHandle = CreateWindowExA(
                0,
                WindowClass.lpszClassName,
                "2D Cellular Automata",
                WindowStyle,
                32,		        // origin X
                32,		        // origin Y
                WindowWidth,	// width
                WindowHeight,	// height
                0,
                0,
                WindowClass.hInstance,
                0
            );

            if (WindowHandle)
            {
                bool Running = true;
                while (Running)
                {
                    MSG Message;
                    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                    {
                        if (Message.message == WM_QUIT)
                            Running = false;

                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    }
                }
            }
            else { OutputDebugStringA("WindowHandle Error!\n"); }
                
        }
        else { OutputDebugStringA("RegisterClass Error!\n"); }
    }
}