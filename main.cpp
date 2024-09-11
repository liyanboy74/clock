#include <time.h>

#include "simlcd.h"
#include "dispcolor.h"
#include "font.h"
#include "color.h"

#include "main.h"

bool trigered=false;

simlcd_buffer_t LCD_BUFFER;
simlcd_buffer_t msaa_buf,div_buf;

time_t rawtime;
struct tm * timeinfo;

bool once_1=true;
static bool cliked=false;
uint8_t tr=150;

uint8_t cc_c=0,cc_t=0;

int simlcd_touch_event(uint32_t x,uint32_t y,uint16_t event)
{
	static int xp,yp,xwp,ywp;
	x*=msaa_buf.scale;
	y*=msaa_buf.scale;
	switch(event)
	{
		case SDL_MOUSEBUTTONDOWN:cliked=true;xwp=x;ywp=y;break;
		case SDL_MOUSEBUTTONUP:cliked=false;break;
		case SDL_MOUSEMOTION:
		if(cliked)
		{
			SDL_GetWindowPosition(msaa_buf.window,&xp,&yp);
			SDL_SetWindowPosition(msaa_buf.window,xp+(x-xwp),yp+(y-ywp));
		}
		break;
	}
    return 0;
}

void draw()
{
	#ifdef _WIN32
	// Seleck bakgront color to hide
	if(cc_c) simlcd_set_color(&LCD_BUFFER,0xfe,0xff,0xfe);
	else simlcd_set_color(&LCD_BUFFER,0x01,0x00,0x01);
	simlcd_draw_rect(&LCD_BUFFER,0,0,LCD_BUFFER.w,LCD_BUFFER.h);
	#endif

	dispcolor_FillCircle(240,240,235,(cc_c)?WHITE:BLACK);
	DrawClock(timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,cc_c,cc_t);
}


#if _WIN32
// Makes a window transparent by setting a transparency color.
bool MakeWindowTransparent(SDL_Window* window, COLORREF colorKey,Uint8 tr) {
	// Get window handle (https://stackoverflow.com/a/24118145/3357935)
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);  // Initialize wmInfo
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hWnd = wmInfo.info.win.window;

	// Change window type to layered (https://stackoverflow.com/a/3970218/3357935)
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	// Set transparency color
	SetLayeredWindowAttributes(hWnd, colorKey, tr, LWA_COLORKEY|LWA_ALPHA);

	return 0;
}
#endif

int loop(int key)
{
	static int i=50,j=-1;

    switch (key)
    {
        case SDL_SCANCODE_SPACE :
			(cc_c)?cc_c=0:cc_c=1;	//Change color
			i=50,j=-1;				//Update
			once_1=true;
			break;
		case SDL_SCANCODE_KP_PLUS:
			if(tr<0xff)tr+=5;
			once_1=true;
			i=50,j=-1;				//Update
			break;
		case SDL_SCANCODE_KP_MINUS:
			if(tr>=10)tr-=5;
			once_1=true;
			i=50,j=-1;				//Update
			break;
        case SDL_SCANCODE_ESCAPE:
			return -1;break;
    }

	time(&rawtime);
	timeinfo=localtime(&rawtime);

	if(j!=timeinfo->tm_sec)
	{
		j=timeinfo->tm_sec;

		draw();

		// dispcolor_Update();

		// simlcd_div(&LCD_BUFFER,&div_buf,4);
		// simlcd_display(&div_buf);

		simlcd_msaa(&LCD_BUFFER,&msaa_buf,4);
		simlcd_display(&msaa_buf);

		if(once_1)
		{
			#if(_WIN32)
			MakeWindowTransparent(msaa_buf.window,cc_c?0xfefffe:0x010001,tr);
			#elif (__linux__)
			SDL_SetWindowOpacity(msaa_buf.window,(float)tr/255.0);
			#endif
			once_1=false;
		}
	}

	if(cliked)simlcd_delay(10);
	else simlcd_delay(200);

    return 0;
}

void simlcd_exit()
{
	simlcd_deinit(&msaa_buf);
	// simlcd_deinit(&div_buf);
}

int main(int argc,char *argv[])
{
    dispcolor_Init(240*2,240*2);
    simlcd_play();
    return 0;
}

static void DrawArrow(int16_t angle, uint8_t lineLen, uint8_t thick,
		uint16_t color) {
	angle -= 90;
	float angleRad = (float) angle * PI / 180;
	int x = cos(angleRad) * lineLen + xC;
	int y = sin(angleRad) * lineLen + yC;

	//dispcolor_DrawLine_Wu(xC, yC, x, y, color);
	dispcolor_DrawLine2(xC, yC, x, y, color, thick);
}

void DrawClock(uint8_t hour, uint8_t min, uint8_t sec, uint8_t light,
		uint8_t secBubbles) {
	uint16_t bgColor, riskColor, digitColor, arrowColor, secArcColor;

	if (light) {
		bgColor = WHITE;
		riskColor = digitColor = arrowColor = BLACK;
		secArcColor = MAGENTA;
	} else {
		bgColor = BLACK;
		riskColor = digitColor = arrowColor = WHITE;
		secArcColor = GREEN;
	}

	// ����� �� ����������
	uint8_t radius1 = 230;
	for (uint16_t angle = 0; angle <= 360; angle += 6) {
		uint8_t riskSize;
		if (!(angle % 90))
			riskSize = 12;
		else if (!(angle % 30))
			riskSize = 9;
		else
			riskSize = 6;

		uint8_t radius2 = radius1 - riskSize;
		float angleRad = (float) angle * PI / 180;
		int x1 = cos(angleRad) * radius1 + xC;
		int y1 = sin(angleRad) * radius1 + yC;
		int x2 = cos(angleRad) * radius2 + xC;
		int y2 = sin(angleRad) * radius2 + yC;

		// dispcolor_DrawLine_Wu(x1, y1, x2, y2, riskColor);
		dispcolor_DrawLine2(x1, y1, x2, y2, riskColor,2);

	}

	// ������� �����
	float scale=2.05;
	dispcolor_DrawString(scale*165, scale*30, FONTID_32F, (char*)&"1", digitColor);//
	dispcolor_DrawString(scale*200, scale*63, FONTID_32F, (char*)&"2", digitColor);//
	dispcolor_DrawString(scale*210, scale*106, FONTID_32F,(char*)&"3", digitColor);
	dispcolor_DrawString(scale*200, scale*160, FONTID_32F,(char*)&"4", digitColor);//
	dispcolor_DrawString(scale*165, scale*193, FONTID_32F,(char*)&"5", digitColor);//
	dispcolor_DrawString(scale*112, scale*205, FONTID_32F,(char*)&"6", digitColor);
	dispcolor_DrawString(scale*65, scale*193, FONTID_32F,(char*)&"7", digitColor);//
	dispcolor_DrawString(scale*32, scale*160, FONTID_32F,(char*)&"8", digitColor);//
	dispcolor_DrawString(scale*17, scale*106, FONTID_32F,(char*)&"9", digitColor);
	dispcolor_DrawString(scale*32, scale*63, FONTID_32F, (char*)&"10", digitColor);//
	dispcolor_DrawString(scale*65, scale*30, FONTID_32F, (char*)&"11", digitColor);//
	dispcolor_DrawString(scale*106, scale*14, FONTID_32F, (char*)&"12", digitColor);

	// dispcolor_printf(75+110, 80+220, FONTID_32F, digitColor,(char*)&"%02d  %02d  %02d", hour,min, sec);

	// ������� �������
	DrawArrow(hour * 30 + min / 2, 40*2.4, 15, arrowColor);//50
	// �������� �������
	DrawArrow(min * 6 + sec / 10, 80*2.2, 10, arrowColor);//100

	// ��������� ����
	// if (!sec)
	// 	sec = 60;
	// if (secBubbles) {
	// 	int16_t startAngle = -90;
	// 	int16_t endAngle = sec * 6 - 90;

	// 	for (int16_t angle = startAngle; angle <= endAngle; angle += 6) {
	// 		float angleRad = (float) angle * PI / 180;
	// 		int x = cos(angleRad) * 235 + xC;
	// 		int y = sin(angleRad) * 235 + yC;

	// 		if (angle == endAngle)
	// 			dispcolor_FillCircleWu(x, y, 4, secArcColor);
	// 		else
	// 			dispcolor_FillCircleWu(x, y, 3, secArcColor);
	// 	}
	// } else
	// 	dispcolor_DrawArc(xC, yC, 230, 0, sec * 6, secArcColor, 4);

	DrawArrow(sec*6,200,2,RED);
	DrawArrow((sec>=30)?(sec-30)*6:(sec+30)*6,30,2,RED);
	dispcolor_FillCircle(xC,yC,10,RED);
}



