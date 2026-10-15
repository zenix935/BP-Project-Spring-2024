#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>
#include <windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfx.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
using namespace std;

// Screen dimensions
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 1000;
vector<string> filenames; //Holds content of the leftbox
string FileName; //name of the file
string FILENAME;
void ensureLastLineVisible(int currentLine, int &scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines);
void leftfilenames();
vector<pair<int,SDL_Color>> GetWordColor(string &line,bool darkmode);
void ErrorForInclude(vector<string>& errors, const vector<string>& lines);
void BuiltDebugger(vector<string>& errors,const vector<string>& lines);
string SaveOP(SDL_Renderer *renderer,vector<string> &lines,string existingfile,bool fileexists);
void OpenFile(int num,vector<string> &lines);
bool compile(const string &filePath);
void run_in_another_window(const string &filePath);
void autocomplete(vector<string> &lines,int &cursorPos,int currentLine,bool tab);
int levenshteinDistance(const std::string &s1, const std::string &s2);
void findSimilarWords(const vector<string> &lines,vector<string> &errors, int threshold);
void checkVariableDeclarations(const vector<string>& lines,vector<string> &errors);
string getFunctionName(const string& declaration);
int countParameters(const std::string& declaration);
void functionvarinum(const vector<string>& lines,vector<string> &errors);
void musicplayer(string musicpath,Mix_Music* music,bool muted);

int main(int argc, char* argv[]) {
    Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048);
    //get file names for leftbox
    leftfilenames();
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Unusable Studio Code",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("Monaco.TTF", 24); // Replace with the path to your .ttf font
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    //load icon
    SDL_Surface* icon=IMG_Load("C:/MINE/codes/Uni/PROJECT_BP_AA/pic/iconpic.png");
    SDL_SetWindowIcon(window,icon);
    SDL_FreeSurface(icon);

    string musicpath="C:/MINE/codes/Uni/PROJECT_BP_AA/musics/Pianaura - La Foule (La Monde) (Piano Version).mp3";
    vector<string> rightboxB={"File","Edit","View","Debug","Compile","Run","music"}; // Holds texts for right box
    std::vector<std::string> lines = {""}; // Holds multiple lines of text
    vector<vector<string>> LINES; // Holds multiple vectors of line for undo and redo
    vector<string> errors={""}; //Holds the errors for IFbox
    vector<string> IFtext={""};
    int currentLine = 0; // Track the current line being edited
    int cursorPos = 1; // Track the cursor position within the current line
    int scrollOffset = 0; // Keeps track of scrolling
    int backspacenum=0; // Tracks the number of backspaces
    int textinputnum=0; //Tracks the number of text inputs
    int DoNum=0; //tracks the redo and undo num
    int cursorpos1,cursorpos2,linepos1,linepos2;
    const int LINE_HEIGHT = TTF_FontHeight(font); // Height of each line
    bool compiled=false; //check if the file is compiled
    bool openedfile=false;
    bool muted=false;


    // Timer for cursor blinking
    Uint32 lastCursorToggle = SDL_GetTicks();
    bool cursorVisible = true;
    const Uint32 CURSOR_BLINK_INTERVAL = 500; // 500 ms for blinking

    SDL_Event e;
    bool quit = false;
    bool darkmode=true;
    SDL_Color leftbox,mainbox,cursor,rightbox,IFbox,IFbox2,textColor,rightbox2,commentColor,errColor;
    Mix_Music* music;

    musicplayer(musicpath,music,muted);

    while (!quit) {
        // Handle cursor blinking
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime > lastCursorToggle + CURSOR_BLINK_INTERVAL) {
            cursorVisible = !cursorVisible;
            lastCursorToggle = currentTime;
        }
        //handle dark mode
        if(darkmode)
        {
            leftbox={50,50,50,255};
            rightbox={50,50,50,255};
            mainbox={0,0,0,255};
            cursor={255,255,255,255};
            IFbox={0,0,40,255};
            IFbox2={0,255,100,255};
            textColor = {255,255,255,255};
            rightbox2={70,70,70,255};
            commentColor={92,99,112,255};
            errColor={255,100,100,255};
        }
        //handle light mode
        else
        {
            leftbox={225,225,225,255};
            rightbox={225,225,225,255};
            mainbox={255,255,255,255};
            cursor={0,0,0,255};
            IFbox={255,255,215,255};
            IFbox2={50,150,50,255};
            textColor = {0,0,0,255};
            rightbox2={200,200,200,255};
            commentColor={128,128,128,255};
            errColor={255,0,0,255};
        }

        // Event loop
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                int Xmouse,Ymouse;
                SDL_Rect quitpic={500,450,600,300};
                bool sideQuit=false;
                while(!sideQuit)
                {
                    SDL_Surface *surquitpic1=IMG_Load("C:/MINE/codes/Uni/PROJECT_BP_AA/pic/quitpic.png");
                    SDL_Texture *texquitpic1=SDL_CreateTextureFromSurface(renderer,surquitpic1);
                    SDL_FreeSurface(surquitpic1);
                    SDL_RenderCopy(renderer,texquitpic1,nullptr,&quitpic);
                    SDL_RenderPresent(renderer);
                    SDL_Event event;
                    while(SDL_PollEvent(&event)!=0)
                    {
                        if(event.type==SDL_MOUSEBUTTONDOWN)
                        {
                            SDL_GetGlobalMouseState(&Xmouse,&Ymouse);
                            //save the file
                            if(Xmouse<=755 && Xmouse>=575 && Ymouse<=690 && Ymouse>=630)
                            {
                                FILENAME=SaveOP(renderer,lines,FILENAME,openedfile);
                                quit=true;
                                sideQuit=true;
                            }
                            if(Xmouse<=1015 && Xmouse>=835 && Ymouse<=690 && Ymouse>=630)
                            {
                                quit=true;
                                sideQuit=true;
                            }
                            if((Xmouse<500 && Ymouse<1000) || (Xmouse>0 && Ymouse>0 && Ymouse<450) || (Xmouse>0 && Ymouse>750) || (Xmouse>1100 && Ymouse>0))
                            {
                                sideQuit=true;
                                break;
                            }
                        }
                    }
                }
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                // Handle scroll
                if (e.wheel.y > 0) { // Scroll up
                    scrollOffset = std::max(0, scrollOffset - LINE_HEIGHT);
                } else if (e.wheel.y < 0) { // Scroll down
                    scrollOffset += LINE_HEIGHT;
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    backspacenum++;
                    // Ensure cursorPos is within the valid range
                    if (cursorPos > 1 && cursorPos <= lines[currentLine].size()) {
                        // Remove character before cursor
                        lines[currentLine].erase(cursorPos - 1, 1);
                        cursorPos--;
                    }
                    else if (currentLine > 0) {
                        // Merge with previous line
                        cursorPos = lines[currentLine - 1].size();
                        lines[currentLine - 1] += lines[currentLine].substr(1, lines[currentLine].size());
                        lines.erase(lines.begin() + currentLine);
                        currentLine--;
                    }

                    // Ensure there's always at least one line
                    if (lines.empty()) {
                        lines.emplace_back("");
                        currentLine = 0;
                        cursorPos = 0;
                    }
                    if(backspacenum==3)
                    {
                        backspacenum=0;
                        if(LINES.size()==10)
                        {
                            LINES.erase(LINES.begin());
                            LINES.push_back(lines);
                        }
                        else
                        {
                            LINES.push_back(lines);
                        }
                    }
                }
                else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (cursorPos <= lines[currentLine].size()) {
                        string remainder = lines[currentLine].substr(cursorPos);
                        lines[currentLine] = lines[currentLine].substr(0, cursorPos);
                        lines.insert(lines.begin() + currentLine + 1, remainder);
                        currentLine++;
                        cursorPos = 0;
                        ensureLastLineVisible(currentLine, scrollOffset, 800, LINE_HEIGHT, lines.size());
                    }
                    if(LINES.size()==10)
                    {
                        LINES.erase(LINES.begin());
                        LINES.push_back(lines);
                    }
                    else
                    {
                        LINES.push_back(lines);
                    }
                }
                else if (e.key.keysym.sym == SDLK_TAB) {
                    // Add spaces for tab
                    int temp=lines[currentLine].length();
                    autocomplete(lines,cursorPos,currentLine,true);
                    int temp2=lines[currentLine].length();
                    if(temp2==temp)
                    {
                        lines[currentLine].insert(cursorPos, "    ");
                        cursorPos += 4;
                    }
                    if(LINES.size()==10)
                    {
                        LINES.erase(LINES.begin());
                        LINES.push_back(lines);
                    }
                    else
                    {
                        LINES.push_back(lines);
                    }
                }
                else if (e.key.keysym.sym == SDLK_LEFT) {
                    // Move cursor left
                    if (cursorPos > 1) {
                        cursorPos--;
                    } else if (currentLine > 0) {
                        currentLine--;
                        cursorPos = lines[currentLine].size();
                    }
                }
                else if (e.key.keysym.sym == SDLK_RIGHT) {
                    // Move cursor right
                    if (cursorPos < lines[currentLine].size()) {
                        cursorPos++;
                    } else if (currentLine < lines.size() - 1) {
                        currentLine++;
                        cursorPos = 0;
                    }
                }
                else if (e.key.keysym.sym == SDLK_UP) {
                    // Move cursor up
                    if (currentLine > 0) {
                        currentLine--;
                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                        ensureLastLineVisible(currentLine, scrollOffset, 800, LINE_HEIGHT, lines.size());
                    }
                }
                else if (e.key.keysym.sym == SDLK_DOWN) {
                    if (currentLine < lines.size() - 1) {
                        currentLine++;
                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                        ensureLastLineVisible(currentLine, scrollOffset, 800, LINE_HEIGHT, lines.size());
                    }
                }
                else if (e.key.keysym.sym == SDLK_RSHIFT)
                {
                    SDL_Event event;
                    bool sidequit=false;
                    cursorpos1=cursorPos;
                    linepos1=currentLine;
                    while(!sidequit)
                    {
                        while(SDL_PollEvent(&event))
                        {
                            if(event.type == SDL_KEYDOWN)
                            {
                                if (event.key.keysym.sym == SDLK_LEFT) {
                                    // Move cursor left
                                    if (cursorPos > 1) {
                                        cursorPos--;
                                    } else if (currentLine > 0) {
                                        currentLine--;
                                        cursorPos = lines[currentLine].size();
                                    }
                                }
                                else if (event.key.keysym.sym == SDLK_RIGHT) {
                                    // Move cursor right
                                    if (cursorPos < lines[currentLine].size()) {
                                        cursorPos++;
                                    } else if (currentLine < lines.size() - 1) {
                                        currentLine++;
                                        cursorPos = 0;
                                    }
                                }
                                else if (event.key.keysym.sym == SDLK_UP) {
                                    // Move cursor up
                                    if (currentLine > 0) {
                                        currentLine--;
                                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                                        ensureLastLineVisible(currentLine, scrollOffset, 800, LINE_HEIGHT, lines.size());
                                    }
                                }
                                else if (event.key.keysym.sym == SDLK_DOWN) {
                                    if (currentLine < lines.size() - 1) {
                                        currentLine++;
                                        cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
                                        ensureLastLineVisible(currentLine, scrollOffset, 800, LINE_HEIGHT, lines.size());
                                    }
                                }
                            }
                            else if (event.type == SDL_KEYUP)
                            {
                                if(event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)
                                {
                                    cursorpos2=cursorPos;
                                    linepos2=currentLine;
                                    if(linepos2<linepos1)
                                    {
                                        swap(linepos2,linepos1);
                                        swap(cursorpos1,cursorpos2);
                                    }
                                    sidequit=true;
                                }
                            }
                            // Clear screen
                            SDL_SetRenderDrawColor(renderer,mainbox.r,mainbox.g,mainbox.b,mainbox.a);
                            SDL_RenderClear(renderer);
                            boxRGBA(renderer,0,0,300,1000,leftbox.r,leftbox.g,leftbox.b,leftbox.a);
                            boxRGBA(renderer,1300,0,1600,1000,rightbox.r,rightbox.g,rightbox.b,rightbox.a);
                            boxRGBA(renderer,0,800,1600,1000,IFbox.r,IFbox.g,IFbox.b,IFbox.a);

                            // Render text
                            int y = -scrollOffset+5; // Start rendering based on the scroll offset
                            //IFbox
                            int Yerr=800,Xerr=20;
                            for(auto & error : errors)
                            {
                                if(error.empty())
                                    error=" ";
                                SDL_Surface *err=TTF_RenderText_Blended(font,error.c_str(),errColor);
                                SDL_Texture* errT=SDL_CreateTextureFromSurface(renderer,err);
                                int textWidth=(err->w)*6/10;
                                int textHeight=(err->h)*6/10;
                                SDL_Rect errRect={Xerr,Yerr,textWidth,textHeight};
                                SDL_FreeSurface(err);
                                SDL_RenderCopy(renderer,errT, nullptr,&errRect);
                                SDL_DestroyTexture(errT);
                                Yerr+=16;
                            }
                            int YIF=800,XIF=820;
                            for(auto & i : IFtext)
                            {
                                if(i.empty())
                                    i=" ";
                                SDL_Surface* IFsur=TTF_RenderText_Blended(font,i.c_str(),IFbox2);
                                SDL_Texture* IFtex=SDL_CreateTextureFromSurface(renderer,IFsur);
                                int textWidth=(IFsur->w);
                                int textHeight=(IFsur->h);
                                SDL_Rect IFRect={XIF,YIF,textWidth,textHeight};
                                SDL_FreeSurface(IFsur);
                                SDL_RenderCopy(renderer,IFtex, nullptr,&IFRect);
                                SDL_DestroyTexture(IFtex);
                                YIF+=24;
                            }
                            //rightbox texts
                            int Yright=5;
                            for(const auto & i : rightboxB)
                                {
                                SDL_Surface* RightboxText=TTF_RenderText_Blended(font,i.c_str(),textColor);
                                SDL_Texture* Text=SDL_CreateTextureFromSurface(renderer,RightboxText);
                                int textWidth=RightboxText->w;
                                int textHeight=RightboxText->h;
                                SDL_Rect rightboxrect={1320,Yright,textWidth,textHeight};
                                SDL_FreeSurface(RightboxText);
                                SDL_RenderCopy(renderer,Text, nullptr,&rightboxrect);
                                SDL_DestroyTexture(Text);
                                Yright+=textHeight+10;
                                }
                            //leftbox texts
                            string leftfilename=FileName+":";
                            SDL_Surface* leftfilesur=TTF_RenderText_Blended(font,leftfilename.c_str(),textColor);
                            SDL_Texture* leftfiletxt=SDL_CreateTextureFromSurface(renderer,leftfilesur);
                            int textWidth2=leftfilesur->w;
                            int textHeight2=leftfilesur->h;
                            SDL_Rect leftboxrect={2,5,textWidth2,textHeight2};
                            SDL_FreeSurface(leftfilesur);
                            SDL_RenderCopy(renderer,leftfiletxt, nullptr,&leftboxrect);
                            SDL_DestroyTexture(leftfiletxt);
                            int Yleft=35;
                            for(const auto & filename : filenames)
                            {
                                SDL_Surface* leftboxsur=TTF_RenderText_Blended(font,filename.c_str(),textColor);
                                SDL_Texture* leftboxtxt=SDL_CreateTextureFromSurface(renderer,leftboxsur);
                                int textWidth=leftboxsur->w*8/10;
                                int textHeight=leftboxsur->h*8/10;
                                SDL_Rect leftboxrect2={12,Yleft,textWidth,textHeight};
                                SDL_FreeSurface(leftboxsur);
                                SDL_RenderCopy(renderer,leftboxtxt, nullptr,&leftboxrect2);
                                SDL_DestroyTexture(leftboxtxt);
                                Yleft+=textHeight+10;
                            }
                            y=-scrollOffset+5;
                            //main text
                            for (size_t i = 0; i < lines.size(); ++i) {
                                if (y + LINE_HEIGHT > 0 && y < 800) { // Render only visible lines
                                    if (lines[i].empty())
                                        lines[i] = " ";// Show cursor on the current line
                                    int x=300;
                                    int temp=lines[i].size();
                                    for(int p=0;p<lines[i].size();p++)
                                    {
                                        if(lines[i][p]=='/' && lines[i][p-1]=='/')
                                        {
                                            temp=p-1;
                                            break;
                                        }
                                    }
                                    vector<pair<int,SDL_Color>> highlights=GetWordColor(lines[i],darkmode);
                                    for(size_t j=0;j<lines[i].size();j++)
                                    {
                                        SDL_Color charColor=textColor;
                                        if(j>=temp)
                                        {
                                            charColor=commentColor;
                                        }
                                        else
                                        {
                                            for(const auto &highlight:highlights)
                                            {
                                                if(highlight.first==j)
                                                {
                                                    charColor=highlight.second;
                                                    break;
                                                }
                                            }
                                        }
                                        string charString(1,lines[i][j]);
                                        SDL_Surface* textSurface = TTF_RenderText_Blended(font,charString.c_str(),charColor);
                                        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                                        int textWidth = textSurface->w*96/100;
                                        int textHeight = textSurface->h*96/100;
                                        SDL_Rect renderQuad = {x, y, textWidth, textHeight};
                                        SDL_FreeSurface(textSurface);
                                        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
                                        SDL_DestroyTexture(textTexture);
                                        x+=textWidth+1;
                                    }
                                    // Render cursor if this is the current line
                                    if (i == currentLine) {
                                        int cursorX = 0;
                                        if (cursorPos > 0) {
                                            TTF_SizeText(font, lines[i].substr(0, cursorPos).c_str(), &cursorX, nullptr);
                                        }
                                        cursorX += 1; // Add padding for the left margin
                                        SDL_SetRenderDrawColor(renderer,cursor.r,cursor.g,cursor.b,cursor.a);
                                        SDL_RenderDrawLine(renderer, cursorX+300, y, cursorX+300, y + LINE_HEIGHT);
                                    }
                                }
                                    y += LINE_HEIGHT; // Move to the next line
                            }
                            // Update screen
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                else if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL)
                {
                    SDL_Event event;
                    bool sidequit=false;
                    while(!sidequit)
                    {
                        while(SDL_PollEvent(&event))
                        {
                            if(event.type == SDL_KEYUP)
                            {
                                if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL)
                                    sidequit=true;
                                else if(char(event.key.keysym.sym)=='c')
                                {
                                    if(linepos1==linepos2)
                                    {
                                        string ClipBoard;
                                        ClipBoard+=lines[linepos1];
                                        SDL_SetClipboardText(ClipBoard.c_str());
                                    }
                                    else
                                    {
                                        string ClipBoard;
                                        ClipBoard+=lines[linepos1].substr(cursorpos1,lines[linepos1].size())+"\n";
                                        for(int i=linepos1+1;i<linepos2;i++)
                                        {
                                            string temp=lines[i].substr(0,lines[i].size());
                                            ClipBoard+=temp+"\n";
                                        }
                                        ClipBoard+=lines[linepos2].substr(0,cursorpos2);
                                        SDL_SetClipboardText(ClipBoard.c_str());
                                    }
                                }
                                else if(char(event.key.keysym.sym)=='v')
                                {
                                    char* ClipBoard=SDL_GetClipboardText();
                                    stringstream ss(ClipBoard);
                                    string temp;
                                    while(getline(ss, temp, '\n'))
                                    {
                                        auto it=lines.begin()+currentLine;
                                        lines.insert(it,temp);
                                        currentLine++;
                                    }
                                }
                                else if(char(event.key.keysym.sym)=='x')
                                {
                                    if(linepos1==linepos2)
                                    {
                                        string ClipBoard;
                                        ClipBoard+=lines[linepos1];
                                        SDL_SetClipboardText(ClipBoard.c_str());
                                        lines[linepos1].erase(cursorpos1,cursorpos2);
                                    }
                                    else
                                    {
                                        string ClipBoard;
                                        ClipBoard+=lines[linepos1].substr(cursorpos1,lines[linepos1].size())+"\n";
                                        for(int i=linepos1+1;i<linepos2;i++)
                                        {
                                            string temp=lines[i].substr(0,lines[i].size());
                                            ClipBoard+=temp+"\n";
                                        }
                                        ClipBoard+=lines[linepos2].substr(0,cursorpos2);
                                        SDL_SetClipboardText(ClipBoard.c_str());

                                        lines[linepos1].erase(cursorpos1);
                                        for(int i=linepos1+1;i<linepos2;i++)
                                            lines[i].erase();
                                        lines[linepos2].erase(0,cursorpos2);
                                    }
                                }
                                else if(char(event.key.keysym.sym)=='z')
                                {
                                    if(DoNum<LINES.size())
                                    {
                                        DoNum++;
                                        lines=LINES[LINES.size()-DoNum];
                                        currentLine=lines.size()-1;
                                        cursorPos=lines[currentLine].length();
                                    }
                                }
                                else if(char(event.key.keysym.sym)=='a')
                                {
                                    cursorpos1=0;
                                    cursorpos2=lines[lines.size()-1].length()-1;
                                    linepos1=0;
                                    linepos2=lines.size()-1;
                                }
                                else if(char(event.key.keysym.sym)=='g')
                                {
                                    bool sideQuit=false;
                                    string numnum=" ";
                                    string temp;
                                    int cursorpos=0;
                                    SDL_Rect picrect={500,450,600,300};
                                    SDL_Color color={0,0,0,255};
                                    while(!sideQuit)
                                    {
                                        SDL_Event e2;
                                        SDL_Surface* picsur=IMG_Load("C:/MINE/codes/Uni/PROJECT_BP_AA/pic/linepic.png");
                                        SDL_Texture* picTexture = SDL_CreateTextureFromSurface(renderer, picsur);
                                        SDL_RenderCopy(renderer, picTexture, nullptr, &picrect);
                                        SDL_DestroyTexture(picTexture);
                                        SDL_FreeSurface(picsur);
                                        while(SDL_PollEvent(&e2))
                                        {
                                            if(e2.type==SDL_TEXTINPUT)
                                            {
                                                temp=e2.text.text;
                                                if(temp[temp.size()-1]>='0' && temp[temp.size()-1]<='9')
                                                {
                                                    numnum.insert(cursorpos,(e2.text.text));
                                                    cursorpos++;
                                                }
                                            }
                                            else if(e2.type==SDL_KEYDOWN)
                                            {
                                                if(e2.key.keysym.sym==SDLK_RETURN)
                                                {
                                                    if(stoi(numnum)<=lines.size())
                                                    {
                                                        currentLine=stoi(numnum)-1;
                                                        cursorPos=lines[currentLine].length();
                                                    }
                                                    sidequit=true;
                                                    sideQuit=true;
                                                }
                                                else if(e2.key.keysym.sym==SDLK_BACKSPACE)
                                                {
                                                    if(cursorpos>0)
                                                    {
                                                        numnum.erase(cursorpos-1,1);
                                                        cursorpos--;
                                                    }
                                                }
                                            }
                                        }
                                        int X=700,Y=675;
                                        SDL_Surface* numsur=TTF_RenderText_Blended(font,numnum.c_str(),color);
                                        SDL_Texture* numTexture = SDL_CreateTextureFromSurface(renderer, numsur);
                                        int numHeight=numsur->h;
                                        int numWidth=numsur->w;
                                        SDL_Rect numRect={X,Y,numWidth,numHeight};
                                        SDL_RenderCopy(renderer, numTexture, nullptr, &numRect);
                                        SDL_DestroyTexture(numTexture);
                                        SDL_FreeSurface(numsur);
                                        SDL_RenderPresent(renderer);
                                    }
                                    ensureLastLineVisible(currentLine,scrollOffset,800,LINE_HEIGHT,lines.size());
                                }
                                else if(char(event.key.keysym.sym)=='s')
                                {
                                    FILENAME=SaveOP(renderer,lines,FILENAME,openedfile);
                                    if(!FILENAME.empty())
                                    {
                                        openedfile=true;
                                    }
                                    sidequit=true;
                                }
                            }
                        }
                    }
                }
            }
            else if (e.type == SDL_TEXTINPUT) {
                if (e.text.text)
                {
                    textinputnum++;
                    if(textinputnum==4)
                    {
                        textinputnum=0;
                        if(LINES.size()==10)
                        {
                            LINES.erase(LINES.begin());
                            LINES.push_back(lines);
                        }
                        else
                            LINES.push_back(lines);
                    }
                    DoNum=0;
                    if(lines[currentLine].length()>70)
                    {
                        currentLine += 1;
                        lines.emplace_back("");
                        cursorPos=0;
                    }
                    lines[currentLine].insert(cursorPos, e.text.text);
                    autocomplete(lines,cursorPos,currentLine,false);
                    cursorPos += strlen(e.text.text);
                    ensureLastLineVisible(currentLine, scrollOffset, 800, LINE_HEIGHT, lines.size());
                }
            }
            //handle clicks
            else if(e.type==SDL_MOUSEBUTTONDOWN)
            {
                int Xmouse, Ymouse;
                SDL_GetGlobalMouseState(&Xmouse,&Ymouse);
                cout<<Xmouse<<" "<<Ymouse<<endl;
                //File
                if(Xmouse<=1600 && Xmouse>=1300 && Ymouse<=45 && Ymouse>=20)
                {
                    bool sidequit=false;
                    while(!sidequit)
                    {
                        boxRGBA(renderer,1100,5,1300,105,rightbox2.r,rightbox2.g,rightbox2.b,rightbox2.a);
                        vector<string> rightboxtext2={"New Project","Save Project","Exit"};
                        int Y=10;
                        for(const auto & i : rightboxtext2)
                        {
                            SDL_Surface* rightboxsur2=TTF_RenderText_Blended(font,i.c_str(),textColor);
                            SDL_Texture* rightboxT2=SDL_CreateTextureFromSurface(renderer,rightboxsur2);
                            int textWidth=rightboxsur2->w;
                            int textHeight=rightboxsur2->h;
                            SDL_Rect textRect={1105,Y,textWidth,textHeight};
                            SDL_RenderCopy(renderer,rightboxT2, nullptr,&textRect);
                            SDL_DestroyTexture(rightboxT2);
                            SDL_FreeSurface(rightboxsur2);
                            Y+=textHeight+10;
                        }
                        SDL_RenderPresent(renderer);
                        SDL_Event e2;
                        while(SDL_PollEvent(&e2))
                        {
                            int Xmouse2,Ymouse2;
                            if(e2.type==SDL_MOUSEBUTTONDOWN)
                            {
                                SDL_GetGlobalMouseState(&Xmouse2,&Ymouse2);
                                //new project
                                if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=40 && Ymouse2>=20)
                                {
                                    SDL_Rect newPpic={500,450,600,300};
                                    while(!sidequit)
                                    {
                                        SDL_Surface* newPsur=IMG_Load("C:/MINE/codes/Uni/PROJECT_BP_AA/pic/newprojectpic.png");
                                        SDL_Texture* newPts=SDL_CreateTextureFromSurface(renderer,newPsur);
                                        SDL_RenderCopy(renderer,newPts, nullptr,&newPpic);
                                        SDL_DestroyTexture(newPts);
                                        SDL_FreeSurface(newPsur);
                                        SDL_RenderPresent(renderer);
                                        SDL_Event event;
                                        while(SDL_PollEvent(&event))
                                        {
                                            if(event.type==SDL_MOUSEBUTTONDOWN)
                                            {
                                                SDL_GetGlobalMouseState(&Xmouse2,&Ymouse2);
                                                if(Xmouse2<=720 && Xmouse>=600 && Ymouse2<=710 && Ymouse2>=650)
                                                {
                                                    openedfile=false;
                                                    FILENAME="";
                                                    sidequit=true;
                                                }
                                                else if(Xmouse2<=1000 && Xmouse2>=880 && Ymouse2<=710 && Ymouse2>=650)
                                                {
                                                    openedfile=false;
                                                    lines.clear();
                                                    lines.emplace_back("");
                                                    cursorPos=1;
                                                    currentLine=0;
                                                    FILENAME="";
                                                    sidequit=true;
                                                }
                                            }
                                        }
                                    }
                                }

                                //save
                                else if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=80 && Ymouse2>=60)
                                {
                                    FILENAME=SaveOP(renderer,lines,FILENAME,openedfile);
                                    if(!FILENAME.empty())
                                        openedfile=true;
                                    sidequit=true;
                                }

                                //Exit
                                else if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=115 && Ymouse2>=93)
                                {
                                    SDL_Rect quitpic={500,450,600,300};
                                    bool sideQuit=false;
                                    while(!sideQuit)
                                    {
                                        SDL_Surface *surquitpic1=IMG_Load("C:/MINE/codes/Uni/PROJECT_BP_AA/pic/quitpic.png");
                                        SDL_Texture *texquitpic1=SDL_CreateTextureFromSurface(renderer,surquitpic1);
                                        SDL_FreeSurface(surquitpic1);
                                        SDL_RenderCopy(renderer,texquitpic1, nullptr,&quitpic);
                                        SDL_RenderPresent(renderer);
                                        SDL_Event event;
                                        while(SDL_PollEvent(&event)!=0)
                                        {
                                            if(event.type==SDL_MOUSEBUTTONDOWN)
                                            {
                                                SDL_GetGlobalMouseState(&Xmouse,&Ymouse);
                                                //save the file
                                                if(Xmouse<=755 && Xmouse>=575 && Ymouse<=690 && Ymouse>=630)
                                                {
                                                    FILENAME=SaveOP(renderer,lines,FILENAME,openedfile);
                                                    quit=true;
                                                    sidequit=true;
                                                    sideQuit=true;
                                                }
                                                if(Xmouse<=1015 && Xmouse>=835 && Ymouse<=690 && Ymouse>=630)
                                                {
                                                    quit=true;
                                                    sidequit=true;
                                                    sideQuit=true;
                                                }
                                                if((Xmouse<500 && Ymouse<1000) || (Xmouse>0 && Ymouse>0 && Ymouse<450) || (Xmouse>0 && Ymouse>750) || (Xmouse>1100 && Ymouse>0))
                                                {
                                                    sidequit=true;
                                                    sideQuit=true;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                    sidequit=true;
                            }
                        }
                    }
                }

                //Edit
                if(Xmouse<=1600 && Xmouse>=1300 && Ymouse<=80 && Ymouse>=50)
                {
                    bool sidequit=false;
                    while(!sidequit)
                    {
                        boxRGBA(renderer,1100,50,1300,120,rightbox2.r,rightbox2.g,rightbox2.b,rightbox2.a);
                        vector<string> rightboxtext2={"Undo","Redo"};
                        int Y=55;
                        for(const auto & i : rightboxtext2)
                        {
                            SDL_Surface* rightboxsur2=TTF_RenderText_Blended(font,i.c_str(),textColor);
                            SDL_Texture* rightboxT2=SDL_CreateTextureFromSurface(renderer,rightboxsur2);
                            int textWidth=rightboxsur2->w;
                            int textHeight=rightboxsur2->h;
                            SDL_Rect textRect={1105,Y,textWidth,textHeight};
                            SDL_RenderCopy(renderer,rightboxT2, nullptr,&textRect);
                            SDL_DestroyTexture(rightboxT2);
                            SDL_FreeSurface(rightboxsur2);
                            Y+=textHeight+10;
                        }
                        SDL_RenderPresent(renderer);
                        SDL_Event e2;
                        while(SDL_PollEvent(&e2))
                        {
                            int Xmouse2,Ymouse2;
                            if(e2.type==SDL_MOUSEBUTTONDOWN)
                            {
                                SDL_GetGlobalMouseState(&Xmouse2,&Ymouse2);
                                //Undo
                                if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=87 && Ymouse2>=70)
                                {
                                    if(DoNum<LINES.size())
                                    {
                                        DoNum++;
                                        lines=LINES[LINES.size()-DoNum];
                                        sidequit=true;
                                        currentLine=lines.size()-1;
                                        cursorPos=lines[currentLine].length();
                                    }
                                    else
                                        sidequit=true;
                                }

                                //Redo
                                else if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2>=105 && Ymouse2<=125)
                                {
                                    if(DoNum>0)
                                    {
                                        DoNum--;
                                        lines=LINES[LINES.size()-DoNum];
                                        sidequit=true;
                                        currentLine=lines.size()-1;
                                        cursorPos=lines[currentLine].length();
                                    }
                                    else
                                        sidequit=true;
                                }
                                else
                                    sidequit=true;
                            }
                        }
                    }
                }

                //view
                if(Xmouse<=1600 && Xmouse>=1300 && Ymouse<=110 && Ymouse>=90)
                {
                    bool sidequit=false;
                    while(!sidequit)
                    {
                        boxRGBA(renderer,1100,90,1300,160,rightbox2.r,rightbox2.g,rightbox2.b,rightbox2.a);
                        vector<string> rightboxtext2={"DarkMode","LightMode"};
                        int Y=95;
                        for(const auto & i : rightboxtext2)
                        {
                            SDL_Surface* rightboxsur2=TTF_RenderText_Blended(font,i.c_str(),textColor);
                            SDL_Texture* rightboxT2=SDL_CreateTextureFromSurface(renderer,rightboxsur2);
                            int textWidth=rightboxsur2->w;
                            int textHeight=rightboxsur2->h;
                            SDL_Rect textRect={1105,Y,textWidth,textHeight};
                            SDL_RenderCopy(renderer,rightboxT2, nullptr,&textRect);
                            SDL_DestroyTexture(rightboxT2);
                            SDL_FreeSurface(rightboxsur2);
                            Y+=textHeight+10;
                        }
                        SDL_RenderPresent(renderer);
                        SDL_Event e2;
                        while(SDL_PollEvent(&e2))
                        {
                            int Xmouse2,Ymouse2;
                            if(e2.type==SDL_MOUSEBUTTONDOWN)
                            {
                                SDL_GetGlobalMouseState(&Xmouse2,&Ymouse2);
                                cout<<Xmouse2<<" "<<Ymouse2<<endl;
                                //dark&light mode
                                if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=130 && Ymouse2>=110)
                                {
                                    darkmode=true;
                                    sidequit=true;
                                }
                                else if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=165 && Ymouse2>=145)
                                {
                                    darkmode=false;
                                    sidequit=true;
                                }
                                else
                                    sidequit=true;
                            }
                        }
                    }
                }
                //Debug
                if(Xmouse<=1600 && Xmouse>=1300 && Ymouse<=145 && Ymouse>=123)
                {
                    errors={""};
                    ErrorForInclude(errors,lines);
                    BuiltDebugger(errors,lines);
                    findSimilarWords(lines,errors,2);
                    checkVariableDeclarations(lines,errors);
                    functionvarinum(lines,errors);
                }

                //Compile
                if(Xmouse<=1600 && Xmouse>=1300 && Ymouse<=180 && Ymouse>=160)
                {
                    bool success=compile(FILENAME);
                    if(success)
                    {
                        IFtext={""};
                        IFtext.emplace_back("file compiled successfully");
                        compiled=true;
                    }
                    else
                    {
                        IFtext={""};
                        IFtext.push_back("failed to compile!");
                    }
                }

                //Run
                if(Xmouse<=1600 && Xmouse>=1300 && Ymouse<=215 && Ymouse>=195)
                {
                    if(compiled)
                        run_in_another_window(FILENAME);
                    else
                        errors.push_back("please compile the file first");
                }

                //left box to open the files
                if(Xmouse<=300 && Xmouse>=0 && Ymouse>=20 && Ymouse<=1000)
                {
                    int Y=Ymouse;
                    Y-=50;
                    if(Y>0)
                    {
                        Y/=30;
                        OpenFile(Y+1,lines);
                        openedfile=true;
                    }
                }

                //music
                if(Xmouse<=1600 && Xmouse>=1300 && Ymouse<250 && Ymouse>=230)
                {
                    bool sidequit=false;
                    while(!sidequit)
                    {
                        vector<string> musicboxtext={"Nevermore","La Foule","Idea 10","mute/unmute"};
                        boxRGBA(renderer,1100,230,1300,370,rightbox2.r,rightbox2.g,rightbox2.b,rightbox2.a);
                        int Y=235;
                        for(int i=0;i<musicboxtext.size();i++)
                        {
                            SDL_Surface* musicsur=TTF_RenderText_Blended(font,musicboxtext[i].c_str(),textColor);
                            SDL_Texture* music2=SDL_CreateTextureFromSurface(renderer,musicsur);
                            int textWidth=musicsur->w;
                            int textHeight=musicsur->h;
                            SDL_Rect textRect={1105,Y,textWidth,textHeight};
                            SDL_RenderCopy(renderer,music2,NULL,&textRect);
                            SDL_DestroyTexture(music2);
                            SDL_FreeSurface(musicsur);
                            Y+=textHeight+10;
                        }
                        SDL_RenderPresent(renderer);
                        SDL_Event e2;
                        while(SDL_PollEvent(&e2))
                        {
                            int Xmouse2,Ymouse2;
                            if(e2.type==SDL_MOUSEBUTTONDOWN)
                            {
                                SDL_GetGlobalMouseState(&Xmouse2,&Ymouse2);
                                cout<<Xmouse2<<" "<<Ymouse2<<endl;
                                //Nevermore
                                if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=265 && Ymouse2>=240)
                                {
                                    musicpath="C:/MINE/codes/Uni/PROJECT_BP_AA/musics/Denis Stelmakh - Nevermore.mp3";
                                    musicplayer(musicpath,music,muted);
                                    sidequit=true;
                                }
                                //La foule
                                else if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=295 && Ymouse2>=245)
                                {
                                    musicpath="C:/MINE/codes/Uni/PROJECT_BP_AA/musics/Pianaura - La Foule (La Monde) (Piano Version).mp3";
                                    musicplayer(musicpath,music,muted);
                                    sidequit=true;
                                }
                                //Idea 10
                                else if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2<=335 && Ymouse2>=235)
                                {
                                    musicpath="C:/MINE/codes/Uni/PROJECT_BP_AA/musics/Gibran Alcocer - Idea 10.mp3";
                                    musicplayer(musicpath,music,muted);
                                    sidequit=true;
                                }
                                //mute
                                else if(Xmouse2<=1300 && Xmouse2>=1100 && Ymouse2>=355 && Ymouse2<=375)
                                {
                                    if(muted)
                                        muted=false;
                                    else
                                        muted=true;
                                    musicplayer(musicpath,music,muted);
                                    sidequit=true;
                                }
                                else
                                    sidequit=true;
                            }
                        }
                    }
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer,mainbox.r,mainbox.g,mainbox.b,mainbox.a);
        SDL_RenderClear(renderer);
        boxRGBA(renderer,0,0,300,1000,leftbox.r,leftbox.g,leftbox.b,leftbox.a);
        boxRGBA(renderer,1300,0,1600,1000,rightbox.r,rightbox.g,rightbox.b,rightbox.a);
        boxRGBA(renderer,0,800,1600,1000,IFbox.r,IFbox.g,IFbox.b,IFbox.a);

        // Render text
        int y = -scrollOffset+5; // Start rendering based on the scroll offset

        //IFbox
        int Yerr=800,Xerr=20;
        for(size_t i=0;i<errors.size();i++)
        {
            if(errors[i].empty())
                errors[i]=" ";
            SDL_Surface *err=TTF_RenderText_Blended(font,errors[i].c_str(),errColor);
            SDL_Texture* errT=SDL_CreateTextureFromSurface(renderer,err);
            int textWidth=(err->w)*6/10;
            int textHeight=(err->h)*6/10;
            SDL_Rect errRect={Xerr,Yerr,textWidth,textHeight};
            SDL_FreeSurface(err);
            SDL_RenderCopy(renderer,errT,NULL,&errRect);
            SDL_DestroyTexture(errT);
            Yerr+=16;
        }
        int YIF=800,XIF=820;
        for(size_t i=0;i<IFtext.size();i++)
        {
            if(IFtext[i].empty())
                IFtext[i]=" ";
            SDL_Surface* IFsur=TTF_RenderText_Blended(font,IFtext[i].c_str(),IFbox2);
            SDL_Texture* IFtex=SDL_CreateTextureFromSurface(renderer,IFsur);
            int textWidth=(IFsur->w);
            int textHeight=(IFsur->h);
            SDL_Rect IFRect={XIF,YIF,textWidth,textHeight};
            SDL_FreeSurface(IFsur);
            SDL_RenderCopy(renderer,IFtex,NULL,&IFRect);
            SDL_DestroyTexture(IFtex);
            YIF+=24;
        }

        //rightbox texts
        int Yright=5;
        for(size_t i=0;i<rightboxB.size();i++)
        {
            SDL_Surface* RightboxText=TTF_RenderText_Blended(font,rightboxB[i].c_str(),textColor);
            SDL_Texture* Text=SDL_CreateTextureFromSurface(renderer,RightboxText);
            int textWidth=RightboxText->w;
            int textHeight=RightboxText->h;
            SDL_Rect rightboxrect={1320,Yright,textWidth,textHeight};
            SDL_FreeSurface(RightboxText);
            SDL_RenderCopy(renderer,Text,NULL,&rightboxrect);
            SDL_DestroyTexture(Text);
            Yright+=textHeight+10;
        }

        //leftbox texts
        string leftfilename=FileName+":";
        SDL_Surface* leftfilesur=TTF_RenderText_Blended(font,leftfilename.c_str(),textColor);
        SDL_Texture* leftfiletxt=SDL_CreateTextureFromSurface(renderer,leftfilesur);
        int textWidth2=leftfilesur->w;
        int textHeight2=leftfilesur->h;
        SDL_Rect leftboxrect={2,5,textWidth2,textHeight2};
        SDL_FreeSurface(leftfilesur);
        SDL_RenderCopy(renderer,leftfiletxt,NULL,&leftboxrect);
        SDL_DestroyTexture(leftfiletxt);
        int Yleft=35;
        for(size_t i=0;i<filenames.size();i++)
        {
            SDL_Surface* leftboxsur=TTF_RenderText_Blended(font,filenames[i].c_str(),textColor);
            SDL_Texture* leftboxtxt=SDL_CreateTextureFromSurface(renderer,leftboxsur);
            int textWidth=leftboxsur->w*8/10;
            int textHeight=leftboxsur->h*8/10;
            SDL_Rect leftboxrect2={12,Yleft,textWidth,textHeight};
            SDL_FreeSurface(leftboxsur);
            SDL_RenderCopy(renderer,leftboxtxt,NULL,&leftboxrect2);
            SDL_DestroyTexture(leftboxtxt);
            Yleft+=textHeight+10;
        }

        y=-scrollOffset+5;

        //main text
        for (size_t i = 0; i < lines.size(); ++i) {
            if (y + LINE_HEIGHT > 0 && y < 800) { // Render only visible lines
                if (lines[i].empty())
                    lines[i] = " ";// Show cursor on the current line
                int x=300;
                int temp=lines[i].size();
                for(int p=0;p<lines[i].size();p++)
                {
                    if(lines[i][p]=='/' && lines[i][p-1]=='/')
                    {
                        temp=p-1;
                        break;
                    }
                }
                vector<pair<int,SDL_Color>> highlights=GetWordColor(lines[i],darkmode);
                for(size_t j=0;j<lines[i].size();j++)
                {
                    SDL_Color charColor=textColor;
                    if(j>=temp)
                    {
                        charColor=commentColor;
                    }
                    else
                    {
                        for(const auto &highlight:highlights)
                        {
                            if(highlight.first==j)
                            {
                                charColor=highlight.second;
                                break;
                            }
                        }
                    }
                    string charString(1,lines[i][j]);
                    SDL_Surface* textSurface = TTF_RenderText_Blended(font,charString.c_str(),charColor);
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    int textWidth = textSurface->w*96/100;
                    int textHeight = textSurface->h*96/100;
                    SDL_Rect renderQuad = {x, y, textWidth, textHeight};

                    SDL_FreeSurface(textSurface);

                    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
                    SDL_DestroyTexture(textTexture);
                    x+=textWidth+1;
                }
                // Render cursor if this is the current line
                if (i == currentLine &&  cursorVisible) {
                    int cursorX = 0;
                    if (cursorPos > 0) {
                        TTF_SizeText(font, lines[i].substr(0, cursorPos).c_str(), &cursorX, nullptr);
                    }
                    cursorX += 1; // Add padding for the left margin
                    SDL_SetRenderDrawColor(renderer,cursor.r,cursor.g,cursor.b,cursor.a);
                    SDL_RenderDrawLine(renderer, cursorX+300, y, cursorX+300, y + LINE_HEIGHT);
                }
            }
            y += LINE_HEIGHT; // Move to the next line
        }

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Clean up
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return 0;
}


// Ensure the current line is visible when adding new lines or moving the cursor
void ensureLastLineVisible(int currentLine, int &scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines) {
    int cursorY = currentLine * LINE_HEIGHT - scrollOffset;
    if (cursorY < 0) {
        // Scroll up
        scrollOffset = currentLine * LINE_HEIGHT;
    } else if (cursorY + LINE_HEIGHT > SCREEN_HEIGHT) {
        // Scroll down
        scrollOffset = (currentLine + 1) * LINE_HEIGHT - SCREEN_HEIGHT;
    }

    // Ensure last line is always visible
    int contentHeight = totalLines * LINE_HEIGHT;
    if (contentHeight > SCREEN_HEIGHT) {
        scrollOffset = min(scrollOffset, contentHeight - SCREEN_HEIGHT);
    } else {
        scrollOffset = 0; // No scrolling needed if content fits
    }
}

// List of correct words
const std::vector<std::string> correctWords =
{
    "int", "double", "false", "else", "while", "return", "cout", "cin", "main", "bool","void","true","break"
};
const std::vector<std::string> mainWords =
{
    "int", "double", "false", "else", "while", "return", "cout", "cin", "main", "bool","void","true","std","include","for","endl","iostream","cmath"
};


//handle text colors
vector<pair<int,SDL_Color>> GetWordColor(string &line,bool darkmode)
{
    vector<pair<int,SDL_Color>> vec;
    if(!line.empty())
    {
        if(darkmode)
        {
            //regex for keywords
            regex patterntype(R"(\b(int|double|float|char|bool)\b)");
            sregex_iterator currentP(line.begin(), line.end(), patterntype);
            sregex_iterator lastP;
            while(currentP!=lastP)
            {
                smatch match= *currentP;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{224,128,108,255});
                ++currentP;
            }
            regex keywords(R"(\b(for|while|class|struct|if|else)\b)");
            sregex_iterator currentK(line.begin(), line.end(), keywords);
            sregex_iterator lastK;
            while(currentK!=lastK)
            {
                smatch match= *currentK;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{198,120,221,255});
                ++currentK;
            }
            regex par("(\\(|\\)|\\[|\\]|\\{|\\})");
            sregex_iterator currentPar(line.begin(), line.end(), par);
            sregex_iterator lastPar;
            while(currentPar!=lastPar)
            {
                smatch match= *currentPar;
                int start=match.position(0);
                int end=start;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{171,178,191,255});
                ++currentPar;
            }
            regex preProcess(R"(\#(include|ifdef|define))");
            sregex_iterator currentPre(line.begin(), line.end(), preProcess);
            sregex_iterator lastPre;
            while(currentPre!=lastPre)
            {
                smatch match= *currentPre;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{86,182,194,255});
                ++currentPre;
            }
            //regex for operators
            regex operators(R"(\+|\-|\*|\/)");
            sregex_iterator currentO(line.begin(), line.end(), operators);
            sregex_iterator lastO;
            while(currentO!=lastO)
            {
                smatch match= *currentO;
                int start=match.position(0);
                int end=start;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{213,94,0,255});
                ++currentO;
            }
            //regex for strings
            regex strings(R"("[^"\\]*(\\.[^"\\]*)*")");
            sregex_iterator currentS(line.begin(), line.end(), strings);
            sregex_iterator lastS;
            while(currentS!=lastS)
            {
                smatch match= *currentS;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{152,195,121,255});
                ++currentS;
            }
            //regex for char
            regex chars(R"('[^'\\]*(\\.[^'\\]*)*')");
            sregex_iterator currentC(line.begin(), line.end(), chars);
            sregex_iterator lastC;
            while(currentC!=lastC)
            {
                smatch match= *currentC;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{152,195,201,255});
                ++currentC;
            }
            //regex for numbers
            regex numbers(R"(\b\d+(\.\d+)?\b)");
            sregex_iterator currentN(line.begin(), line.end(), numbers);
            sregex_iterator lastN;
            while(currentN!=lastN)
            {
                smatch match= *currentN;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{209,154,102,255});
                ++currentN;
            }
            //regex for variable names
            regex variables(R"(\b(int|double|float|char|bool)\s+([a-zA-Z_]\w*))");
            sregex_iterator currentV(line.begin(), line.end(), variables);
            sregex_iterator lastV;
            while(currentV!=lastV)
            {
                smatch match= *currentV;
                int start=match.position(2);
                int end=start+match.length(2)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{229,192,123,255});
                ++currentV;
            }
        }
        else
        {
                        //regex for keywords
            regex patterntype(R"(\b(int|double|float|char|bool)\b)");
            sregex_iterator currentP(line.begin(), line.end(), patterntype);
            sregex_iterator lastP;
            while(currentP!=lastP)
            {
                smatch match= *currentP;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{0,128,128,255});
                ++currentP;
            }
            regex keywords(R"(\b(for|while|class|struct|if|else)\b)");
            sregex_iterator currentK(line.begin(), line.end(), keywords);
            sregex_iterator lastK;
            while(currentK!=lastK)
            {
                smatch match= *currentK;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{0,51,102,255});
                ++currentK;
            }
            regex par("(\\(|\\)|\\[|\\]|\\{|\\})");
            sregex_iterator currentPar(line.begin(), line.end(), par);
            sregex_iterator lastPar;
            while(currentPar!=lastPar)
            {
                smatch match= *currentPar;
                int start=match.position(0);
                int end=start;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{184,134,11,255});
                ++currentPar;
            }
            regex preProcess(R"(\#(include|ifdef|define))");
            sregex_iterator currentPre(line.begin(), line.end(), preProcess);
            sregex_iterator lastPre;
            while(currentPre!=lastPre)
            {
                smatch match= *currentPre;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{128,0,0,255});
                ++currentPre;
            }
            //regex for operators
            regex operators(R"(\+|\-|\*|\/)");
            sregex_iterator currentO(line.begin(), line.end(), operators);
            sregex_iterator lastO;
            while(currentO!=lastO)
            {
                smatch match= *currentO;
                int start=match.position(0);
                int end=start;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{128,0,0,255});
                ++currentO;
            }
            //regex for strings
            regex strings(R"("[^"\\]*(\\.[^"\\]*)*")");
            sregex_iterator currentS(line.begin(), line.end(), strings);
            sregex_iterator lastS;
            while(currentS!=lastS)
            {
                smatch match= *currentS;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{0,100,0,255});
                ++currentS;
            }
            //regex for char
            regex chars(R"('[^'\\]*(\\.[^'\\]*)*')");
            sregex_iterator currentC(line.begin(), line.end(), chars);
            sregex_iterator lastC;
            while(currentC!=lastC)
            {
                smatch match= *currentC;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{0,100,0,255});
                ++currentC;
            }
            //regex for numbers
            regex numbers(R"(\b\d+(\.\d+)?\b)");
            sregex_iterator currentN(line.begin(), line.end(), numbers);
            sregex_iterator lastN;
            while(currentN!=lastN)
            {
                smatch match= *currentN;
                int start=match.position(0);
                int end=start+match.length(0)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{128,0,128,255});
                ++currentN;
            }
            //regex for variable names
            regex variables(R"(\b(int|double|float|char|bool)\s+([a-zA-Z_]\w*))");
            sregex_iterator currentV(line.begin(), line.end(), variables);
            sregex_iterator lastV;
            while(currentV!=lastV)
            {
                smatch match= *currentV;
                int start=match.position(2);
                int end=start+match.length(2)-1;
                for(int i=start;i<=end;i++)
                    vec.emplace_back(i,SDL_Color{139,0,0,255});
                ++currentV;
            }
        }
    }
    return vec;
}

//handle existence of the libraries
void ErrorForInclude(vector<string>& errors, const vector<string>& lines)
{
    bool bitsfind=false;
    bool iostfind=false,iosterr=false;
    bool mathfind=false,matherr=false;
    string bits="(#include)( )+(<bits/stdc\\+\\+.h>)";
    string iost="(#include)( )+(<iostream>)";
    string math="(#include)( )+(<cmath>)";
    vector<string> iostlib={"cout","cin","endl","getline"};
    vector<string> mathlib={"sqrt\\(.*\\)","pow\\(.*\\)","sin\\(.*\\)","cos\\(.*\\)","tan\\(.*\\)","abs\\(.*\\)","exp\\(.*\\)","log\\(.*\\)","log10\\(.*\\)","floor\\(.*\\)","ceil\\(.*\\)"};
    regex re(bits);
    regex re2(iost);
    regex re3(math);
    for(int i=0;i<lines.size();i++)
    {
        if(regex_search(lines[i],re))
            bitsfind=true;
        if(regex_search(lines[i],re2))
            iostfind=true;
        if(regex_search(lines[i],re3))
            mathfind=true;
    }
    string temp;
    if(!bitsfind && !iostfind)
    {
        for(int j=0;j<iostlib.size();j++)
        {
            regex reg(iostlib[j]);
            for(int i=0;i<lines.size();i++)
            {
                if(regex_search(lines[i],reg))
                {
                    temp=temp+to_string(i+1)+"   ";
                    iosterr=true;
                }
            }
        }
    }
    string temp2;
    if(!bitsfind && !mathfind)
    {
        for(int j=0;j<mathlib.size();j++)
        {
            regex reg(mathlib[j]);
            for(int i=0;i<lines.size();i++)
            {
                if(regex_search(lines[i],reg))
                {
                    temp2=temp2+to_string(i+1)+"   ";
                    matherr=true;
                }
            }
        }
    }
    if(iosterr)
        errors.push_back("iostream Library Not Found:"+temp);
    if(matherr)
        errors.push_back("cmath Library Not Found:"+temp2);
}

//Handle the debugging
void BuiltDebugger(vector<string>& errors, const vector<string>& lines)
{
    // ()
    for(int i=0;i<lines.size();i++)
    {
        int PopenNUM=0;
        int PcloseNUM=0;
        for(int ii=0;ii<lines[i].length();ii++)
        {
            if(lines[i][ii]=='(')
                PopenNUM++;
            if(lines[i][ii]==')')
                PcloseNUM++;
        }
        if(PopenNUM!=PcloseNUM)
            errors.push_back("The Parentheses Numbers Doesnt Match:"+to_string(i+1));
    }
    // {}
    int BopenNUM=0;
    int BcloseNUM=0;
    for(int i=0;i<lines.size();i++)
    {
        for(int ii=0;ii<lines[i].length();ii++)
        {
            if(lines[i][ii]=='{')
                BopenNUM++;
            if(lines[i][ii]=='}')
                BcloseNUM++;
        }
    }
    if(BopenNUM>BcloseNUM)
        errors.push_back("The Number of Curly Brackets Dont Match:You Have "+to_string(BopenNUM-BcloseNUM)+" Opened Brackets More");
    else if(BopenNUM<BcloseNUM)
        errors.push_back("The Number of Curly Brackets Dont Match:You Have "+to_string(BcloseNUM-BopenNUM)+" Closed Brackets More");
    // []
    int B2openNUM=0;
    int B2closeNUM=0;
    for(int i=0;i<lines.size();i++)
    {
        for(int ii=0;ii<lines[i].length();ii++)
        {
            if(lines[i][ii]=='[')
                B2openNUM++;
            if(lines[i][ii]==']')
                B2closeNUM++;
        }
    }
    if(B2openNUM>B2closeNUM)
        errors.push_back("The Number of Brackets Dont Match:You Have "+to_string(B2openNUM-B2closeNUM)+" Opened Brackets More");
    else if(B2openNUM<B2closeNUM)
        errors.push_back("The Number of Brackets Dont Match:You Have "+to_string(B2closeNUM-B2openNUM)+" Closed Brackets More");
    // ""
    for(int i=0;i<lines.size();i++)
    {
        int Ccount=0;
        for(int ii=0;ii<lines[i].length();ii++)
        {
            if(lines[i][ii]=='"')
                Ccount++;
        }
        if(Ccount%2!=0)
            errors.push_back("You Did Not Use \" Correctly: "+to_string(i+1));
        regex re("(.)*(string)( )*(.*)\\=");
        int C2count=0;
        if(regex_search(lines[i],re))
        {
            for(int ii=0;ii<lines[i].length();ii++)
            {
                if(lines[i][ii]=='"')
                    C2count++;
            }
            if(C2count==0)
                errors.push_back("You Did Not Use \" Correctly: "+to_string(i+1));
        }
    }
    // ''
    for(int i=0;i<lines.size();i++)
    {
        int Acount=0;
        for(int ii=0;ii<lines[i].length();ii++)
        {
            if(lines[i][ii]=='\'')
                Acount++;
        }
        if(Acount%2!=0)
            errors.push_back("You Did Not Use ' Correctly: "+to_string(i+1));
        regex re2("(.)*(char)( )*(.*)\\=");
        int A2count=0;
        if(regex_search(lines[i],re2))
        {
            for(int ii=0;ii<lines[i].length();ii++)
            {
                if(lines[i][ii]=='\'')
                    A2count++;
            }
            if(A2count==0)
                errors.push_back("You Did Not Use ' Correctly: "+to_string(i+1));
        }
    }
    // */ /*
    bool opened=false;
    bool closed=false;
    for(int i=0;i<lines.size();i++)
    {
        for(int ii=0;ii<lines[i].length();ii++)
        {
            if(lines[i][ii]=='*' && lines[i][ii+1]=='/')
                closed=true;
            if(lines[i][ii]=='/' && lines[i][ii+1]=='*')
                opened=true;
        }
    }
    if((closed && !opened) || (!closed && opened))
        errors.push_back("The Comments Were Not Initialized Correctly");
    // ;
    regex re("( )*");
    regex re2("( )*(#include)( )+(<)(.*)(>)");
    regex re3("( )*(int)( )+(main)(\\()(.*)(\\))( )*");
    regex re4("( )*(\\{)( )*");
    regex re5("( )*(\\})( )*");
    regex re6(R"(\s*for\s*\([^;]*;[^;]*;[^)]*\)\s*)");
    regex re7("( )*(while)( )*(\\()(.*)(\\))( )*");
    regex re8("( )*(.*)(;)( )*");
    regex re9("(.*)(\\/)(\\/)(.*)");
    bool aftermain=false;
    for(int i=0;i<lines.size();i++)
    {
        if(regex_match(lines[i],re3))
            aftermain=true;
        if(aftermain)
        {
            if(lines[i].empty() || regex_match(lines[i],re) || regex_match(lines[i],re2) || regex_match(lines[i],re3) || regex_match(lines[i],re4) || regex_match(lines[i],re5)
                || regex_match(lines[i],re6) || regex_match(lines[i],re7) || regex_match(lines[i],re8) || regex_match(lines[i],re9))
                bool t=false;
            else
            {
                errors.push_back("; Not Found: "+to_string(i+1));
            }
        }
    }

    //keywords for names
    vector<string> variableTypes={"int","bool","string","double","float","char"};
    bool aftermain2=false;
    for(int i=0;i<lines.size();i++)
    {
        if(aftermain2)
        {
            for(int ii=0;ii<variableTypes.size();ii++)
            {
                for(int iii=0;iii<correctWords.size();iii++)
                {
                    regex re("(.*)"+variableTypes[ii]+"( )*"+correctWords[iii]+"(.*)");
                    if(regex_match(lines[i],re))
                        errors.push_back("You Can't Use Keywords Az Names: "+to_string(i+1));
                }
            }
        }
        regex RE("( )*(int)( )+(main)(\\()(\\))( )*");
        if(regex_match(lines[i],RE))
            aftermain2=true;
    }

    //==+
    for(int i=0;i<lines.size();i++)
    {
        regex reg8(R"([+=-]{3,10})");
        if(regex_search(lines[i],reg8))
            errors.push_back("Invalid Operator Used In Line: "+to_string(i+1));
    }

    //switch case
    int iphone16promax=0;
    for(int i=0;i<lines.size();i++)
    {
        regex reg(R"(\s*(if)\s*(\()(.*)(\))(.*))");
        regex reg2(R"(\s*(else)\s+(if)(\()(.*)(\))(.*))");
        cout<<regex_search(lines[i],reg)<<" "<<regex_match(lines[i],reg2)<<endl;
        if(regex_match(lines[i],reg))
            iphone16promax=0;
        if(regex_match(lines[i],reg2))
            iphone16promax++;
        if(iphone16promax==2)
        {
            errors.push_back("You Can Use Switch-Case Instead :) : "+to_string(i+1));
            break;
        }
    }
}

std::string getFunctionName(const std::string& declaration)
{
    std::regex functionNameRegex(R"(\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(.*\))");
    std::smatch match;
    if (std::regex_search(declaration, match, functionNameRegex))
    {
        return match[1].str();
    }
    return "";
}

int countParameters(const std::string& declaration)
{
    // Match the parameter list within the parentheses
    std::regex paramRegex(R"(\(([^)]*)\))");
    std::smatch match;

    if (std::regex_search(declaration, match, paramRegex))
    {
        std::string params = match[1].str();

        // Split the parameters by commas
        std::regex commaRegex(R"(\s*,\s*)");
        std::sregex_token_iterator it(params.begin(), params.end(), commaRegex, -1);
        std::sregex_token_iterator end;

        // Count the number of non-empty tokens
        std::vector<std::string> paramList;
        while (it != end)
        {
            if (!it->str().empty())
            {
                paramList.push_back(*it);
            }
            ++it;
        }
        return paramList.size();
    }

    return 0;
}

void functionvarinum(const vector<string>& lines,vector<string> &errors)
{
    vector<pair<string,int>> pp;
    regex reg("( )*(int)( )+(main)( )*(\\()(\\))( )*");
    regex reg2(R"((\w+\s+)+(\w+::)?(\w+)\s*\([^\)]*\)\s*;?)");
    bool beforemain=true;
    for(int i=0;i<lines.size();i++)
    {
        if(regex_match(lines[i],reg))
            beforemain=false;
        if(beforemain)
        {
            if(regex_match(lines[i],reg2))
            {
                string name=getFunctionName(lines[i]);
                int count=countParameters (lines[i]);
                pp.push_back(make_pair(name,count));
            }
        }
    }
    beforemain=true;
    for(int i=0;i<lines.size();i++)
    {
        if(regex_match(lines[i],reg))
            beforemain=false;
        for(int ii=0;ii<pp.size();ii++)
        {
            std::regex regF("\\s*"+pp[ii].first+"\\s*\\(([^)]*)\\)(;)");
            if(!beforemain)
            {
                if(regex_search(lines[i],regF))
                {
                    if(countParameters(lines[i])!=pp[ii].second)
                        errors.push_back("parameters not matching: "+to_string(i+1));
                }
            }
        }
    }
}
void checkVariableDeclarations(const vector<string>& lines,vector<string> &errors)
{
    std::unordered_set<std::string> declaredVariables;
    vector<string> temp;
    std::regex declarationRegex(R"(\s*(int|float|double|char|bool|std::string|string|void)\s+([a-zA-Z_]\w*)(.*))");
    std::regex usageRegex(R"(\b([a-zA-Z_]\w*)\b)");
    regex reg("( )*(int)( )+(main)( )*(\\()(\\))( )*");
    std::string line;
    int lineNumber = 0;
    bool aftermain=false;
    while (lineNumber<lines.size())
    {
        line=lines[lineNumber];
        lineNumber++;

        if(regex_match(line,reg))
            aftermain=true;

        // Check for variable declarations
        std::smatch match;
        if (std::regex_search(line, match, declarationRegex)) {
            declaredVariables.insert(match[2].str());
            temp.push_back(match[2].str());
        }

        if (aftermain)
        {
            // Check for variable usage
            auto wordsBegin = std::sregex_iterator(line.begin(), line.end(), usageRegex);
            auto wordsEnd = std::sregex_iterator();

            for (std::sregex_iterator it = wordsBegin; it != wordsEnd; ++it)
            {
                std::string word = it->str();
                bool found=false;
                for(int i=0;i<declaredVariables.size();i++)
                {
                    if(word==temp[i])
                        found=true;
                }
                for(int i=0;i<mainWords.size();i++)
                {
                    if(word==mainWords[i])
                        found=true;
                }
                if(!found)
                {
                    if (declaredVariables.find(word) == declaredVariables.end())
                    {
                        errors.push_back("Potentially undeclared variable at line "+to_string(lineNumber)+": "+word);
                    }
                }
            }
        }
    }
}

// Function to calculate Levenshtein distance
int levenshteinDistance(const std::string &s1, const std::string &s2)
{
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));
    d[0][0] = 0;
    for (unsigned int i = 1; i <= len1; ++i)
        d[i][0] = i;
    for (unsigned int i = 1; i <= len2; ++i)
        d[0][i] = i;
    for (unsigned int i = 1; i <= len1; ++i)
        for (unsigned int j = 1; j <= len2; ++j)
            d[i][j] = std::min({d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)});
    return d[len1][len2];
}

void findSimilarWords(const vector<string> &lines,vector<string> &errors, int threshold)
{
    std::string line;
    int lineNumber = 0;
    std::regex wordRegex(R"(\b[a-zA-Z_][a-zA-Z0-9_]*\b)");
    while (lineNumber<lines.size())
    {
        line=lines[lineNumber];
        lineNumber++;
        std::sregex_iterator wordsBegin(line.begin(), line.end(), wordRegex);
        std::sregex_iterator wordsEnd;

        for (std::sregex_iterator it = wordsBegin; it != wordsEnd; ++it) {
            std::string word = it->str();
            bool found=false;
            for(const auto& correctWord : correctWords)
            {
                if(correctWord==word)
                {
                    found=true;
                    break;
                }
            }
            if(!found)
            {
                for (const auto& correctWord : correctWords) {
                    if(word.length()>2)
                    {
                        if (levenshteinDistance(word, correctWord) <= threshold && word != correctWord)
                        {
                            errors.push_back("Potentially incorrect word at line "+to_string(lineNumber)+": "+word+"?"+correctWord);
                        }
                    }
                }
            }
        }
    }
}
//Handle Autocomplete
void autocomplete(vector<string>& lines, int &cursorPos, int currentLine,bool tab)
{
    if(!tab)
    {
        if(lines[currentLine][cursorPos]=='(')
            lines[currentLine].insert(cursorPos+1, ")");
        if(lines[currentLine][cursorPos]=='[')
            lines[currentLine].insert(cursorPos+1, "]");
        if(lines[currentLine][cursorPos]=='{')
            lines[currentLine].insert(cursorPos+1, "}");
        if(lines[currentLine][cursorPos]=='\"')
            lines[currentLine].insert(cursorPos+1, "\"");
        if(lines[currentLine][cursorPos]=='\'')
            lines[currentLine].insert(cursorPos+1, "\'");
    }
    else
    {
        string check=lines[currentLine].substr(0,cursorPos);
        regex regIF("( )*if"),regELS("( )*els"),regWHI("( )*whi"),regSWI("( )*swi"),regCA("( )*ca"),regDEF("( )*def"),regSCA("( )*sca"),regPRI("( )*pri");
        regex regSTR("( )*str"),regAB("( )*ab"),regSQ("( )*sq"),regMA("( )*int( )*ma"),regIN("( )*in"),regFLO("( )*flo"),regDOU("( )*dou"),regLO("( )*lo"),regBO("( )*bo");
        if(regex_match(check,regIF))
        {
            lines[currentLine].insert(cursorPos,"()");
            cursorPos++;
            vector<string>::iterator it=lines.begin()+currentLine+1;
            lines.insert(it,"{");
            it=lines.begin()+currentLine+2;
            lines.insert(it,"");
            it=lines.begin()+currentLine+3;
            lines.insert(it,"}");
        }
        if(regex_match(check,regELS))
        {
            lines[currentLine].insert(cursorPos,"e");
            cursorPos++;
            vector<string>::iterator it=lines.begin()+currentLine+1;
            lines.insert(it,"{");
            it=lines.begin()+currentLine+2;
            lines.insert(it,"");
            it=lines.begin()+currentLine+3;
            lines.insert(it,"}");
        }
        if(regex_match(check,regWHI))
        {
            lines[currentLine].insert(cursorPos,"le()");
            cursorPos+=3;
            vector<string>::iterator it=lines.begin()+currentLine+1;
            lines.insert(it,"{");
            it=lines.begin()+currentLine+2;
            lines.insert(it,"");
            it=lines.begin()+currentLine+3;
            lines.insert(it,"}");
        }
        if(regex_match(check,regSWI))
        {
            lines[currentLine].insert(cursorPos,"tch()");
            cursorPos+=4;
            vector<string>::iterator it=lines.begin()+currentLine+1;
            lines.insert(it,"{");
            it=lines.begin()+currentLine+2;
            lines.insert(it,"");
            it=lines.begin()+currentLine+3;
            lines.insert(it,"}");
        }
        if(regex_match(check,regCA))
        {
            lines[currentLine].insert(cursorPos,"se :");
            cursorPos+=3;
        }
        if(regex_match(check,regDEF))
        {
            lines[currentLine].insert(cursorPos,"ault;");
            cursorPos+=5;
        }
        if(regex_match(check,regSCA))
        {
            lines[currentLine].insert(cursorPos,"nf()");
            cursorPos+=3;
        }
        if(regex_match(check,regPRI))
        {
            lines[currentLine].insert(cursorPos,"ntf()");
            cursorPos+=4;
        }
        if(regex_match(check,regSTR))
        {
            lines[currentLine].insert(cursorPos,"len()");
            cursorPos+=4;
        }
        if(regex_match(check,regAB))
        {
            lines[currentLine].insert(cursorPos,"s()");
            cursorPos+=2;
        }
        if(regex_match(check,regSQ))
        {
            lines[currentLine].insert(cursorPos,"rt()");
            cursorPos+=3;
        }
        if(regex_match(check,regMA))
        {
            lines[currentLine].insert(cursorPos,"in()");
            cursorPos+=3;
            vector<string>::iterator it=lines.begin()+currentLine+1;
            lines.insert(it,"{");
            it=lines.begin()+currentLine+2;
            lines.insert(it,"");
            it=lines.begin()+currentLine+3;
            lines.insert(it,"}");
        }
        if(regex_match(check,regIN))
        {
            lines[currentLine].insert(cursorPos,"t ");
            cursorPos+=2;
        }
        if(regex_match(check,regFLO))
        {
            lines[currentLine].insert(cursorPos,"at ");
            cursorPos+=3;
        }
        if(regex_match(check,regDOU))
        {
            lines[currentLine].insert(cursorPos,"ble ");
            cursorPos+=4;
        }
        if(regex_match(check,regLO))
        {
            lines[currentLine].insert(cursorPos,"ng ");
            cursorPos+=3;
        }
        if(regex_match(check,regBO))
        {
            lines[currentLine].insert(cursorPos,"ol ");
            cursorPos+=3;
        }
    }
}

//Handle saving
string SaveOP(SDL_Renderer* renderer,vector<string> &lines,string existingfile,bool fileexists)
{
    int Xmouse,Ymouse;
    int cursorPos=1;
    bool sidequit=false,quit=false;
    string name=" "; //name of the current file
    string filename;
    SDL_Event e;
    SDL_Rect savepic={500,450,600,300};
    SDL_Color color={255,200,255,255};
    TTF_Font* font = TTF_OpenFont("Monaco.TTF", 24);
    if(!fileexists)
    {
        while(!sidequit)
        {
            SDL_Surface *sursavepic=IMG_Load("C:/MINE/codes/Uni/PROJECT_BP_AA/pic/savepic.png");
            SDL_Texture *texsavepic=SDL_CreateTextureFromSurface(renderer,sursavepic);
            SDL_FreeSurface(sursavepic);
            SDL_RenderCopy(renderer,texsavepic,NULL,&savepic);
            SDL_DestroyTexture(texsavepic);
            while(SDL_PollEvent(&e))
            {
                if(e.type==SDL_MOUSEBUTTONDOWN)
                {
                    SDL_GetGlobalMouseState(&Xmouse,&Ymouse);
                    cout<<Xmouse<<" "<<Ymouse<<endl;
                    if(Xmouse<=1125 && Xmouse>=1100 && Ymouse<=490 && Ymouse>=465)
                    {
                        quit=true;
                        sidequit=true;
                    }
                }
                if(e.type==SDL_TEXTINPUT)
                {
                    if(e.text.text)
                    {
                        name.insert(cursorPos,e.text.text);
                        cursorPos+=strlen(e.text.text);
                    }
                }
                if(e.type==SDL_KEYDOWN)
                {
                    if(e.key.keysym.sym==SDLK_BACKSPACE)
                    {
                        if(cursorPos>1)
                        {
                            name.erase(cursorPos-1,1);
                            cursorPos--;
                        }
                    }
                    if(e.key.keysym.sym==SDLK_RETURN)
                    {
                        if(name.size()>1)
                        {
                            sidequit=true;
                        }
                    }
                }
            }
            int X=520,Y=640;
            SDL_Surface *namesur=TTF_RenderText_Blended(font,name.c_str(),color);
            SDL_Texture *texnamesur=SDL_CreateTextureFromSurface(renderer,namesur);
            int textWidth=namesur->w;
            int textHeight=namesur->h;
            SDL_Rect textRect={X,Y,textWidth,textHeight};
            SDL_FreeSurface(namesur);
            SDL_RenderCopy(renderer,texnamesur,NULL,&textRect);
            SDL_DestroyTexture(texnamesur);
            SDL_RenderPresent(renderer);
        }
        filename="C:\\MINE\\project\\"+FileName+"\\"+name+".cpp";
    }
    else
        filename=existingfile;
    if(!quit)
    {
        ofstream outfile(filename);
        for(int i=0;i<lines.size();i++)
        {
            outfile<<lines[i]<<endl;
        }
        outfile.close();
        leftfilenames();
        return filename;
    }
    return "";
}

//get the file names for left box
void leftfilenames()
{
    filenames.clear();
    string path="C:/MINE/project";
    WIN32_FIND_DATA findFileData;
    HANDLE hFile=FindFirstFile((path+"\\*").c_str(),&findFileData);
    do
    {
        const string filename=findFileData.cFileName;
        if(filename!="." && filename!="..")
        {
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                FileName=filename;
        }
    }
    while(FindNextFile(hFile,&findFileData)!=0);
    path=path+"/"+FileName;
    WIN32_FIND_DATA findFileData2;
    HANDLE hFile2=FindFirstFile((path+"\\*").c_str(),&findFileData2);
    do
    {
        const string filename=findFileData2.cFileName;
        if(filename!="." && filename!="..")
        {
            if (findFileData2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                bool t=false;
            else
                filenames.push_back(filename);
        }
    }
    while(FindNextFile(hFile2,&findFileData2)!=0);
    if(filenames.empty())
        filenames.emplace_back("No Projects saved yet");
}

// Function to compile a C++ source file
bool compile(const std::string &filePath) {
    // Extract the filename from the full path
    size_t lastSlash = filePath.find_last_of("/\\");
    std::string filename = (lastSlash == std::string::npos) ? filePath : filePath.substr(lastSlash + 1);

    // Extract the directory
    std::string directory = (lastSlash == std::string::npos) ? "." : filePath.substr(0, lastSlash);

    std::string compileCommand;
#ifdef _WIN32
    compileCommand = "cd /d \"" + directory + "\" && g++ \"" + filename + "\" -o output";
#else
    compileCommand = "cd \"" + directory + "\" && g++ \"" + filename + "\" -o output";
#endif


    int result = std::system(compileCommand.c_str());
    return (result == 0);
}

// Function to run an executable
void run_in_another_window(const std::string &filePath) {
    // Extract the directory
    size_t lastSlash = filePath.find_last_of("/\\");
    std::string directory = (lastSlash == std::string::npos) ? "." : filePath.substr(0, lastSlash);

    std::string runCommand;
#ifdef _WIN32
    // Use quotes directly around the directory to handle spaces
    runCommand = "start cmd /c \"cd /d \"" + directory + "\" && output.exe && pause\"";
#elif __linux__
    runCommand = "x-terminal-emulator -e \"cd '" + directory + "' && ./output; read -n 1 -s -r -p 'Press any key to continue...'\"";
#elif __APPLE__
    runCommand = "osascript -e 'tell application \"Terminal\" to do script \"cd \\\"" + directory + "\\\" && ./output\"'";
#else
    std::cerr << "Unsupported operating system." << std::endl;
    return;
#endif
    // Execute the constructed command
    int result = std::system(runCommand.c_str());
    if (result != 0) {
        std::cerr << "Failed to run the program in a new terminal." << std::endl;
    }
}

//opens the selected file
void OpenFile(int num, vector<string>& lines)
{
    string path="C:/MINE/project/"+FileName;
    string name;
    WIN32_FIND_DATA findFileData;
   ..
    int N=0;
    do
    {
        const string filename=findFileData.cFileName;
        if(filename!="." && filename!="..")
        {
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                bool t=false;
            else
            {
                name=filename;
                N++;
            }
        }
    }
    while(FindNextFile(hFile,&findFileData)!=0 && N<num);
    lines.clear();
    ifstream inputFile;
    inputFile.open(path+"/"+name);
    string temp;
    while(getline(inputFile,temp))
        lines.push_back(temp);
    FILENAME=path+"/"+name;
}
void musicplayer(string musicpath,Mix_Music* music,bool muted)
{
    //music
    music=Mix_LoadMUS(musicpath.c_str());
    if(!muted)
    {
        Mix_PlayMusic(music,-1);
    }
    else
        Mix_PauseMusic();
}





