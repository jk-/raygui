/*******************************************************************************************
*
*   raw_importer - image raw importer
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 raylib technologies
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <string.h>             // Required for: strcpy()
#include <stdlib.h>             // Required for: atoi()

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------
static void ImportRAW();        // Button: ImportRAW logic

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Image RAW Importer");
    
    Texture2D texture = { 0 };

    // raw_importer: controls initialization
    //----------------------------------------------------------------------------------
    Vector2 windowOffset = { screenWidth/2 - 200/2, screenHeight/2 - 465/2 };

    bool importWindowActive = false;
    int widthValue = 0;
    int heightValue = 0;
    int pixelFormatActive = 0;
    const char *pixelFormatTextList[8] = { "CUSTOM", "GRAYSCALE", "GRAY ALPHA", "R5G6B5", "R8G8B8", "R5G5B5A1", "R4G4B4A4", "R8G8B8A8" };

    int channelsActive = 3;
    const char *channelsTextList[4] = { "1", "2", "3", "4" };
    int bitDepthActive = 0;
    const char *bitDepthTextList[3] = { "8", "16", "32" };
    int headerSizeValue = 0;
    //----------------------------------------------------------------------------------
    
    // Image file info
    int dataSize = 0;
    char fileName[32] = "\0";
    
    bool btnLoadPressed = false;
    
    bool imageLoaded = false;
    float imageScale = 1.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Check if a file is dropped
        if (IsFileDropped())
        {
            int fileCount = 0;
            char **droppedFiles = GetDroppedFiles(&fileCount);

            // Check file extensions for drag-and-drop
            if ((fileCount == 1) && IsFileExtension(droppedFiles[0], ".raw"))
            {
                FILE *imageFile = fopen(droppedFiles[0], "rb");
                fseek(imageFile, 0L, SEEK_END);
                dataSize = ftell(imageFile);
                fclose(imageFile);
                
                // NOTE: Returned string is just a pointer to droppedFiles[0],
                // we need to make a copy of that data somewhere else: fileName
                strcpy(fileName, GetFileName(droppedFiles[0]));
                
                // Try to guess possible raw values
                // Let's assume image is square, RGBA, 8 bit per channel
                widthValue = round(sqrt(dataSize/4));
                heightValue = widthValue;
                headerSizeValue = dataSize - widthValue*heightValue*4;

                importWindowActive = true;
            }

            ClearDroppedFiles();
        }
        
        // Check if load button has been pressed
        if (btnLoadPressed)
        {
            // Depending on channels and bit depth, select correct pixel format
            if ((widthValue != 0) && (heightValue != 0))
            {
                int format = -1;

                if (pixelFormatActive == 0)
                {
                    int channels = atoi(channelsTextList[channelsActive]);
                    int bpp = atoi(bitDepthTextList[bitDepthActive]);
            
                    // Select correct format depending on channels and bpp
                    if (bpp == 8)
                    {
                        if (channels == 1) format = UNCOMPRESSED_GRAYSCALE;
                        else if (channels == 2) format = UNCOMPRESSED_GRAY_ALPHA;
                        else if (channels == 3) format = UNCOMPRESSED_R8G8B8;
                        else if (channels == 4) format = UNCOMPRESSED_R8G8B8A8;
                    }
                    else if (bpp == 32)
                    {
                        if (channels == 1) format = UNCOMPRESSED_R32;
                        else if (channels == 2) TraceLog(LOG_WARNING, "Channel bit-depth not supported!");
                        else if (channels == 3) format = UNCOMPRESSED_R32G32B32;
                        else if (channels == 4) format = UNCOMPRESSED_R32G32B32A32;
                    }
                    else if (bpp == 16) TraceLog(LOG_WARNING, "Channel bit-depth not supported!");
                }
                else format = pixelFormatActive;
                
                if (format != -1)
                {
                    Image image = LoadImageRaw(fileName, widthValue, heightValue, format, headerSizeValue);
                    texture = LoadTextureFromImage(image);
                    UnloadImage(image);
                    
                    importWindowActive = false;
                    btnLoadPressed = false;
                    
                    if (texture.id > 0)
                    {
                        imageLoaded = true;
                        imageScale = (float)(screenHeight - 100)/texture.height;
                    }
                }
            }
        }
        
        if (imageLoaded) imageScale += (float)GetMouseWheelMove();   // Image scale control
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(style[DEFAULT_BACKGROUND_COLOR]));
            
            if (texture.id != 0) 
            {
                DrawTextureEx(texture, (Vector2){ screenWidth/2 - texture.width*imageScale/2, screenHeight/2 - texture.height*imageScale/2 }, 0, imageScale, WHITE);
                DrawText(FormatText("SCALE x%.0f", imageScale), 20, screenHeight - 40, 20, GetColor(style[DEFAULT_LINES_COLOR]));
            }
            else DrawText("drag & drop RAW image file", 320, 180, 10, GetColor(style[DEFAULT_LINES_COLOR]));

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            if (importWindowActive)
            {
                importWindowActive = !GuiWindowBox((Rectangle){ windowOffset.x + 0, windowOffset.y + 0, 200, 465 }, "Image RAW Import Options");
            
                GuiLabel((Rectangle){ windowOffset.x + 10, windowOffset.y + 30, 65, 20 }, "Import file:");
                GuiLabel((Rectangle){ windowOffset.x + 85, windowOffset.y + 30, 75, 20 }, fileName);
                GuiLabel((Rectangle){ windowOffset.x + 10, windowOffset.y + 50, 65, 20 }, "File size:");
                GuiLabel((Rectangle){ windowOffset.x + 85, windowOffset.y + 50, 75, 20 }, FormatText("%i bytes", dataSize));
                GuiGroupBox((Rectangle){ windowOffset.x + 10, windowOffset.y + 85, 180, 80 }, "Resolution");
                GuiLabel((Rectangle){ windowOffset.x + 20, windowOffset.y + 100, 33, 25 }, "Width:");
                widthValue = GuiValueBox((Rectangle){ windowOffset.x + 60, windowOffset.y + 100, 80, 25 }, widthValue, 100); 
                GuiLabel((Rectangle){ windowOffset.x + 145, windowOffset.y + 100, 30, 25 }, "pixels");
                GuiLabel((Rectangle){ windowOffset.x + 20, windowOffset.y + 130, 33, 25 }, "Height:");
                heightValue = GuiValueBox((Rectangle){ windowOffset.x + 60, windowOffset.y + 130, 80, 25 }, heightValue, 100); 
                GuiLabel((Rectangle){ windowOffset.x + 145, windowOffset.y + 130, 30, 25 }, "pixels");
                GuiGroupBox((Rectangle){ windowOffset.x + 10, windowOffset.y + 180, 180, 160 }, "Pixel Format");
                pixelFormatActive = GuiComboBox((Rectangle){ windowOffset.x + 20, windowOffset.y + 195, 160, 25 },  pixelFormatTextList, 8, pixelFormatActive);
                GuiLine((Rectangle){ windowOffset.x + 20, windowOffset.y + 220, 160, 20 }, 1);
                
                if (pixelFormatActive != 0) GuiDisable();
                GuiLabel((Rectangle){ windowOffset.x + 20, windowOffset.y + 235, 50, 20 }, "Channels:");
                channelsActive = GuiToggleGroup((Rectangle){ windowOffset.x + 20, windowOffset.y + 255, 159, 25 }, channelsTextList, 4, channelsActive);
                GuiLabel((Rectangle){ windowOffset.x + 20, windowOffset.y + 285, 50, 20 }, "Bit Depth:");
                bitDepthActive = GuiToggleGroup((Rectangle){ windowOffset.x + 20, windowOffset.y + 305, 159, 25 }, bitDepthTextList, 3, bitDepthActive);
                GuiEnable();
                
                GuiGroupBox((Rectangle){ windowOffset.x + 10, windowOffset.y + 355, 180, 50 }, "Header");
                GuiLabel((Rectangle){ windowOffset.x + 25, windowOffset.y + 370, 27, 25 }, "Size:");
                headerSizeValue = GuiValueBox((Rectangle){ windowOffset.x + 55, windowOffset.y + 370, 85, 25 }, headerSizeValue, 100); 
                GuiLabel((Rectangle){ windowOffset.x + 145, windowOffset.y + 370, 30, 25 }, "bytes");
                
                btnLoadPressed = GuiButton((Rectangle){ windowOffset.x + 10, windowOffset.y + 420, 180, 30 }, "Import RAW");
            }
            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    if (texture.id != 0) UnloadTexture(texture);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}