#include <libk/stdio.h>
#include <libk/ctype.h>

#include <vitasdk.h>
#include <taihen.h>

#include "config.h"
#include "draw.h"
#include "fs.h"
#include "menus.h"
#include "power.h"
#include "utils.h"
#include "math_utils.h"

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

#define HOOKS_NUM 11

#define CLOCK_SET_DELAY_INIT 5000000 // 5 seconds
#define TIMER_SECOND         1000000 // 1 second

#define BUTTON_COMBO_1 ((ctrl->buttons & SCE_CTRL_LTRIGGER) && (ctrl->buttons & SCE_CTRL_RTRIGGER) && (ctrl->buttons & SCE_CTRL_START))
#define BUTTON_COMBO_2 ((ctrl->buttons & SCE_CTRL_L1) && (ctrl->buttons & SCE_CTRL_R1) && (ctrl->buttons & SCE_CTRL_START))
#define BUTTON_COMBO_3 ((ctrl->buttons & SCE_CTRL_LTRIGGER) && (ctrl->buttons & SCE_CTRL_RTRIGGER) && (ctrl->buttons & SCE_CTRL_SELECT))
#define BUTTON_COMBO_4 ((ctrl->buttons & SCE_CTRL_L1) && (ctrl->buttons & SCE_CTRL_R1) && (ctrl->buttons & SCE_CTRL_SELECT))

static SceUID tai_uid[HOOKS_NUM];
static tai_hook_ref_t hook[HOOKS_NUM];

SceInt showVSH = 0;
SceInt assignOperation = 0;
static SceBool isConfigSet = SCE_FALSE;
static SceUInt64 timer = 0, tick = 0, t_tick = 0;

static SceInt frames = 0, fps_data = 0;

static float screenFilterTransparency[] = {0.01f, 0.05f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f, 0.6f, 0.65f, 0.7f, 0.75f};

// Shaders
#include "../shaders/rgba_f.h"
#include "../shaders/rgba_v.h"

SceGxmShaderPatcher* patcher;

static const SceGxmProgram *const gxm_program_rgba_v = (SceGxmProgram*)&rgba_v;
static const SceGxmProgram *const gxm_program_rgba_f = (SceGxmProgram*)&rgba_f;

static SceGxmShaderPatcherId rgba_vertex_id;
static SceGxmShaderPatcherId rgba_fragment_id;
static const SceGxmProgramParameter* rgba_position;
static const SceGxmProgramParameter* rgba_rgba;
static const SceGxmProgramParameter* wvp;
static SceGxmVertexProgram* rgba_vertex_program_patched;
static SceGxmFragmentProgram* rgba_fragment_program_patched;
static vector3f* rgba_vertices = NULL;
static uint16_t* rgba_indices = NULL;
static SceUID rgba_vertices_uid, rgba_indices_uid;

static vector4f rect_rgba;
static matrix4x4 mvp;

// This function is from Framecounter by Rinnegatamante.
static SceVoid DisplayFPS(SceVoid)
{
	t_tick = sceKernelGetProcessTimeWide();
			
	if (tick == 0)
		tick = t_tick;
	else
	{
		if ((t_tick - tick) > TIMER_SECOND)
		{
			fps_data = frames;
			frames = 0;
			tick = t_tick;
		}

		drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
		drawStringf(0, 528, "FPS: %d", fps_data);
	}
	
	frames++;
}

static SceVoid DrawScrenFilter(SceInt transparency)
{
	drawRect(0, 0, 960, 544, RGBT(0, 0, 0, transparency));
}

SceInt sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf *pParam, SceDisplaySetBufSync sync) 
{
	if (hook[0] == 0)
		return -1;
	
	drawSetFrameBuf(pParam);

	if (timer == 0)
		timer = sceKernelGetProcessTimeWide();
	else if ((sceKernelGetProcessTimeWide() - timer) > (isConfigSet? Config_GetInterval() : CLOCK_SET_DELAY_INIT)) // Check in 5 seconds initially
	{
		if (Clock_Config.clock_set) // As long as VSH doesn't report default
		{
			// if current clock state don't match the ones in config -> re set the desired clock config.
			if ((scePowerGetArmClockFrequency() != profiles[Clock_Config.c_clock][0]) || (scePowerGetBusClockFrequency() != profiles[Clock_Config.c_clock][1]) || 
				(scePowerGetGpuClockFrequency() != profiles[Clock_Config.g_clock][2]) || (scePowerGetGpuXbarClockFrequency() != profiles[Clock_Config.g_clock][3]))
			{
				scePowerSetArmClockFrequency(profiles[Clock_Config.c_clock][0]);
				scePowerSetBusClockFrequency(profiles[Clock_Config.c_clock][1]);
				scePowerSetGpuClockFrequency(profiles[Clock_Config.g_clock][2]);
				scePowerSetGpuXbarClockFrequency(profiles[Clock_Config.g_clock][3]);
				isConfigSet = SCE_TRUE; // Once this is true check if the clock states have changed in 30 second intervals
			}
		}
		
		timer = 0;
	}

	if ((Menu_Config.battery_keep_display) && (showVSH == 0))
	{
		if (Menu_Config.battery_percent)
			Power_DisplayBatteryPercentage();
		if (Menu_Config.battery_lifetime)
			Power_DisplayBatteryLifetime();
		
		if (Menu_Config.battery_temp && !Menu_Config.clock_display)
			Power_DisplayBatteryTemp(0);
		else if (Menu_Config.battery_temp && Menu_Config.clock_display)
			Power_DisplayBatteryTemp(32);
	}

	if ((Menu_Config.clock_keep_display) && (showVSH == 0))
	{
		if (Menu_Config.clock_display)
		{
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
			drawStringf(0, 0, "CPU: %d/%d MHz", scePowerGetArmClockFrequency(), scePowerGetBusClockFrequency());
			drawStringf(0, 16, "GPU: %d/%d MHz", scePowerGetGpuClockFrequency(), scePowerGetGpuXbarClockFrequency());
		}
	}

	if ((Menu_Config.fps_keep_display && showVSH == 0))
		DisplayFPS();
	
	// if (Menu_Config.screen_filter_keep_enabled && showVSH == 0)
	// 	DrawScrenFilter(Menu_Config.screen_filter_transparency);

	if (showVSH != 0)
	{
		if (Menu_Config.battery_percent)
			Power_DisplayBatteryPercentage();
		if (Menu_Config.battery_lifetime)
			Power_DisplayBatteryLifetime();
		if (Menu_Config.battery_temp && !Menu_Config.clock_display)
			Power_DisplayBatteryTemp(0);
		else if (Menu_Config.battery_temp && Menu_Config.clock_display)
			Power_DisplayBatteryTemp(32);
		if (Menu_Config.clock_display)
		{
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
			drawStringf(0, 0, "CPU: %d/%d MHz", scePowerGetArmClockFrequency(), scePowerGetBusClockFrequency());
			drawStringf(0, 16, "GPU: %d/%d MHz", scePowerGetGpuClockFrequency(), scePowerGetGpuXbarClockFrequency());
		}
		if (Menu_Config.fps_display)
			DisplayFPS();
		
		Menu_Display();
	}

	return TAI_CONTINUE(SceInt, hook[0], pParam, sync);
}

static SceInt HandleControls(int port, tai_hook_ref_t hook, SceCtrlData *ctrl, int count)
{
	SceInt ret = 0;

	if (hook == 0)
		ret = 1;
	else
	{
    	ret = TAI_CONTINUE(int, hook, port, ctrl, count);
	
		if (showVSH == 0)
		{
			if (BUTTON_COMBO_1 || BUTTON_COMBO_2 || BUTTON_COMBO_3 || BUTTON_COMBO_4)
			{	
				if (Clock_Config.c_clock == -1)
				{
					profile_game[0] = scePowerGetArmClockFrequency();
					profile_game[1] = scePowerGetBusClockFrequency();
					Clock_Config.c_clock = 0;
				}
				
				if (Clock_Config.g_clock == -1)
				{
					profile_game[2] = scePowerGetGpuClockFrequency();
					profile_game[3] = scePowerGetGpuXbarClockFrequency();
					Clock_Config.g_clock = 0;
				}
				
				showVSH = VSH_MAIN_MENU;
			}
		}
		else if (showVSH != 0)
		{
			pressed_buttons = ctrl->buttons & ~old_buttons;

			Menu_HandleControls(pressed_buttons);

			old_buttons = ctrl->buttons;
			ctrl->buttons = 0;
		}
	}

	return ret;
}

static SceInt sceCtrlPeekBufferPositive_patched(SceInt port, SceCtrlData *ctrl, SceInt count) 
{
	return HandleControls(port, hook[1], ctrl, count);
}   

static SceInt sceCtrlPeekBufferPositive2_patched(SceInt port, SceCtrlData *ctrl, SceInt count) 
{
	return HandleControls(port, hook[2], ctrl, count);
}   

static SceInt sceCtrlReadBufferPositive_patched(SceInt port, SceCtrlData *ctrl, SceInt count) 
{
    return HandleControls(port, hook[3], ctrl, count);
}   

static SceInt sceCtrlReadBufferPositive2_patched(SceInt port, SceCtrlData *ctrl, SceInt count) 
{
    return HandleControls(port, hook[4], ctrl, count);
} 

static SceInt scePowerSetClockFrequency_patched(tai_hook_ref_t hook, SceInt port, SceInt freq)
{
	if (Clock_Config.c_clock == -1)
		return TAI_CONTINUE(SceInt, hook, freq);
	else
		return TAI_CONTINUE(SceInt, hook, profiles[Clock_Config.c_clock][port]);
	
	if (Clock_Config.g_clock == -1)
		return TAI_CONTINUE(SceInt, hook, freq);
	else
		return TAI_CONTINUE(SceInt, hook, profiles[Clock_Config.g_clock][port]);
}

static SceInt scePowerGetArmClockFrequency_patched(SceInt freq) 
{
    return scePowerSetClockFrequency_patched(hook[5], 0, freq);
}

static SceInt scePowerSetBusClockFrequency_patched(SceInt freq) 
{
    return scePowerSetClockFrequency_patched(hook[6], 1, freq);
}

static SceInt scePowerSetGpuClockFrequency_patched(SceInt freq) 
{
    return scePowerSetClockFrequency_patched(hook[7], 2, freq);
}

static SceInt scePowerSetGpuXbarClockFrequency_patched(SceInt freq) 
{
    return scePowerSetClockFrequency_patched(hook[8], 3, freq);
}

void* gpu_alloc_map(SceKernelMemBlockType type, SceGxmMemoryAttribFlags gpu_attrib, size_t size, SceUID *uid){
	SceUID memuid;
	void *addr;

	if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW)
		size = ALIGN(size, 256 * 1024);
	else
		size = ALIGN(size, 4 * 1024);

	memuid = sceKernelAllocMemBlock("gpumem", type, size, NULL);
	if (memuid < 0)
		return NULL;

	if (sceKernelGetMemBlockBase(memuid, &addr) < 0)
		return NULL;

	if (sceGxmMapMemory(addr, size, gpu_attrib) < 0) {
		sceKernelFreeMemBlock(memuid);
		return NULL;
	}

	if (uid)
		*uid = memuid;

	return addr;
}

SceInt sceGxmShaderPatcherCreate_patched(const SceGxmShaderPatcherParams *params, SceGxmShaderPatcher **shaderPatcher){
	int res =  TAI_CONTINUE(int, hook[9], params, shaderPatcher);
	
	// Grabbing a reference to used shader patcher
	patcher = *shaderPatcher;
	
	// Registering our shaders
	sceGxmShaderPatcherRegisterProgram(
		patcher,
		gxm_program_rgba_v,
		&rgba_vertex_id);
	sceGxmShaderPatcherRegisterProgram(
		patcher,
		gxm_program_rgba_f,
		&rgba_fragment_id);
		
	// Getting references to our vertex streams/uniforms
	rgba_position = sceGxmProgramFindParameterByName(gxm_program_rgba_v, "aPosition");
	rgba_rgba = sceGxmProgramFindParameterByName(gxm_program_rgba_f, "color");
	wvp = sceGxmProgramFindParameterByName(gxm_program_rgba_v, "wvp");
	
	// Setting up our vertex stream attributes
	SceGxmVertexAttribute rgba_vertex_attribute;
	SceGxmVertexStream rgba_vertex_stream;
	rgba_vertex_attribute.streamIndex = 0;
	rgba_vertex_attribute.offset = 0;
	rgba_vertex_attribute.format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	rgba_vertex_attribute.componentCount = 3;
	rgba_vertex_attribute.regIndex = sceGxmProgramParameterGetResourceIndex(rgba_position);
	rgba_vertex_stream.stride = sizeof(vector3f);
	rgba_vertex_stream.indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	// Creating our shader programs
	sceGxmShaderPatcherCreateVertexProgram(patcher,
		rgba_vertex_id, &rgba_vertex_attribute,
		1, &rgba_vertex_stream, 1, &rgba_vertex_program_patched);
	sceGxmShaderPatcherCreateFragmentProgram(patcher,
		rgba_fragment_id, SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
		SCE_GXM_MULTISAMPLE_NONE, NULL, gxm_program_rgba_f,
		&rgba_fragment_program_patched);
	
	// Allocating default vertices/indices on CDRAM
	rgba_vertices = gpu_alloc_map(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, SCE_GXM_MEMORY_ATTRIB_READ,
		4 * sizeof(vector3f), &rgba_vertices_uid);
	rgba_indices = gpu_alloc_map(
		SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, SCE_GXM_MEMORY_ATTRIB_READ,
		4 * sizeof(unsigned short), &rgba_indices_uid);
		
	// Setting up default vertices
	rgba_vertices[0].x = 0.0f;
	rgba_vertices[0].y = 0.0f;
	rgba_vertices[0].z = 0.5f;
	rgba_vertices[1].x = 0.0f;
	rgba_vertices[1].y = 544.0f;
	rgba_vertices[1].z = 0.5f;
	rgba_vertices[2].x = 960.0f;
	rgba_vertices[2].y = 544.0f;
	rgba_vertices[2].z = 0.5f;
	rgba_vertices[3].x = 960.0f;
	rgba_vertices[3].y = 0.0f;
	rgba_vertices[3].z = 0.5f;
	
	// Setting up default indices
	int i;
	for (i=0;i<4;i++){
		rgba_indices[i] = i;
	}
	
	// Setting up default modelviewprojection matrix
	matrix4x4 projection, modelview;
	matrix4x4_identity(modelview);
	matrix4x4_init_orthographic(projection, 0, 960, 544, 0, -1, 1);
	matrix4x4_multiply(mvp, projection, modelview);
	
	return res;
}

SceInt sceGxmEndScene_patched(SceGxmContext *context, const SceGxmNotification *vertexNotification, const SceGxmNotification *fragmentNotification)
{
	if (Menu_Config.screen_filter_keep_enabled && showVSH == 0)
	{
		rect_rgba.a = screenFilterTransparency[Menu_Config.screen_filter_transparency];
	}
	else
	{
		rect_rgba.a = 0.01f;
	}

	
	/* Before ending scene, we draw our stuffs */
	
	// Setting up desired shaders
	sceGxmSetVertexProgram(context, rgba_vertex_program_patched);
	sceGxmSetFragmentProgram(context, rgba_fragment_program_patched);
	
	// Setting vertex stream and uniform values
	void *rgba_buffer, *wvp_buffer;
	sceGxmReserveFragmentDefaultUniformBuffer(context, &rgba_buffer);
	sceGxmSetUniformDataF(rgba_buffer, rgba_rgba, 0, 4, &rect_rgba.r);
	sceGxmReserveVertexDefaultUniformBuffer(context, &wvp_buffer);
	sceGxmSetUniformDataF(wvp_buffer, wvp, 0, 16, (const float*)mvp);
	sceGxmSetVertexStream(context, 0, rgba_vertices);
	
	// Scheduling a draw command
	sceGxmDraw(context, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, SCE_GXM_INDEX_FORMAT_U16, rgba_indices, 4);
	
	return TAI_CONTINUE(SceInt, hook[10], context, vertexNotification, fragmentNotification);
}

SceVoid _start() __attribute__ ((weak, alias ("module_start")));
SceInt module_start(SceSize argc, const SceVoid *args) 
{
	// Setting up default color
	rect_rgba.r = 0.0f;
	rect_rgba.g = 0.0f;
	rect_rgba.b = 0.0f;

	sceAppMgrAppParamGetString(0, 12, titleID , 256); // Get titleID of current running application.
	FS_RecursiveMakeDir("ur0:/data/vsh/titles");
	Config_LoadConfig();

	tai_uid[0] = Utils_TaiHookFunctionImport(&hook[0], 0x4FAACD11, 0x7A410B64, sceDisplaySetFrameBuf_patched);
	tai_uid[1] = Utils_TaiHookFunctionImport(&hook[1], 0xD197E3C7, 0xA9C3CED6, sceCtrlPeekBufferPositive_patched);
	tai_uid[2] = Utils_TaiHookFunctionImport(&hook[2], 0xD197E3C7, 0x15F81E8C, sceCtrlPeekBufferPositive2_patched);
	tai_uid[3] = Utils_TaiHookFunctionImport(&hook[3], 0xD197E3C7, 0x67E7AB83, sceCtrlReadBufferPositive_patched);
	tai_uid[4] = Utils_TaiHookFunctionImport(&hook[4], 0xD197E3C7, 0xC4226A3E, sceCtrlReadBufferPositive2_patched);
	tai_uid[5] = Utils_TaiHookFunctionImport(&hook[5], 0x1082DA7F, 0x74DB5AE5, scePowerGetArmClockFrequency_patched);
	tai_uid[6] = Utils_TaiHookFunctionImport(&hook[6], 0x1082DA7F, 0xB8D7B3FB, scePowerSetBusClockFrequency_patched);
	tai_uid[7] = Utils_TaiHookFunctionImport(&hook[7], 0x1082DA7F, 0x717DB06C, scePowerSetGpuClockFrequency_patched);
	tai_uid[8] = Utils_TaiHookFunctionImport(&hook[8], 0x1082DA7F, 0xA7739DBE, scePowerSetGpuXbarClockFrequency_patched);
	tai_uid[9] = Utils_TaiHookFunctionImport(&hook[9], TAI_ANY_LIBRARY, 0x05032658, sceGxmShaderPatcherCreate_patched);
	tai_uid[10] = Utils_TaiHookFunctionImport(&hook[10], TAI_ANY_LIBRARY, 0xFE300E2F, sceGxmEndScene_patched);
	
	return SCE_KERNEL_START_SUCCESS;
}

SceInt module_stop(SceSize argc, const SceVoid *args) 
{
	// free hooks that didn't fail
	for (SceInt i = 0; i < HOOKS_NUM; i++)
	{
		if (R_SUCCEEDED(tai_uid[i])) 
			taiHookRelease(tai_uid[i], hook[i]);
	}

	return SCE_KERNEL_STOP_SUCCESS;
}
