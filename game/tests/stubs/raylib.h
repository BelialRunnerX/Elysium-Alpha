// Intended function: imported tests implementation for raylib; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once
#include <cstddef>
struct Vector2 { float x{}, y{}; };
struct Vector3 { float x{}, y{}, z{}; };
struct Color { unsigned char r{},g{},b{},a{}; };
struct Camera3D { Vector3 position{}, target{}, up{}; float fovy{}; int projection{}; };
struct Mesh { int vertexCount{}, triangleCount{}; float* vertices{}; float* texcoords{}; float* texcoords2{}; float* normals{}; float* tangents{}; unsigned char* colors{}; unsigned short* indices{}; };
struct Model { int _stub{}; };
struct Texture2D { unsigned int id{}; int width{}, height{}, mipmaps{}, format{}; };
struct Image { void* data{}; int width{}, height{}, mipmaps{}, format{}; };
inline constexpr Color WHITE{255,255,255,255};
inline constexpr Color RAYWHITE{245,245,245,255};
inline constexpr Color GRAY{130,130,130,255};
inline constexpr int CAMERA_PERSPECTIVE=0;
inline constexpr int PIXELFORMAT_UNCOMPRESSED_R8G8B8A8=7;
inline constexpr unsigned FLAG_MSAA_4X_HINT=1u<<0, FLAG_VSYNC_HINT=1u<<1, FLAG_WINDOW_RESIZABLE=1u<<2;
enum { MOUSE_BUTTON_LEFT=0, MOUSE_BUTTON_RIGHT=1 };
enum {
 KEY_ZERO='0',KEY_ONE='1',KEY_TWO='2',KEY_THREE='3',KEY_FOUR='4',KEY_FIVE='5',KEY_SIX='6',KEY_SEVEN='7',KEY_EIGHT='8',KEY_NINE='9',
 KEY_A='A',KEY_B='B',KEY_C='C',KEY_D='D',KEY_E='E',KEY_F='F',KEY_G='G',KEY_H='H',KEY_I='I',KEY_J='J',KEY_K='K',KEY_L='L',KEY_M='M',KEY_N='N',KEY_O='O',KEY_P='P',KEY_Q='Q',KEY_R='R',KEY_S='S',KEY_T='T',KEY_U='U',KEY_V='V',KEY_W='W',KEY_X='X',KEY_Y='Y',KEY_Z='Z',
 KEY_SPACE=32,KEY_ESCAPE=256,KEY_LEFT_CONTROL=341,KEY_RIGHT_CONTROL=345,KEY_LEFT_SHIFT=340,KEY_LEFT_BRACKET=91,KEY_RIGHT_BRACKET=93,KEY_F1=290,KEY_F2=291,KEY_F3=292,KEY_F5=294,KEY_F9=298
};
void SetConfigFlags(unsigned);
void InitWindow(int,int,const char*);
void SetTargetFPS(int);
void DisableCursor();
void CloseWindow();
bool WindowShouldClose();
float GetFrameTime();
bool IsKeyPressed(int);
bool IsKeyDown(int);
bool IsMouseButtonDown(int);
bool IsMouseButtonPressed(int);
bool IsWindowReady();
Vector2 GetMouseDelta();
float GetMouseWheelMove();
int GetScreenWidth();
int GetScreenHeight();
void BeginDrawing();
void EndDrawing();
void ClearBackground(Color);
void BeginMode3D(Camera3D);
void EndMode3D();
void DrawRectangle(int,int,int,int,Color);
void DrawRectangleLines(int,int,int,int,Color);
void DrawLine(int,int,int,int,Color);
void DrawText(const char*,int,int,int,Color);
int MeasureText(const char*,int);
const char* TextFormat(const char*,...);
void DrawLine3D(Vector3,Vector3,Color);
void DrawCube(Vector3,float,float,float,Color);
void DrawCubeWires(Vector3,float,float,float,Color);
void DrawSphere(Vector3,float,Color);
void* MemAlloc(unsigned int);
void UploadMesh(Mesh*,bool);
Model LoadModelFromMesh(Mesh);
void UnloadModel(Model);
void DrawModel(Model,Vector3,float,Color);
Texture2D LoadTextureFromImage(Image);
void UnloadTexture(Texture2D);
