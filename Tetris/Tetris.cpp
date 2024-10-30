#include <vector>
#include <random>
#include <thread>
#include <windows.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>
using namespace std;

std::chrono::milliseconds operator""ms(unsigned long long milliseconds)
{
    return std::chrono::milliseconds(milliseconds); 
}

const int width = 12;
const int height = 18;

const int nScreenWidth = 120; // Console Screen Size X
const int nScreenHeight = 30; // Console Screen Size Y

unsigned char *pField = nullptr;

wstring tetromino[7];

void build_tetromino()
{
    // T1
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    // T2
    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    // T3
    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    // T4
    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    // T5
    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    // T6
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"....");

    // T7
    tetromino[6].append(L"....");
    tetromino[6].append(L"..XX");
    tetromino[6].append(L"..X.");
    tetromino[6].append(L"..X.");
}

int Rotate(int px,int py,int r)
{
    switch (r % 4)
    {
        case 0 : return py * 4 + px; // 0degree
        case 1 : return 12 + py - (px * 4); // 90degree
        case 2 : return 15 - (py * 4) - px; // 180degree
        case 3 : return 3 - py + (px * 4); //270degree
    }
    return 0;
}

bool Check_Fit(int nTetromino,int nRotation,int nPosx,int nPosy)
{
    for(int px = 0;px < 4; ++ px)
        for(int py = 0;py < 4; ++ py) // check all the element within this tetromino
        {
            int pi = Rotate(px,py,nRotation); // get index into piece
            int fi = (nPosy + py) * width + (nPosx + px); // get index into field

            if(nPosx + px >= 0 && nPosx + px < width && nPosy + py >= 0 && nPosy + py < height) // within the boundary
                if(tetromino[nTetromino][pi] == L'X' && pField[fi] != 0) return false;
        }

    return true;
}

// Game Logic Stuff

bool gameover = false;

int nCurrentPiece = 1;
int nCurrentRotation = 0;
int nCurrentX = width >> 1,nCurrentY = 0;

// key pressed
bool Key[4];
bool Rotate_Hold = false;

// game speed
int nSpeed = 20;
int nSpeedCounter = 0;
bool ForceDown = false;
int nPiececount = 0;

// remove line
vector<int> vLine;

// scores
int nScore = 0;

int main()
{
    build_tetromino();

    pField = new unsigned char[width * height]; // create game field buffer
    for(int x = 0;x < width; ++ x) // boarder
        for(int y = 0;y < height; ++ y)
            pField[y * width + x] = (x == 0 || x == width - 1 || y == height - 1) ? 9 : 0; // 9 represent the boarder
        
    wchar_t *screen = new wchar_t[nScreenHeight * nScreenWidth];
    for(int i = 0;i < nScreenHeight * nScreenWidth; ++ i) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,0,NULL,CONSOLE_TEXTMODE_BUFFER,NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    while(gameover != true)
    {
        // GAME TIMING
        //cout<< dwBytesWritten <<endl;
        this_thread :: sleep_for(50ms); // game tick
        ++nSpeedCounter, ForceDown = (nSpeedCounter == nSpeed);

        // INPUT
        for (int k = 0; k < 4; k++)   
            Key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0; // \x27 \x25 \x28 right left down

        // GAME LOGIC
        if(Key[0] && Check_Fit(nCurrentPiece,nCurrentRotation,nCurrentX + 1,nCurrentY)) // check ->R
                ++nCurrentX;

        if(Key[1] && Check_Fit(nCurrentPiece,nCurrentRotation,nCurrentX - 1,nCurrentY)) // check ->L
                --nCurrentX;
        
        if(Key[2] && Check_Fit(nCurrentPiece,nCurrentRotation,nCurrentX,nCurrentY + 1)) // check ->D
                ++nCurrentY;
        
        if(Key[3] && Check_Fit(nCurrentPiece,nCurrentRotation + 1,nCurrentX,nCurrentY) && !Rotate_Hold)
                ++nCurrentRotation, Rotate_Hold = true;
        else Rotate_Hold = false;

        if(ForceDown)
        {
            if(Check_Fit(nCurrentPiece,nCurrentRotation,nCurrentX,nCurrentY + 1))
                ++ nCurrentY;
            else // then this piece should be locked
            {
                // lock the piece in the field
                for(int px = 0;px < 4; ++ px)
                    for(int py = 0;py < 4; ++ py)
                        if(tetromino[nCurrentPiece][Rotate(px,py,nCurrentRotation)] == L'X')
                            pField[(nCurrentY + py) * width + (nCurrentX + px)] = nCurrentPiece + 1;
                
                // speed up
                ++nPiececount;
                if(nPiececount % 10 == 0)
                    if(nSpeed >= 10) -- nSpeed;
                
                // check if there is any line
                for(int py = 0;py < 4; ++ py)
                    if(nCurrentY + py < height - 1) // within the boundary
                    {
                        bool bline = true;
                        for(int px = 1;px < width - 1; ++ px)
                            if(pField[(nCurrentY + py) * width + px] == 0) bline = false;
                        if(bline) // Remove line, set to '='
                        {
                            for(int px = 1;px < width - 1; ++ px)
                                pField[(nCurrentY + py) * width + px] = 8;
                            vLine.push_back(nCurrentY + py); // add it to remove;
                        }
                    }
                
                // add the scores
                nScore += 25;
                if(!vLine.empty())
                    nScore += (1 << vLine.size()) * 100;

                //drop a new piece
                nCurrentPiece = rand() % 7;
                nCurrentRotation = 0;
                nCurrentX = width >> 1, nCurrentY = 0;

                gameover = !Check_Fit(nCurrentPiece,nCurrentRotation,nCurrentX,nCurrentY);
            }
            nSpeedCounter = 0;
        }

        // RENDER OUTPUT

        // Draw Field
        for(int x = 0;x < width; ++ x)
            for(int y = 0;y < height; ++ y)
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * width + x]]; // here +2 is left 2 lines for socre display
        
        //Draw Current Piece
        for(int px = 0;px < 4; ++ px)
            for(int py = 0;py < 4; ++ py)
                if(tetromino[nCurrentPiece][Rotate(px,py,nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 'A';

        //Draw Current Scores
        swprintf_s(&screen[2 * nScreenWidth + width + 6],16,L"Score: %8d",nScore);

    
        if(!vLine.empty())
        {
            WriteConsoleOutputCharacterW(hConsole,screen,nScreenWidth * nScreenHeight,{0,0},&dwBytesWritten);
            this_thread :: sleep_for(400ms); // Delay a bit~

            for(int i = 0,v;i < vLine.size(); ++ i) // actually an array can also solve this problem
            {
                v = vLine[i];
                for(int px = 1;px < width - 1; ++ px)
                {
                    for(int py = v;py; -- py)
                        pField[py * width + px] = pField[(py - 1) * width + px];
                    pField[px] = 0; // the first row should be 0
                }
            }
            vLine.clear();
        }
        
        //Display Frame
        WriteConsoleOutputCharacterW(hConsole,screen,nScreenHeight * nScreenWidth,{0,0},&dwBytesWritten);
    }

    CloseHandle(hConsole);
    cout<< "Game Over! Score: " << nScore <<endl;
    system("pause");

    return 0;
}
