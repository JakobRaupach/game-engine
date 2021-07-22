#include <iostream>
#include <SDL2/SDL.h>
#include <vector>

using namespace std;

// const
const int WIDTH = 1440, HEIGHT = 900, FRAMES = 30;

// structs
struct vec3d
{
    float x, y, z;
};

struct triangle
{
    vec3d p[3];
};

struct mesh
{
    vector<triangle> tris;
};

struct mat4x4
{
    float m[4][4] = {0};
};

class game_engine
{
    public: 
        game_engine()
        {
            mTicksCount = 0;
        }

    private:
        mesh cube;
        mat4x4 matProj;
        Uint32 mTicksCount;

        float fTheta;

        void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
        {
            o.x = i.x * m.m[0][0] + i.x * m.m[1][0] + i.x * m.m[2][0] + i.x * m.m[3][0];
            o.y = i.y * m.m[0][1] + i.y * m.m[1][1] + i.y * m.m[2][1] + i.y * m.m[3][1];
            o.z = i.z * m.m[0][2] + i.z * m.m[1][2] + i.z * m.m[2][2] + i.z * m.m[3][2];
            float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

            if (w != 0.0f)
            {
                o.x /= w;
                o.y /= w;
                o.z /= w;
            }
    }

    public:
        SDL_Window *window;
        SDL_Renderer *renderer;

        int Init()
        {
            if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
            {
                return -1;
            }

            return 0;
        }

        void CreateWindow(char *title, int height, int width)
        {
            window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
            renderer = SDL_CreateRenderer(window, -1, 0);
        }

        void CreateGame()
        {
            ClearWindow();
            fillCube();

            float fNear = 0.1f;
            float fFar = 1000.0f;
            float fFov = 90.0f;
            float fAspectRatio = (float) HEIGHT / WIDTH;
            float fFovRad = 1.0f / tanf(fFov * 0.5f);

            matProj.m[0][0] = fAspectRatio * fFovRad;
            matProj.m[1][1] = fFovRad;
            matProj.m[2][2] = fFar / (fFar - fNear);
            matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
            matProj.m[2][3] = 1.0f;
            matProj.m[3][3] = 0.0f;
        }

        void UpdateGame()
        {
            while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 1000 / FRAMES))
                ;

            float deltaTime = (SDL_GetTicks() - (float) mTicksCount) / 1000.0f;
            mTicksCount = SDL_GetTicks();

            ClearWindow();
            Draw(deltaTime);
        }

        void Draw(float fDeltaTime)
        {
            mat4x4 matRotZ, matRotX;

            fTheta += 1.0f * fDeltaTime;

            // Rotation Z
            matRotZ.m[0][0] = cosf(fTheta);
            matRotZ.m[0][1] = sinf(fTheta);
            matRotZ.m[1][0] = -sinf(fTheta);
            matRotZ.m[1][1] = cosf(fTheta);
            matRotZ.m[2][2] = 1;
            matRotZ.m[3][3] = 1;
                
            // Rotation X
            matRotX.m[0][0] = 1;
            matRotX.m[1][1] = cosf(fTheta * 0.5f);
            matRotX.m[1][2] = sinf(fTheta * 0.5f);
            matRotX.m[2][1] = -sinf(fTheta * 0.5f);
            matRotX.m[2][2] = cosf(fTheta * 0.5f);
            matRotX.m[3][3] = 1;


            for (auto tri : cube.tris)
            {
                triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

                MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
                MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
                MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

                MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
                MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
                MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

                triTranslated = triRotatedZX;
                triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
                triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
                triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

                MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
                MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
                MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

                triProjected.p[0].x += 1.0f;
                triProjected.p[0].y += 1.0f;
                triProjected.p[1].x += 1.0f;
                triProjected.p[1].y += 1.0f;
                triProjected.p[2].x += 1.0f;
                triProjected.p[2].y += 1.0f;

                triProjected.p[0].x *= 0.5f * (float)WIDTH;
                triProjected.p[0].y *= 0.5f * (float)HEIGHT;
                triProjected.p[1].x *= 0.5f * (float)WIDTH;
                triProjected.p[1].y *= 0.5f * (float)HEIGHT;
                triProjected.p[2].x *= 0.5f * (float)WIDTH;
                triProjected.p[2].y *= 0.5f * (float)HEIGHT;

                DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                             triProjected.p[1].x, triProjected.p[1].y,
                             triProjected.p[2].x, triProjected.p[2].y);
            }
            SDL_RenderPresent(renderer);
        }

        void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
        {
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            SDL_RenderDrawLine(renderer, x2, y2, x3, y3);
            SDL_RenderDrawLine(renderer, x3, y3, x1, y1);

        }

    private:

        void ClearWindow()
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }

        void fillCube()
        {
            cube.tris = {
                // South
                {
                    0.0f, 0.0f, 0.0f, 
                    0.0f, 1.0f, 0.0f, 
                    1.0f, 1.0f, 0.0f
                },
                {
                    0.0f, 0.0f, 0.0f, 
                    0.0f, 1.0f, 0.0f, 
                    0.0f, 1.0f, 0.0f
                },

                // East
                {
                    1.0f, 0.0f, 0.0f, 
                    1.0f, 1.0f, 0.0f, 
                    1.0f, 1.0f, 1.0f
                },
                {
                    1.0f, 0.0f, 0.0f, 
                    1.0f, 1.0f, 1.0f, 
                    1.0f, 0.0f, 1.0f
                },

                // North
                {
                    1.0f, 0.0f, 1.0f, 
                    1.0f, 1.0f, 1.0f, 
                    0.0f, 1.0f, 1.0f
                },
                {
                    1.0f, 0.0f, 1.0f, 
                    0.0f, 1.0f, 1.0f, 
                    1.0f, 1.0f, 1.0f
                },

                // West
                {
                    0.0f, 0.0f, 1.0f, 
                    0.0f, 1.0f, 1.0f, 
                    0.0f, 1.0f, 0.0f
                },
                {
                    0.0f, 0.0f, 1.0f, 
                    0.0f, 1.0f, 0.0f, 
                    0.0f, 0.0f, 0.0f
                },

                // Top
                {
                    0.0f, 1.0f, 0.0f, 
                    0.0f, 1.0f, 1.0f, 
                    1.0f, 1.0f, 1.0f
                },
                {
                    0.0f, 1.0f, 0.0f, 
                    1.0f, 1.0f, 1.0f, 
                    1.0f, 1.0f, 0.0f
                },

                // Bottom
                {
                    1.0f, 0.0f, 1.0f, 
                    0.0f, 0.0f, 1.0f, 
                    0.0f, 0.0f, 0.0f
                },
                {
                    1.0f, 0.0f, 1.0f, 
                    0.0f, 0.0f, 0.0f, 
                    1.0f, 0.0f, 0.0f
                },
            };
        }
};

int main(int argc, char *argv[])
{
    game_engine engine;

    engine.Init();
    engine.CreateWindow("My Game Engine", HEIGHT, WIDTH);
    engine.CreateGame();

    SDL_Event e;
    bool quit = false;



    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
        }
        engine.UpdateGame();
    }
    return 0;
}
