#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h> // For malloc/free
#include "nomad3d.h"
#include "testpicture.h"
#include "sdlcanvas.h"

using namespace Nomad3D;

CMS3DObject obj;
CMS3DObject obj2;
CMS3DObject obj22;
CMS3DObject obj3;
CCamera cam;
CCamera cam2;
CScene scene1;
CScene scene2;
CScene scene3;
CTestPicture picture1;
//CPicture picture1;
CPicture picture2;
CPicture picture3;
CPlayer player1;

CPoint3 eye(0,0,45);
CPoint3 look(0,0,-1000);
CVector3 up(0,1,0);

CSDLCanvas canvas;

void GameInit()
{
    canvas.Init();
    render.SetCanvas(&canvas);

	//bool b=obj.ReadObjectFile("butterfly.ms3d");
	//bool b=obj.ReadObjectFile("ABC.ms3d");
	//bool b=obj.ReadObjectFile("Desklamp.ms3d");
	bool b=obj.ReadObjectFile("jump_specular.ms3d");//("ninja.ms3d");//("sphere12.ms3d");//("Desklamp.ms3d");//("butterfly.ms3d");//("tube.ms3d");//
	//bool b=obj.ReadObjectFile("sphere.ms3d");
	assert(b);
	b = obj2.ReadObjectFile("crow.ms3d");//("plane15.ms3d");//
	b = obj22.ReadObjectFile("ABC.ms3d");
	assert(b);
	b=obj3.ReadObjectFile("butterfly.ms3d");//("ABC.ms3d");//
	assert(b);
//	b=obj4.ReadObjectFile("Desklamp.ms3d");
/*	b=obj31.ReadObjectFile("jump.ms3d");
	assert(b);
*/
	cam.LookAt(eye,look,up);
	cam.SetPerspective(90, float(NM3D_WINDOW_WIDTH*1.0/NM3D_WINDOW_HEIGHT), .1f, 250);
	//cam2.LookAt(eye,look,up);
	//cam2.SetPerspective(90, float(200*1.0/200), 1.1f, 150);
	cam2.LookAt(eye,look,up);
	cam2.SetPerspective(90, float(200*1.0/100), 1.1f, 150);

//	render.SetDC(g_hdc);

	scene1.AddObject(&obj);
	scene1.AddObject(&obj2);
	//scene1.AddObject(&obj4);
	//scene1.AddObject(&obj3);
	//scene1.AddObject(&obj4);
	scene1.SetCamera(cam);

	scene2.AddObject(&obj22);
	scene2.SetCamera(cam2);

	scene3.AddObject(&obj3);
	scene3.SetCamera(cam2);
/*
	scene31.AddObject(&obj31);
	scene31.SetCamera(cam);
*/
	picture1.AddObject(&obj);
	picture1.SetScene(&scene1);
	picture1.SetViewInfo(CViewInfo(0,0,NM3D_WINDOW_WIDTH,NM3D_WINDOW_HEIGHT,CRGBA(0,0,0,1)));

	picture2.SetScene(&scene2);
	picture2.SetViewInfo(CViewInfo(NM3D_WINDOW_WIDTH-200,0,200,200,CRGBA(100,0,0,1)));

//	picture3.AddObject(&obj3);
	picture3.SetScene(&scene3);
	picture3.SetViewInfo(CViewInfo(NM3D_WINDOW_WIDTH-200,NM3D_WINDOW_HEIGHT-100,200,100,CRGBA(10,100,10,1)));
/*	
	picture31.SetScene(&scene31);
	picture31.SetViewInfo(CViewInfo(WINDOW_WIDTH/16*7+4,WINDOW_HEIGHT/8*6+3,WINDOW_WIDTH/16,WINDOW_HEIGHT/8,CRGBA(100,100,10,1)));
*/
	player1.AddPicture(&picture1);
//	player1.AddPicture(&picture2);
	player1.AddPicture(&picture3);
//	player1.AddPicture(&picture31);

	//CPoint4 pos(0,40,15);
	//CVector3 dir(10,10,20);
	CPoint4 pos(0,0,0);
	CVector3 dir(1,0.7f,0); // Light direction is defined by the vector (dir - origin)
    CRGBA amb(50,50,50);
	light.SetGlobalAmbient(amb);
	light.CreateLight(CLight::NM3D_LIGHT_STATE_ON,CLight::NM3D_LIGHT_TYPE_DIRECTION,CRGBA(20,20,20),CRGBA(100,100,100),CRGBA(255,255,255),
		&pos,&dir,0,0,0,0,0);
	light.CreateLight(CLight::NM3D_LIGHT_STATE_ON,CLight::NM3D_LIGHT_TYPE_POINT,CRGBA(20,20,20),CRGBA(200,200,200),CRGBA(255,255,255),
		&pos,&dir,0,0.004f,0,0,0);
	light.CreateLight(CLight::NM3D_LIGHT_STATE_ON,CLight::NM3D_LIGHT_TYPE_SPOT,CRGBA(20,20,20),CRGBA(250,250,250),CRGBA(255,255,255),
		&pos,&dir,0,0.002f,0,15,2.0);
}

void GameLoop()
{
	//Start_Clock();
	//render.ClearVBuffer();
	player1.ShowPicture(-1);
	render.Flip();
	//Wait_Clock(FPS_60);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    GameInit();

    bool running = true; 
    SDL_Event e;
 
    while (running)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if ( (e.type == SDL_QUIT) ||
                 (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) )
            {
                running = false;
            }
            else if (e.type == SDL_KEYDOWN)
            {
				static int z=1;

				CCamera* pCamera = scene1.GetCamera();
				float fUnits = 1.0f;

                switch (e.key.keysym.sym)
                {
                case SDLK_UP:
                    pCamera->Walk(-fUnits);
                    break;
                case SDLK_DOWN:
                    pCamera->Walk(fUnits);
                    break;
                case SDLK_LEFT:
                    pCamera->Strafe(-fUnits);
                    break;
                case SDLK_RIGHT:
                    pCamera->Strafe(fUnits);
                    break;
                case 'A':
                case 'a':
                    pCamera->Yaw(-1);
                    break;
                case 'D':
                case 'd':
                    pCamera->Yaw(1);
                    break;
                case 'L':
                case 'l':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_LINE);
                    break;
                case 'F':
                case 'f':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_FLAT);
                    break;
                case 'G':
                case 'g':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_GOURAUD);
                    break;
                case 'H':
                case 'h':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_GOURAUD_LaMoth);
                    break;
                case 'T':
                case 't':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_GOURAUD_TEXTURE);
                    break;
                case 'Y':
                case 'y':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_GOURAUD_TEXTURE_LaMoth);
                    break;
                case 'S':
                case 's':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_GOURAUD_TEXTURE_SS);
                    break;
                case 'B':
                case 'b':
                    render.SetRenderType(CRender::NM3D_RENDER_TYPE_LINE_NO_BACK);
                    break;
                case 'Z':
                case 'z':
                    render.SetZBufferState(!render.GetZBufferState());
                    break;
                case 'X':
                case 'x':
                    if (light.GetLightState(0) == CLight::NM3D_LIGHT_STATE_OFF)
                        light.SetLightState(0, CLight::NM3D_LIGHT_STATE_ON);
                    else
                        light.SetLightState(0, CLight::NM3D_LIGHT_STATE_OFF);
                    break;
                case 'C':
                case 'c':
                    if (light.GetLightState(1) == CLight::NM3D_LIGHT_STATE_OFF)
                        light.SetLightState(1, CLight::NM3D_LIGHT_STATE_ON);
                    else
                        light.SetLightState(1, CLight::NM3D_LIGHT_STATE_OFF);
                    break;
                case 'V':
                case 'v':
                    if (light.GetLightState(2) == CLight::NM3D_LIGHT_STATE_OFF)
                        light.SetLightState(2, CLight::NM3D_LIGHT_STATE_ON);
                    else
                        light.SetLightState(2, CLight::NM3D_LIGHT_STATE_OFF);
                    break;
                case SDLK_SPACE:
                    eye.Assign(0, 0, z++);
                    break;
                case SDLK_RETURN:
                    eye.Assign(0, 0, z--);
                    break;
                }
            }
        }        

        GameLoop();
        // Cap at ~60 FPS
        //SDL_Delay(16);
    }

    return 0;
}
