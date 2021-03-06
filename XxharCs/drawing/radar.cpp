#include <fstream>
#include <vector>
#include "radar.h"
#include "gui.h"
#include "../clientdll.h"
#include "../Misc/xorstr.h"
#include "../players.h"
#include "../cvars.h"
#include "../opengl.h"
#include "../drawing/drawing.h"

extern cl_enginefuncs_s gEngfuncs;
extern SCREENINFO screeninfo;
extern int heightenginefont;
int ov_radar_x, ov_radar_y;
float ov_zoom = 5.5f;
bool mapLoaded = false;

#define BOUND_VALUE(var,min,max) if((var)>(max)){(var)=(max);};if((var)<(min)){(var)=(min);}
#define PI 3.14159265358979f
#define CVARRADARY Config::radar_y

void overview_draw_me(float * origin, int radius, int r, int g, int b, int a);

void HUD_DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	gEngfuncs.pTriAPI->Begin(TRI_LINES);

	gEngfuncs.pTriAPI->Color4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

	gEngfuncs.pTriAPI->Brightness(1);
	gEngfuncs.pTriAPI->Vertex3f((float)x1, (float)y1, 0);
	gEngfuncs.pTriAPI->Vertex3f((float)x2, (float)y2, 0);
	gEngfuncs.pTriAPI->End();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

model_s * m_MapSprite = 0;
int xTiles = 1;
int yTiles = 1;

const float screenaspect = (float)(4.0 / 3.0);

struct overviewInfo_s
{
	float	zoom;		// zoom of map images
	int		layers;		// how may layers do we have
	float	origin[3];  // 
	float	layersHeights[1];
	char	layersImages[1][255];
	int		rotated;	// are map images rotated (90 degrees) ?
};

static overviewInfo_s m_OverviewData;

float m_OverviewZoom = 1.0f;

float ov_dx = 3;
float ov_dy = 4;

//-------------------------------------------

bool parse_overview(const std::string& overview_txt)
{
	// defaults
	m_OverviewData.origin[0] = 0.0f;
	m_OverviewData.origin[1] = 0.0f;
	m_OverviewData.origin[2] = 0.0f;
	m_OverviewData.zoom = 1.0f;
	m_OverviewData.layers = 0;
	m_OverviewData.layersHeights[0] = 0.0f;
	m_OverviewData.layersImages[0][0] = 0;

	static std::string path;
	if (path.empty())
	{
		char buffer[MAX_PATH];
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		path = buffer;
		path = path.substr(0, path.rfind('\\'));
		path += "\\cstrike\\overviews\\";
	}

	std::string line;
	std::ifstream file(path + overview_txt + ".txt", std::ios::in|std::ios::beg);
	
	if (file.bad() || !file.is_open())
		return false;

	static auto trim = [](const std::string& s, const std::string& delim = " \r\n\t") -> std::string
	{
		if (s.empty())
			return s;

		std::string result = s;
		for (char c : delim)
		{
			result.erase(0, result.find_first_not_of(c));
			result.erase(result.find_last_not_of(c) + 1);
		}

		return result;
	};

	static auto split = [](const std::string& s, const std::string& delim) ->std::vector<std::string>
	{
		std::vector<std::string> result;
		size_t last = 0;
		size_t index = s.find_first_of(delim, last);
		while (index != std::string::npos)
		{
			result.push_back(s.substr(last, index - last));
			last = index + 1;
			index = s.find_first_of(delim, last);
		}
		if (index - last > 0)
		{
			result.push_back(s.substr(last, index - last));
		}

		return result;
	};

	static auto replace = [](const std::string& s, const std::string& f, const std::string& r) -> std::string
	{
		std::string::size_type pos = 0;
		std::string newString = s;
		while ((pos = newString.find(f, pos)) != std::string::npos)
		{
			newString.replace(pos, f.length(), r.c_str());
			pos = pos + r.length();
		}

		return newString;
	};

	bool inGlobal = false, inLayer = false;
	while (file.good())
	{
		std::getline(file, line);
		if (line.empty())
			continue;

		size_t breaker = line.find("//");
		if (breaker != std::string::npos)
			line = line.substr(0, breaker);

		line = trim(line);
		if (line == "global")
		{
			inGlobal = true;
			continue;
		}
		if (line == "layer")
		{
			inLayer = true;
			continue;
		}
		if (line == "}")
		{
			if(inLayer)
				++m_OverviewData.layers;
			
			inGlobal = false;
			inLayer = false;
			continue;
		}

		if (line == "{")
			continue;

		size_t offset = std::string::npos;
		if (inGlobal)
		{
			if ((offset = line.find("ZOOM")) != std::string::npos)
			{
				line = line.substr(offset + 4);
				line = trim(line);
				m_OverviewZoom = atof(line.c_str());
				m_OverviewData.zoom = m_OverviewZoom;
			}
			else if ((offset = line.find("ORIGIN")) != std::string::npos)
			{
				size_t index = 0;

				line = line.substr(offset + 6);
				line = replace(line, "\t", " ");
				line = replace(line, "  ", " ");
				line = trim(line);

				auto vec = split(line, " ");
				
				for (auto val : vec)
				{
					if (val.empty() || val == " " || index >= 3)
						continue;

					val = trim(val);
					m_OverviewData.origin[index++] = atof(val.c_str());
				}
			}
			else if ((offset = line.find("ROTATED")) != std::string::npos)
			{
				line = line.substr(offset + 7);
				line = trim(line);

				m_OverviewData.rotated = atoi(line.c_str());
			}
		}
		else if (inLayer)
		{
			if ((offset = line.find("IMAGE")) != std::string::npos)
			{
				line = line.substr(offset + 5);
				line = line.substr(line.find('\"') + 1, line.rfind('\"'));
				line = replace(line, "\"", "");
				line = trim(line);

				strcpy_s(m_OverviewData.layersImages[m_OverviewData.layers], line.c_str());
			}
			else if ((offset = line.find("HEIGHT")) != std::string::npos)
			{
				line = line.substr(offset + 6);
				line = trim(line);

				m_OverviewData.layersHeights[m_OverviewData.layers] = atof(line.c_str());
			}
		}
	}

	if (file.is_open())
		file.close();

	/*
	// parse file:
	char token[1024];
	char* pfile = (char *)gEngfuncs.COM_LoadFile(overview_txt, 5, NULL);

	if (!pfile)
	{
		//gConsole.echo("can't open file %s.", overview_txt );
		mapLoaded = false;
		return false;
	}

	while (true)
	{
		pfile = gEngfuncs.COM_ParseFile(pfile, token);
		if (!pfile)
		{
			break;
		}

		if (!strcmp(token, "global"))
		{
			// parse the global data
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			if (strcmp(token, "{"))
			{
				//gConsole.echo("parse error in %s", overview_txt );
				mapLoaded = false;
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			if (!pfile)
			{
				break;
			}

			while (strcmp(token, "}"))
			{
				if (!strcmp(token, "zoom"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.zoom = (float)atof(token);
					m_OverviewZoom = m_OverviewData.zoom;
				}
				else if (!strcmp(token, "origin"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.origin[0] = (float)atof(token);
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.origin[1] = (float)atof(token);
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.origin[2] = (float)atof(token);
				}
				else if (!strcmp(token, "rotated"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					m_OverviewData.rotated = atoi(token);
				}

				pfile = gEngfuncs.COM_ParseFile(pfile, token); // parse next token
			}
		}
		else if (!strcmp(token, "layer"))
		{
			pfile = gEngfuncs.COM_ParseFile(pfile, token);

			if (strcmp(token, "{"))
			{
				//gConsole.echo("parse error in %s", overview_txt );
				mapLoaded = false;
				return false;
			}

			pfile = gEngfuncs.COM_ParseFile(pfile, token);

			while (strcmp(token, "}"))
			{
				if (!strcmp(token, "image"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					strcpy(m_OverviewData.layersImages[m_OverviewData.layers], token);
				}
				else if (!strcmp(token, "height"))
				{
					pfile = gEngfuncs.COM_ParseFile(pfile, token);
					float height = (float)atof(token);
					m_OverviewData.layersHeights[m_OverviewData.layers] = height;
				}
				pfile = gEngfuncs.COM_ParseFile(pfile, token); // parse next token
			}

			m_OverviewData.layers++;
		}
	}
	*/
	return (m_OverviewData.layersImages[0][0] != '\0');
}

void overview_load(const std::string& levelname)
{
	static std::string lastLevelName;
	if (lastLevelName == levelname)
		return;

	std::string newLevelName = levelname;
	if (newLevelName.empty())
		newLevelName = XorStr("de_dust2");

	if (!parse_overview(newLevelName))
	{
		lastLevelName = newLevelName;
		mapLoaded = false;
		return;
	}

	m_MapSprite = gEngfuncs.LoadMapSprite(m_OverviewData.layersImages[0]);
	if (m_MapSprite == nullptr)
	{
		lastLevelName = newLevelName;
		mapLoaded = false;
		return;
	}

	/*
	// dont load same map again
	static char last_levelname[256] = "";
	char overview_txt[256];

	if (!strcmp(last_levelname, levelname)) { return; }

	char tmpMapName[255];
	tmpMapName[0] = '\0';

	// parse file
	if (levelname[0] == NULL)
		strcpy_s(tmpMapName, XorStr("cs_militia"));
	else
		strcpy_s(tmpMapName, levelname);

	sprintf(overview_txt, XorStr("overviews/%s.txt"), tmpMapName);
	bool parse_success = parse_overview(overview_txt);

	if (!parse_success)
	{
		//gConsole.echo("couldnt parse %s",overview_txt);
		strcpy_s(last_levelname, tmpMapName);
		mapLoaded = false;
		return;
	}

	// map sprite
	m_MapSprite = gEngfuncs.LoadMapSprite(m_OverviewData.layersImages[0]);

	if (!m_MapSprite)
	{
		//gConsole.echo("couldnt load %s",m_OverviewData.layersImages[0]);
		strcpy_s(last_levelname, tmpMapName);
		mapLoaded = false;
		return;
	}
	*/

	mapLoaded = true;

	// set additional data	
	float i = m_MapSprite->numframes / (4 * 3);
	i = (int)sqrt(i); // .NET FIX
	xTiles = i * 4;
	yTiles = i * 3;
}

void overview_loadcurrent()
{
	if (!gEngfuncs.pfnGetLevelName)return;

	char levelname[256];

	strcpy(levelname, gEngfuncs.pfnGetLevelName());

	if (strlen(levelname)<5)return;

	levelname[strlen(levelname) - 4] = 0;

	std::string strMapName = levelname;
	if(strMapName.find("maps/") != std::string::npos)
		strMapName = strMapName.substr(strMapName.find("maps/") + 5);
	if (strMapName.rfind(".bsp") != std::string::npos)
		strMapName = strMapName.substr(0, strMapName.rfind(".bsp"));

	overview_load(strMapName);

	if (mapLoaded)
	{
		Config::radar_x = 95;
		Config::radar_y = 95;
		Config::radar_size = 110;
	}
	else
	{
		Config::radar_x = 80;
		Config::radar_y = 80;
		Config::radar_size = 80;
	}
}
bool initi = false;

void overview_calcRadarPoint(const float*, int *, int *);

inline void get_player_tile_coords(float& x, float& y)
{
	if (m_OverviewData.rotated)
	{
		float origin_tilex = (float)(-ov_dy + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[0]);
		float origin_tiley = (float)(ov_dx + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[1]);

		y = (float)(origin_tilex - (1.0 / 1024) * m_OverviewData.zoom * g_local.pmEyePos[0]);
		x = (float)(origin_tiley - (1.0 / 1024) * m_OverviewData.zoom * g_local.pmEyePos[1]);
		y = -y;
	}
	else
	{
		float origin_tilex = (float)(ov_dx + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[0]);
		float origin_tiley = (float)(ov_dy + m_OverviewData.zoom * (1.0 / 1024.0) * m_OverviewData.origin[1]);

		x = (float)(origin_tilex - (1.0 / 1024) * m_OverviewData.zoom * g_local.pmEyePos[0]);
		y = (float)(origin_tiley - (1.0 / 1024) * m_OverviewData.zoom * g_local.pmEyePos[1]);
	}
}
//float mainViewAngles[3];

int clientopenglcalc(int number) // y axis correction :)
{
	return (screeninfo.iHeight - number);
}
void drawcross(int x, int y, int w, int h, int r, int g, int b, int a)
{
	/*
	gEngfuncs.pfnFillRGBA(x - w, y, w * 2, 2, r, g, b, a);
	gEngfuncs.pfnFillRGBA(x, y - h, 2, h, r, g, b, a);

	gEngfuncs.pfnFillRGBA(x - w, y - 1, w, 1, 0, 0, 0, a);
	gEngfuncs.pfnFillRGBA(x + 2, y - 1, w - 2, 1, 0, 0, 0, a);
	gEngfuncs.pfnFillRGBA(x - w, y + 2, w * 2, 1, 0, 0, 0, a);

	gEngfuncs.pfnFillRGBA(x - w - 1, y - 1, 1, 4, 0, 0, 0, a);
	gEngfuncs.pfnFillRGBA(x + w, y - 1, 1, 4, 0, 0, 0, a);

	gEngfuncs.pfnFillRGBA(x - 1, y - h, 1, h - 1, 0, 0, 0, a);
	gEngfuncs.pfnFillRGBA(x + 2, y - h, 1, h - 1, 0, 0, 0, a);

	gEngfuncs.pfnFillRGBA(x - 1, y - h - 1, 4, 1, 0, 0, 0, a);
	*/

	drawing::DrawFillRect(x - w, y, w * 2, 2, COLOR_RGBA(r, g, b, a));
	drawing::DrawFillRect(x, y - h, 2, h, COLOR_RGBA(r, g, b, a));
	drawing::DrawFillRect(x - w, y - 1, w, 1, COLOR_RGBA(0, 0, 0, a));
	drawing::DrawFillRect(x + 2, y - 1, w - 2, 1, COLOR_RGBA(0, 0, 0, a));
	drawing::DrawFillRect(x - w, y + 2, w * 2, 1, COLOR_RGBA(0, 0, 0, a));
	drawing::DrawFillRect(x - w - 1, y - 1, 1, 4, COLOR_RGBA(0, 0, 0, a));
	drawing::DrawFillRect(x + w, y - 1, 1, 4, COLOR_RGBA(0, 0, 0, a));
	drawing::DrawFillRect(x - 1, y - h, 1, h - 1, COLOR_RGBA(0, 0, 0, a));
	drawing::DrawFillRect(x + 2, y - h, 1, h - 1, COLOR_RGBA(0, 0, 0, a));
	drawing::DrawFillRect(x - 1, y - h - 1, 4, 1, COLOR_RGBA(0, 0, 0, a));
}

static bool LoadOvRadar = false;
void overview_redraw()
{
	if (	/*!g_local.alive||*/ gEngfuncs.Con_IsVisible())return;

	if (!LoadOvRadar)
	{
		LoadOvRadar = true;
		overview_loadcurrent();
	}
	int size = (int)Config::radar_size;

	ov_radar_x = (screeninfo.iWidth / 2);
	ov_radar_y = (screeninfo.iHeight / 2);
	g_gui.window(Config::radar_x - size, CVARRADARY - size, 2 * size + 2, 2 * size + 2, 1, "Radar");
	glViewport(Config::radar_x - size, clientopenglcalc(CVARRADARY + size), (size * 2), (size * 2));

	if (m_MapSprite)
	{
		float z = (90.0f - mainViewAngles[0]) / 90.0f;
		z *= m_OverviewData.layersHeights[0]; // gOverviewData.z_min - 32;	

		float xStep = (2 * 4096.0f / ov_zoom) / xTiles;
		float yStep = -(2 * 4096.0f / (ov_zoom*screenaspect)) / yTiles;

		float vStepRight[2];
		float vStepUp[2];
		float angle = (float)((mainViewAngles[1] + 90.0)* (PI / 180.0));

		if (m_OverviewData.rotated) { angle -= float(PI / 2); }

		vStepRight[0] = (float)cos(angle)*xStep;
		vStepRight[1] = (float)sin(angle)*xStep;
		vStepUp[0] = (float)cos(angle + (PI / 2))*yStep;
		vStepUp[1] = (float)sin(angle + (PI / 2))*yStep;

		float tile_x, tile_y;
		get_player_tile_coords(tile_x, tile_y);

		float xs = ov_radar_x - tile_x*vStepRight[0] - tile_y*vStepUp[0];
		float ys = ov_radar_y - tile_x*vStepRight[1] - tile_y*vStepUp[1];

		float inner[2];
		float outer[2];

		outer[0] = xs;
		outer[1] = ys;

		int frame = 0;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
		gEngfuncs.pTriAPI->CullFace(TRI_NONE);

		if(glEnable_detour != nullptr)
			glEnable_detour(GL_BLEND);
		else
			glEnable(GL_BLEND);

		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);//set alpha value here.000

		for (int ix = 0; ix < yTiles; ix++)
		{
			inner[0] = outer[0];
			inner[1] = outer[1];

			for (int iy = 0; iy < xTiles; iy++)
			{
				gEngfuncs.pTriAPI->SpriteTexture(m_MapSprite, frame);
				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(inner[0], inner[1], z);

				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(inner[0] + vStepRight[0], inner[1] + vStepRight[1], z);

				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f(inner[0] + vStepRight[0] + vStepUp[0], inner[1] + vStepRight[1] + vStepUp[1], z);

				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(inner[0] + vStepUp[0], inner[1] + vStepUp[1], z);
				gEngfuncs.pTriAPI->End();

				frame++;

				inner[0] += vStepUp[0];
				inner[1] += vStepUp[1];
			}

			outer[0] += vStepRight[0];
			outer[1] += vStepRight[1];
		}
		glDisable(GL_BLEND);
	}
	glViewport(0, 0, screeninfo.iWidth, screeninfo.iHeight);

	oglSubtractive = true;
	drawcross(Config::radar_x, CVARRADARY, size / 8, size / 8, 255, 255, 255, 254);

	oglSubtractive = false;
}


///////////////////////////////////////////////////////////////////////////////////////////////

void VecRotateZ(float * in, float angle, float * out)
{
	float a, c, s;

	a = (float)(angle * PI / 180);
	c = (float)cos(a);
	s = (float)sin(a);
	out[0] = c*in[0] - s*in[1];
	out[1] = s*in[0] + c*in[1];
	out[2] = in[2];
}

void overview_calcRadarPoint(const float* origin, int * screenx, int * screeny)
{
	if (	/*!g_local.alive ||*/	gEngfuncs.Con_IsVisible())	return;

	float aim[3], newaim[3];

	aim[0] = origin[0] - g_local.pmEyePos[0];
	aim[1] = origin[1] - g_local.pmEyePos[1];
	aim[2] = origin[2] - g_local.pmEyePos[2];

	VecRotateZ(aim, -mainViewAngles[1], newaim);

	*screenx = (Config::radar_x) - int(newaim[1] / ov_zoom * m_OverviewData.zoom * 0.3f * Config::radar_size / 160);
	*screeny = (CVARRADARY)-int(newaim[0] / ov_zoom * m_OverviewData.zoom * 0.4f * Config::radar_size / 160);
}

void overview_draw_entity(const float* origin, int radius, int r, int g, int b, int a)
{
	if (	/*!g_local.alive ||*/	gEngfuncs.Con_IsVisible())	return;

	int x, y, d;

	overview_calcRadarPoint(origin, &x, &y);

	int radius2 = radius << 1;

	d = -4;

	oglSubtractive = true;
	// gEngfuncs.pfnFillRGBA(x, y, radius2 - d, radius2 - d - 1, r, g, b, a);
	drawing::DrawFillRect(x, y, radius2 - d, radius2 - d - 1, COLOR_RGBA(r, g, b, a));
	oglSubtractive = false;
	whiteBorder(x, y, radius2 - d, radius2 - d - 1);
}

void overview_draw_me(float * origin, int radius, int r, int g, int b, int a)
{
	if (	/*!g_local.alive				||*/
		gEngfuncs.Con_IsVisible()
		)	return;
	int x, y;
	overview_calcRadarPoint(origin, &x, &y);
	int radius2 = radius << 1;
	drawcross(x, y, 11, 11, 255, 255, 255, 100);
}

///////////////////////////////////////////////////////////////////////////////////////////////

void DrawRadar()
{
	//int size = (int)80;
	int size = (int)Config::radar_size;

	g_gui.window(Config::radar_x - size, CVARRADARY - size, 2 * size + 2, 2 * size + 2, 1, "Radar");

	// gEngfuncs.pfnFillRGBA(Config::radar_x, CVARRADARY - size, 1, 2 * size, 255, 255, 255, 180);
	// gEngfuncs.pfnFillRGBA(Config::radar_x - size, CVARRADARY, 2 * size, 1, 255, 255, 255, 180);
	drawing::DrawFillRect(Config::radar_x, CVARRADARY - size, 1, 2 * size, COLOR_RGBA(255, 255, 255, 180));
	drawing::DrawFillRect(Config::radar_x - size, CVARRADARY, 2 * size, 1, COLOR_RGBA(255, 255, 255, 180));
}

void drawRadarPoint(const float* origin, int r, int g, int b, int a, bool blink = false, int boxsize = 3)
{
	int screenx;
	int screeny;

	if (mapLoaded)
	{
		overview_calcRadarPoint(origin, &screenx, &screeny);

		if (screenx>(Config::radar_x + Config::radar_size - boxsize - 1)) screenx = Config::radar_x + Config::radar_size - boxsize - 1;
		else if (screenx<(Config::radar_x - Config::radar_size - 1)) screenx = Config::radar_x - Config::radar_size - 1;

		if (screeny>(CVARRADARY + Config::radar_size - boxsize - 1)) screeny = CVARRADARY + Config::radar_size - boxsize - 1;
		else if (screeny<(CVARRADARY - Config::radar_size - 1)) screeny = CVARRADARY - Config::radar_size - 1;

		//if(	!g_local.alive ) return;
	}
	else
	{
		float dx = origin[0] - g_local.pmEyePos[0];
		float dy = origin[1] - g_local.pmEyePos[1];

		float yaw = mainViewAngles[1] * (M_PI_F / 180.0f);
		g_local.sin_yaw = sin(yaw);
		g_local.minus_cos_yaw = -cos(yaw);

		// rotate
		float x = dy*g_local.minus_cos_yaw + dx*g_local.sin_yaw;
		float y = dx*g_local.minus_cos_yaw - dy*g_local.sin_yaw;

		float range = 2500.0f;
		if (fabs(x)>range || fabs(y)>range)
		{
			// clipping
			if (y>x)
			{
				if (y>-x) {
					x = range*x / y;
					y = range;
				}
				else {
					y = -range*y / x;
					x = -range;
				}
			}
			else {
				if (y>-x) {
					y = range*y / x;
					x = range;
				}
				else {
					x = -range*x / y;
					y = -range;
				}
			}
		}

		screenx = Config::radar_x + int(x / range*float(Config::radar_size));
		screeny = CVARRADARY + int(y / range*float(Config::radar_size));
	}

	oglSubtractive = true;
	// gEngfuncs.pfnFillRGBA(screenx - 1, screeny - 1, boxsize, boxsize, r, g, b, a);
	drawing::DrawFillRect(screenx - 1, screeny - 1, boxsize, boxsize, COLOR_RGBA(r, g, b, a));
	oglSubtractive = false;

	if (blink)blackBorder(screenx - 1, screeny - 1, boxsize, boxsize + 1);
}
