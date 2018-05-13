#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "vshader_shbin.h"
#include <3ds.h>
#include <citro3d.h>

#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))

typedef struct { float x, y, z; } vertex;
static const vertex vertex_list[] =
{

{ 200.0f, 200.0f, 0.5f },
{ 100.0f, 40.0f, 0.5f },
{ 300.0f, 40.0f, 0.5f },

};

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection;
static C3D_Mtx projection;
static void* vbo_data;

static void sceneRender(void) {

// Update the uniforms
C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
// Draw the VBO
C3D_DrawArrays(GPU_TRIANGLES, 0, vertex_list_count);
}

static void sceneExit(void)
{

// Free the VBO
linearFree(vbo_data);
// Free the shader program
shaderProgramFree(&program);
DVLB_Free(vshader_dvlb);
}

static void sceneInit(void)
{

// Load the vertex shader, create a shader program and bind it
vshader_dvlb = DVLB_ParseFile((uint32_t*)vshader_shbin, vshader_shbin_size);
shaderProgramInit(&program);
shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
C3D_BindProgram(&program);
// Get the location of the uniforms
uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
// Configure attributes for use with the vertex shader
C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
AttrInfo_Init(attrInfo);
AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
AttrInfo_AddFixed(attrInfo, 1); // v1=color
// Set the fixed attribute (color) to solid white
C3D_FixedAttribSet(1, 1.0, 1.0, 1.0, 1.0);
// Compute the projection matrix
Mtx_OrthoTilt(&projection, 0.0, 400.0, 0.0, 240.0, 0.0, 1.0, 0.0);
// Create the VBO (vertex buffer object)
vbo_data = linearAlloc(sizeof(vertex_list));
memcpy(vbo_data, vertex_list, sizeof(vertex_list));
// Configure buffers
C3D_BufInfo* bufInfo = C3D_GetBufInfo();
BufInfo_Init(bufInfo);
BufInfo_Add(bufInfo, vbo_data, sizeof(vertex), 1, 0x0);
// Configure the first fragment shading substage to just pass through the vertex color
// See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
C3D_TexEnv* env = C3D_GetTexEnv(0);
C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

}




int main(int argc, char **argv) {


	gfxInitDefault();
	//consoleInit(GFX_TOP, NULL);
	
	//Init citro3d
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    
	//width, height, colorFmt, depthFmt
	C3D_RenderTarget* target =
	C3D_RenderTargetCreate(240, 400, GPU_RB_RGB8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(target, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
    u32 transferFlags = GX_TRANSFER_FLIP_VERT(false)|
    GX_TRANSFER_OUT_TILED(false)|
    GX_TRANSFER_RAW_COPY(false)|
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8)|
    GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8)|
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_XY);
    C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, transferFlags);
	
	// Initialize the scene
    sceneInit();
	
	//printf("Hello 3DS World!");

	// Main loop
	while (aptMainLoop()) {
		hidScanInput();
		// Your code goes here
		
		
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		//gfxFlushBuffers();
		//gfxSwapBuffers();
		//gspWaitForVBlank();
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C3D_FrameDrawOn(target);
	sceneRender();
	C3D_FrameEnd(0);
	}
	sceneExit();
    C3D_Fini(); //End citro3d
	gfxExit();
	return 0;
}
