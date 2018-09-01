#include "config.h"
#include "draw.h"
#include "menus.h"
#include "power.h"
#include "utils.h"

#define MAIN_MAX_ITEMS     12
#define BATTERY_MAX_ITEMS  4
#define FPS_MAX_ITEMS      2
#define SCREEN_FILTER_MAX_ITEMS 2
#define ADVANCED_MAX_ITEMS 3
#define REFRESH_MAX_ITEMS  6
#define COLOUR_MAX_ITEMS   9
#define CONFIRM_MAX_ITEMS  2

static SceInt app_list = 0;
SceInt selection = 0;

SceInt profile_max_battery[] = {111, 111, 111, 111};
SceInt profile_balance[] = {266, 166, 166, 111};
SceInt profile_game[] = {333, 222, 166, 166};
SceInt profile_max_performance[] = {444, 222, 222, 166};

SceInt *profiles[4] = {profile_max_battery, profile_balance, profile_game, profile_max_performance};
static char *colourStr[] = {"Red", "Green", "Blue", "Yellow", "Orange", "Purple", "Pink", "Black", "White", "Custom"};
static SceInt refresh_rates[] = { 1, 5, 10, 20, 30, 45, 60 };

static SceVoid Menu_DisplayMainMenu(SceVoid)
{
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.title_text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.title_bg_col : RGB_GREEN);
	drawStringCenter(100, "PSV VSH MENU MOD");
		
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
		
	if (Clock_Config.c_clock == 2)
		drawStringCenter(130, "CPU CLOCK default");
	else 
		drawStringfCenter(130, "CPU CLOCK %d/%d", scePowerGetArmClockFrequency(), scePowerGetBusClockFrequency());
	if (Clock_Config.g_clock == 2)
		drawStringCenter(146, "GPU CLOCK default");
	else
		drawStringfCenter(146, "GPU CLOCK %d/%d", scePowerGetGpuClockFrequency(), scePowerGetGpuXbarClockFrequency());
		
	drawStringfCenter(162, "VSH MENU COLOUR %s", colourStr[Menu_Config.colour]);
	drawStringCenter(178, "BATTERY OPTIONS ->");
	drawStringCenter(194, "ADVANCED MENU ->");
	drawStringCenter(210, "FPS SETTINGS ->");
	drawStringCenter(226, "SCREEN FILTER ->");
	drawStringCenter(242, "LOAD PROGRAM ->");
	drawStringCenter(258, "SUSPEND DEVICE");
	drawStringCenter(274, "SHUTDOWN DEVICE");
	drawStringCenter(290, "REBOOT DEVICE");
	drawStringCenter(306, "RESTART VSH");
	drawStringCenter(322, "EXIT");
		
	switch(selection)
	{
		case 0:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			if (Clock_Config.c_clock == 2)
				drawStringCenter(130, "CPU CLOCK default");
			else
				drawStringfCenter(130, "CPU CLOCK %d/%d", scePowerGetArmClockFrequency(), scePowerGetBusClockFrequency());
			break;
		case 1:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			if (Clock_Config.g_clock == 2)
				drawStringCenter(146, "GPU CLOCK default");
			else
				drawStringfCenter(146, "GPU CLOCK %d/%d", scePowerGetGpuClockFrequency(), scePowerGetGpuXbarClockFrequency());
			break;
		case 2:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(162, "VSH MENU COLOUR %s", colourStr[Menu_Config.colour]);
			break;
		case 3:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(178, "BATTERY OPTIONS ->");
			break;
		case 4:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(194, "ADVANCED MENU ->");
			break;
		case 5:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(210, "FPS SETTINGS ->");
			break;
		case 6:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(226, "SCREEN FILTER ->");
			break;
		case 7:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(242, "LOAD PROGRAM ->");
			break;
		case 8:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(258, "SUSPEND DEVICE");
			break;
		case 9:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(274, "SHUTDOWN DEVICE");
			break;
		case 10:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(290, "REBOOT DEVICE");
			break;
		case 11:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(306, "RESTART VSH");
			break;
		case 12:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(322, "EXIT");
			break;
	}
}

static SceVoid Menu_DisplayBatteryMenu(SceVoid)
{
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.title_text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.title_bg_col : RGB_GREEN);
	drawStringCenter(100, "BATTERY OPTIONS");
		
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
		
	drawStringCenter(130, "<- BACK");
	drawStringfCenter(162, "KEEP BATTERY DISPLAY %s", Menu_Config.battery_keep_display? "enabled" : "disabled");
	drawStringfCenter(178, "BATTERY PERCENT %s", Menu_Config.battery_percent? "enabled" : "disabled");
	drawStringfCenter(194, "BATTERY LIFETIME %s", Menu_Config.battery_lifetime? "enabled" : "disabled");
	drawStringfCenter(210, "BATTERY TEMP %s", Menu_Config.battery_temp? "enabled" : "disabled");
		
	switch(selection)
	{
		case 0:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(130, "<- BACK");
			break;
		case 1:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(162, "KEEP BATTERY DISPLAY %s", Menu_Config.battery_keep_display? "enabled" : "disabled");
			break;
		case 2:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(178, "BATTERY PERCENT %s", Menu_Config.battery_percent? "enabled" : "disabled");
			break;
		case 3:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(194, "BATTERY LIFETIME %s", Menu_Config.battery_lifetime? "enabled" : "disabled");
			break;
		case 4:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(210, "BATTERY TEMP %s", Menu_Config.battery_temp? "enabled" : "disabled");
			break;
	}
}

static SceVoid Menu_DisplayAdvancedMenu(SceVoid)
{
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.title_text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.title_bg_col : RGB_GREEN);
	drawStringCenter(100, "ADVANCED MENU");
		
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
		
	drawStringCenter(130, "<- BACK");
	drawStringfCenter(162, "REFRESH INTERVAL %02d seconds", refresh_rates[Clock_Config.refresh_interval]);
	drawStringfCenter(178, "KEEP CLOCK DISPLAY %s", Menu_Config.clock_keep_display? "enabled" : "disabled");
	drawStringfCenter(194, "CLOCK DISPLAY %s", Menu_Config.clock_display? "enabled" : "disabled");

	switch(selection)
	{
		case 0:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(130, "<- BACK");
			break;
		case 1:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(162, "REFRESH INTERVAL %02d seconds", refresh_rates[Clock_Config.refresh_interval]);
			break;
		case 2:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(178, "KEEP CLOCK DISPLAY %s", Menu_Config.clock_keep_display? "enabled" : "disabled");
			break;
		case 3:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(194, "CLOCK DISPLAY %s", Menu_Config.clock_display? "enabled" : "disabled");
			break;
	}
}

static SceVoid Menu_DisplayFPSMenu(SceVoid)
{
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.title_text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.title_bg_col : RGB_GREEN);
	drawStringCenter(100, "FPS SETTINGS");
		
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
		
	drawStringCenter(130, "<- BACK");
	drawStringfCenter(162, "KEEP FPS DISPLAY %s", Menu_Config.fps_keep_display? "enabled" : "disabled");
	drawStringfCenter(178, "FPS %s", Menu_Config.fps_display? "enabled" : "disabled");

	switch(selection)
	{
		case 0:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(130, "<- BACK");
			break;
		case 1:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(162, "KEEP FPS DISPLAY %s", Menu_Config.fps_keep_display? "enabled" : "disabled");
			break;
		case 2:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(178, "FPS %s", Menu_Config.fps_display? "enabled" : "disabled");
			break;
	}
}

static SceVoid Menu_DisplayScreenFilterMenu(SceVoid)
{
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.title_text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.title_bg_col : RGB_GREEN);
	drawStringCenter(100, "SCREEN FILTER SETTINGS");
		
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
		
	drawStringCenter(130, "<- BACK");
	drawStringfCenter(162, "SCREEN FILTER %s", Menu_Config.screen_filter_keep_enabled? "enabled" : "disabled");
	drawStringfCenter(178, "SCREEN FILTER TRANSPARENCY %d", Menu_Config.screen_filter_transparency);

	switch(selection)
	{
		case 0:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(130, "<- BACK");
			break;
		case 1:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(162, "SCREEN FILTER %s", Menu_Config.screen_filter_keep_enabled? "enabled" : "disabled");
			break;
		case 2:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(178, "SCREEN FILTER TRANSPARENCY %d", Menu_Config.screen_filter_transparency);
			break;
	}
}

static SceVoid Menu_DisplayProgramMenu(SceVoid)
{
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.title_text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.title_bg_col : RGB_GREEN);
	drawStringCenter(100, "LOAD PROGRAM");
		
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
	
	drawStringCenter(130, "<- BACK");
	
	for (SceInt i = 0; i < 5; i++)
	{
		if (strlen(app_title[i]) != 0)
		{
			app_list = (i + 1); // increment app list
			drawStringCenter(162 + (16 * i), app_title[i]);
		}
	}
		
	switch(selection)
	{
		case 0:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(130, "<- BACK");
			break;
		case 1:
			if (strlen(app_title[0]) != 0)
			{
				drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
				drawStringCenter(162, app_title[0]);
			}
			break;
		case 2:
			if (strlen(app_title[1]) != 0)
			{
				drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
				drawStringCenter(178, app_title[1]);
			}
			break;
		case 3:
			if (strlen(app_title[2]) != 0)
			{
				drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
				drawStringCenter(194, app_title[2]);
			}
			break;
		case 4:
			if (strlen(app_title[3]) != 0)
			{
				drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
				drawStringCenter(210, app_title[3]);
			}
			break;
		case 5:
			if (strlen(app_title[4]) != 0)
			{
				drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
				drawStringCenter(226, app_title[4]);
			}
			break;
	}
}

static SceVoid Menu_DisplayConfirmMenu(SceVoid)
{
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.title_text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.title_bg_col : RGB_GREEN);
	drawStringCenter(100, "YOU SURE?");
		
	drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.bg_col : Config_GetVSHColour());
		
	drawStringCenter(130, "CANCLE");
	drawStringfCenter(162, "PROCEED");

	switch(selection)
	{
		case 0:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringCenter(130, "CANCLE");
			break;
		case 1:
			drawSetColour(Menu_Config.colour == 9? Custom_Colour.text_col : WHITE, Menu_Config.colour == 9? Custom_Colour.cursor_col : SKYBLUE);
			drawStringfCenter(162, "PROCEED");
			break;
	}
}

SceVoid Menu_Display(SceVoid)
{
	switch(showVSH)
	{
		case VSH_MAIN_MENU:
			Menu_DisplayMainMenu();
			break;
		case VSH_BATTERY_MENU:
			Menu_DisplayBatteryMenu();
			break;
		case VSH_ADVANCED_MENU:
			Menu_DisplayAdvancedMenu();
			break;
		case VSH_FPS_MENU:
			Menu_DisplayFPSMenu();
			break;
		case VSH_PROGRAM_MENU:
			Menu_DisplayProgramMenu();
			break;
		case VSH_SCREEN_FILTER_MENU:
			Menu_DisplayScreenFilterMenu();
			break;
		case VSH_CONFIRM_MENU:
			Menu_DisplayConfirmMenu();
			break;
	}
}

SceInt Menu_HandleControls(SceUInt32 pad)
{
	if (showVSH == VSH_MAIN_MENU) // Main VSH Menu
	{
		if (pad & SCE_CTRL_DOWN)
			selection++;
		else if (pad & SCE_CTRL_UP)
			selection--;
			
		Utils_SetMax(&selection, 0, MAIN_MAX_ITEMS);
		Utils_SetMin(&selection, MAIN_MAX_ITEMS, 0);
			
		if (selection == 0) 
		{
			if ((pad & SCE_CTRL_LEFT) && (Clock_Config.c_clock > 0))
			{
				Clock_Config.c_clock--;
				scePowerSetArmClockFrequency(profiles[Clock_Config.c_clock][0]);
				scePowerSetBusClockFrequency(profiles[Clock_Config.c_clock][1]);

				if (Clock_Config.c_clock != 2)
					Clock_Config.clock_set = SCE_TRUE;
				else
					Clock_Config.clock_set = SCE_FALSE;

				Config_SaveClockConfig(Clock_Config);
			}
			else if ((pad & SCE_CTRL_RIGHT) && (Clock_Config.c_clock < 3))
			{
				Clock_Config.c_clock++;
				scePowerSetArmClockFrequency(profiles[Clock_Config.c_clock][0]);
				scePowerSetBusClockFrequency(profiles[Clock_Config.c_clock][1]);
				
				if (Clock_Config.c_clock != 2)
					Clock_Config.clock_set = SCE_TRUE;
				else
					Clock_Config.clock_set = SCE_FALSE;

				Config_SaveClockConfig(Clock_Config);
			}
		}
		else if (selection == 1)
		{
			if ((pad & SCE_CTRL_LEFT) && (Clock_Config.g_clock > 0))
			{
				Clock_Config.g_clock--;
				scePowerSetGpuClockFrequency(profiles[Clock_Config.g_clock][2]);
				scePowerSetGpuXbarClockFrequency(profiles[Clock_Config.g_clock][3]);
				
				if (Clock_Config.c_clock != 2)
					Clock_Config.clock_set = SCE_TRUE;
				else
					Clock_Config.clock_set = SCE_FALSE;

				Config_SaveClockConfig(Clock_Config);
			}
			else if ((pad & SCE_CTRL_RIGHT) && (Clock_Config.g_clock < 3))
			{
				Clock_Config.g_clock++;
				scePowerSetGpuClockFrequency(profiles[Clock_Config.g_clock][2]);
				scePowerSetGpuXbarClockFrequency(profiles[Clock_Config.g_clock][3]);
				
				if (Clock_Config.c_clock != 2)
					Clock_Config.clock_set = SCE_TRUE;
				else
					Clock_Config.clock_set = SCE_FALSE;

				Config_SaveClockConfig(Clock_Config);
			}
		}
		else if (selection == 2)
		{
			if (pad & SCE_CTRL_LEFT)
			{
				if (Menu_Config.colour > 0)
					Menu_Config.colour--;
				else 
					Menu_Config.colour = COLOUR_MAX_ITEMS;
				
				Config_SaveMenuConfig(Menu_Config);
			}
			else if (pad & SCE_CTRL_RIGHT)
			{
				if (Menu_Config.colour < COLOUR_MAX_ITEMS)
					Menu_Config.colour++;
				else 
					Menu_Config.colour = 0;

				Config_SaveMenuConfig(Menu_Config);
			}
		}
		else if ((selection == 3) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			showVSH = VSH_BATTERY_MENU;
		}
		else if ((selection == 4) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			showVSH = VSH_ADVANCED_MENU;
		}
		else if ((selection == 5) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			showVSH = VSH_FPS_MENU;
		}
		else if ((selection == 6) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			showVSH = VSH_SCREEN_FILTER_MENU;
		}
		else if ((selection == 7) && (pad & SCE_CTRL_CROSS)) 
		{
			selection = 0;
			showVSH = VSH_PROGRAM_MENU;
		}
		else if ((selection == 8) && (pad & SCE_CTRL_CROSS))
		{
			scePowerRequestSuspend();
		}
		else if ((selection == 9) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			assignOperation = VSH_POWER_REQUEST_STANDBY;
			showVSH = VSH_CONFIRM_MENU;
		}
		else if ((selection == 10) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			assignOperation = VSH_POWER_REQUEST_COLDERESET;
			showVSH = VSH_CONFIRM_MENU;
		}
		else if ((selection == 11) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			assignOperation = VSH_RESTART_VSH;
			showVSH = VSH_CONFIRM_MENU;
		}
		else if (((selection == 12) && (pad & SCE_CTRL_CROSS)) || (pad & SCE_CTRL_CIRCLE))
		{
			selection = 0;
			showVSH = 0;
		}
	}
	else if (showVSH == VSH_BATTERY_MENU)
	{
		if (pad & SCE_CTRL_DOWN)
			selection++;
		else if (pad & SCE_CTRL_UP)
			selection--;
			
		Utils_SetMax(&selection, 0, BATTERY_MAX_ITEMS);
		Utils_SetMin(&selection, BATTERY_MAX_ITEMS, 0);
		
		if (((selection == 0) && (pad & SCE_CTRL_CROSS)) || (pad & SCE_CTRL_CIRCLE))
		{
			selection = 0;
			showVSH = VSH_MAIN_MENU;
		}
		else
		{
			if ((pad & SCE_CTRL_LEFT) || (pad & SCE_CTRL_RIGHT))
			{
				switch(selection)
				{
					case 1:
						Menu_Config.battery_keep_display = !Menu_Config.battery_keep_display;
						break;
					case 2:
						Menu_Config.battery_percent = !Menu_Config.battery_percent;
						break;
					case 3:
						Menu_Config.battery_lifetime = !Menu_Config.battery_lifetime;
						break;
					case 4:
						Menu_Config.battery_temp = !Menu_Config.battery_temp;
						break;
				}

				Config_SaveMenuConfig(Menu_Config);
			}
		}
	}
	else if (showVSH == VSH_ADVANCED_MENU)
	{
		if (pad & SCE_CTRL_DOWN)
			selection++;
		else if (pad & SCE_CTRL_UP)
			selection--;

		Utils_SetMax(&selection, 0, ADVANCED_MAX_ITEMS);
		Utils_SetMin(&selection, ADVANCED_MAX_ITEMS, 0);

		if (((selection == 0) && (pad & SCE_CTRL_CROSS)) || (pad & SCE_CTRL_CIRCLE))
		{
			selection = 0;
			showVSH = VSH_MAIN_MENU;
		}
		else if (selection == 1)
		{
			if (pad & SCE_CTRL_LEFT)
			{
				if (Clock_Config.refresh_interval > 0)
					Clock_Config.refresh_interval--;
				else 
					Clock_Config.refresh_interval = REFRESH_MAX_ITEMS;
				
				Config_SaveClockConfig(Clock_Config);
			}
			else if (pad & SCE_CTRL_RIGHT)
			{
				if (Clock_Config.refresh_interval < REFRESH_MAX_ITEMS)
					Clock_Config.refresh_interval++;
				else 
					Clock_Config.refresh_interval = 0;

				Config_SaveClockConfig(Clock_Config);
			}
		}
		else
		{
			if ((pad & SCE_CTRL_LEFT) || (pad & SCE_CTRL_RIGHT))
			{
				switch(selection)
				{
					case 2:
						Menu_Config.clock_keep_display = !Menu_Config.clock_keep_display;
						break;
					case 3:
						Menu_Config.clock_display = !Menu_Config.clock_display;
						break;
				}

				Config_SaveMenuConfig(Menu_Config);
			}
		}	
	}
	else if (showVSH == VSH_FPS_MENU)
	{
		if (pad & SCE_CTRL_DOWN)
			selection++;
		else if (pad & SCE_CTRL_UP)
			selection--;

		Utils_SetMax(&selection, 0, FPS_MAX_ITEMS);
		Utils_SetMin(&selection, FPS_MAX_ITEMS, 0);

		if (((selection == 0) && (pad & SCE_CTRL_CROSS)) || (pad & SCE_CTRL_CIRCLE))
		{
			selection = 0;
			showVSH = VSH_MAIN_MENU;
		}
		else
		{
			if ((pad & SCE_CTRL_LEFT) || (pad & SCE_CTRL_RIGHT))
			{
				switch(selection)
				{
					case 1:
						Menu_Config.fps_keep_display = !Menu_Config.fps_keep_display;
						break;
					case 2:
						Menu_Config.fps_display = !Menu_Config.fps_display;
						break;
				}

				Config_SaveMenuConfig(Menu_Config);
			}
		}

	}
	else if (showVSH == VSH_PROGRAM_MENU)
	{
		if (pad & SCE_CTRL_DOWN)
			selection++;
		else if (pad & SCE_CTRL_UP)
			selection--;
			
		Utils_SetMax(&selection, 0, app_list);
		Utils_SetMin(&selection, app_list, 0);
			
		if (selection == 0)
		{
			if ((pad & SCE_CTRL_CROSS) || (pad & SCE_CTRL_CIRCLE))
			{
				selection = 0;
				showVSH = VSH_MAIN_MENU;
			}
		}
		else
		{
			if (pad & SCE_CTRL_CROSS)
			{
				if (strlen(app_title[selection - 1]) != 0)
					Utils_LaunchAppByUriExit(app_titleID[selection - 1]);
			}		
		}
	}
	else if (showVSH == VSH_SCREEN_FILTER_MENU)
	{
		if (pad & SCE_CTRL_DOWN)
			selection++;
		else if (pad & SCE_CTRL_UP)
			selection--;

		Utils_SetMax(&selection, 0, SCREEN_FILTER_MAX_ITEMS);
		Utils_SetMin(&selection, SCREEN_FILTER_MAX_ITEMS, 0);

		if (((selection == 0) && (pad & SCE_CTRL_CROSS)) || (pad & SCE_CTRL_CIRCLE))
		{
			selection = 0;
			showVSH = VSH_MAIN_MENU;
		}
		else
		{
			switch(selection)
			{
				case 1:
					if ((pad & SCE_CTRL_LEFT) || (pad & SCE_CTRL_RIGHT))
					{
						Menu_Config.screen_filter_keep_enabled = !Menu_Config.screen_filter_keep_enabled;
					}
					Config_SaveMenuConfig(Menu_Config);
					break;
				case 2:
					if ((pad & SCE_CTRL_LEFT) && (Menu_Config.screen_filter_transparency > 0))
					{
						Menu_Config.screen_filter_transparency--;
					}
					else if ((pad & SCE_CTRL_RIGHT) && (Menu_Config.screen_filter_transparency < 255))
					{
						Menu_Config.screen_filter_transparency++;
					}
					Config_SaveMenuConfig(Menu_Config);
					break;
			}
		}
	}
	else if (showVSH == VSH_CONFIRM_MENU)
	{
		if (pad & SCE_CTRL_DOWN)
			selection++;
		else if (pad & SCE_CTRL_UP)
			selection--;

		Utils_SetMax(&selection, 0, CONFIRM_MAX_ITEMS);
		Utils_SetMin(&selection, CONFIRM_MAX_ITEMS, 0);

		if (((selection == 0) && (pad & SCE_CTRL_CROSS)) || (pad & SCE_CTRL_CIRCLE) || assignOperation == 0)
		{
			selection = 0;
			showVSH = VSH_MAIN_MENU;
		}
		else if ((selection == 1) && (pad & SCE_CTRL_CROSS))
		{
			selection = 0;
			showVSH = 0;
			switch(assignOperation)
			{
				case VSH_POWER_REQUEST_STANDBY:
					scePowerRequestStandby();
					break;
				case VSH_POWER_REQUEST_COLDERESET:
					scePowerRequestColdReset();
					break;
				case VSH_RESTART_VSH:
					Utils_RestartVSH();
					break;
			}
		}
	}

	return 0;
}
