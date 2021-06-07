#pragma once

#include <windows.h>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <vector>
#include <random>
#include <iostream>

typedef uint8_t     u8;
typedef int32_t     s32;
typedef uint32_t    u32;
typedef float       f32;
typedef double      f64;

namespace Grid
{
    struct Point
    {
        s32 X;
        s32 Y;
    };

    const u32 Width = 128*4;
    const u32 Height = Width/2;
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
};


namespace Bitmap
{
    const u32 SourceWidth = Grid::Width;
    const u32 SourceHeight = Grid::Height;

    const u32 PixelSize = 3;
    const u32 DestinationWidth = SourceWidth * PixelSize;
    const u32 DestinationHeight = SourceHeight * PixelSize;
    const u32 BytesPerPixel = 4;	

     BITMAPINFO Info;
    static void* Memory;

    const u32 MemorySize = SourceWidth * SourceHeight * BytesPerPixel;
	const u32 Pitch = SourceWidth * BytesPerPixel;


    inline u32 Pixel_RGB(u8 R, u8 G, u8 B)
    {
        // B G R 0, so reversed when bit or-ing: 0 R G B
        return (0 << 24) | (R << 16) | (G << 8) | B;
    }
};

namespace Dice
{
    // Setup d6 random die
    std::random_device rand_dev;
    std::default_random_engine rand_eng(rand_dev());
    const u32 sides = 3;
    std::uniform_int_distribution<int> uniform_dist(1, sides);

    inline u32 Roll()
    {
        return uniform_dist(rand_eng);
    }
};

namespace Timing
{
    inline auto TimeIt()
    {
        return std::chrono::high_resolution_clock::now();
    }

    const auto TargetFrameTime = 1.0 / 12;
    auto Accumulator = 0.0;
    auto StartTime = TimeIt();
    auto EndTime = TimeIt();

    inline auto ElapsedTime()
    {
        return std::chrono::duration_cast<std::chrono::duration<f64, std::ratio<1>>>(EndTime - StartTime).count();
    }
};

namespace Cells
{
    struct Cell 
    {
        bool alive = false;
    };

    static std::vector<Cell> CurrentGeneration(Grid::Size);
    static std::vector<Cell> NextGeneration(Grid::Size);

    const Grid::Point Neighbours[] = { 
        Grid::Point{-1, -1},
        Grid::Point{ 0, -1},
        Grid::Point{ 1, -1},

        Grid::Point{-1,  0},
        // Cell itself
        Grid::Point{ 1,  0},

        Grid::Point{-1,  1},
        Grid::Point{ 0,  1},
        Grid::Point{ 1,  1}, 
    };

    std::vector<u32> SurviveRules;
    std::vector<u32> BornRules; 

    const auto dead_pixel = Bitmap::Pixel_RGB(255, 255, 255);
    const auto alive_pixel = Bitmap::Pixel_RGB(0, 0, 0);

    inline u32 DetermineStateAndOutputPixel(Grid::Point cell)
    {
        // Count cell's neighbours
        auto cell_index = Grid::CalcIndex(cell);
        u32 neighbour_count = 0;
        for (auto& neighbour : Neighbours)
        {
            Grid::Point target{ cell.X + neighbour.X, cell.Y + neighbour.Y };
            if ( (target.X < 0 || target.X > (Grid::Width-1)) || (target.Y < 0 || target.Y > (Grid::Height-1)) ) 
                continue;
            if (CurrentGeneration[Grid::CalcIndex(target)].alive) 
                neighbour_count += 1;
        }
        
        // Determine cell's state based on a neighbour count
        bool state = false;
        if (CurrentGeneration[cell_index].alive)
        {
            for (auto& rule : SurviveRules)
                if (neighbour_count == rule) { state = true; }

            NextGeneration[cell_index].alive = state;

            return alive_pixel;
        }
        else 
        {
            for (auto& rule : BornRules)
                if (neighbour_count == rule) { state = true; }

            NextGeneration[cell_index].alive = state;

            return dead_pixel;
        }
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
        // this should be negative height https://docs.microsoft.com/en-us/previous-versions/dd183376(v=vs.85) since: "The rows are written out from top to bottom"
        Bitmap::Info.bmiHeader.biHeight = -Bitmap::SourceHeight; 
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
                *Pixel++ = Cells::DetermineStateAndOutputPixel(Grid::Point{X, Y});
            }

            Row += Bitmap::Pitch;
        }

        Cells::CurrentGeneration.swap(Cells::NextGeneration);
    }

    inline void OutputDestinationBitmapToWindow(HDC DeviceContext)
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

        //std::cout << ret_val << std::endl;
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
                OutputDebugStringA("WM_PAINT\n");
                PAINTSTRUCT Paint;
                HDC DeviceContext = BeginPaint(Window, &Paint);

                RECT ClientRect;
                GetClientRect(Window, &ClientRect);

                OutputDestinationBitmapToWindow(DeviceContext);
                EndPaint(Window, &Paint);
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
        // CA: fill cells randomly
        for (auto& cell : Cells::CurrentGeneration)
        {
            if (Dice::Roll() == Dice::sides)
                cell.alive = true;
            else
                cell.alive = false;
        }

        WNDCLASS WindowClass = {};
        
        WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        WindowClass.lpfnWndProc = MainWindowCallback;
        WindowClass.hInstance = GetModuleHandle(NULL);
        WindowClass.lpszClassName = "Win32Window";

        if (RegisterClass(&WindowClass))
        {
            // Not the window itself, but the area to paint to inside window.
            RECT WindowRect;
            WindowRect.top = 0;
            WindowRect.left = 0;
            WindowRect.bottom = Bitmap::DestinationHeight;
            WindowRect.right = Bitmap::DestinationWidth;

            DWORD WindowStyle = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE;

            // convert area to paint into actual window dimensions
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

                    HDC DeviceContext = GetDC(WindowHandle);

                    Timing::Accumulator += Timing::ElapsedTime();
                    Timing::StartTime = Timing::EndTime;
                    Timing::EndTime = Timing::TimeIt();

                    if (Timing::Accumulator > Timing::TargetFrameTime)
                    {
                        Timing::Accumulator = 0.0;
                        RenderSourceBitmap();
                        OutputDestinationBitmapToWindow(DeviceContext);
                    }

                    ReleaseDC(WindowHandle, DeviceContext);
                }
            }
            else { OutputDebugStringA("WindowHandle Error!\n"); }
                
        }
        else { OutputDebugStringA("RegisterClass Error!\n"); }
    }
}