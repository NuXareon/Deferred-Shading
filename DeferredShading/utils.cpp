#include "utils.h"

void utils::getDir(const std::string path, std::string* dir){
	 std::string::size_type SlashIndex = path.find_last_of("/");
	 if (SlashIndex == std::string::npos) {
        *(dir) = ".";
    }
    else if (SlashIndex == 0) {
        *(dir) = "/";
    }
    else {
        *(dir) = path.substr(0, SlashIndex);
    }
}

bool utils::readFile(const char* path, std::string& out)
{
	std::string line;
	std::ifstream inFile(path);

	if (inFile.is_open()) {
		while (getline(inFile,line)) {
			out.append(line);
			out.append("\n");
		}
		inFile.close();
	} else {
		return false;
	}
	return true;
}

void utils::enableVSyncWin(int i)
{
	#ifdef _WIN32
	const GLubyte* glExt = glGetString(GL_EXTENSIONS);
	if(checkExtension("WGL_EXT_swap_control")) {
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		wglSwapIntervalEXT(i);
	}
	#endif
}

void utils::enableVSyncLinux(int i)
{
	#ifdef __linux__
	const GLubyte* glExt = glGetString(GL_EXTENSIONS);
	if(checkExtension("GLX_EXT_swap_control ")) {
		PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress("glXSwapIntervalEXT");
		glXSwapIntervalEXT(i);
	}
	#endif
}

bool utils::checkExtension(char *ext)
{
    char *p;
    char *end;
    int extNameLen;   

    extNameLen = strlen(ext);
        
    p = (char *)glGetString(GL_EXTENSIONS);
    if (NULL == p) return false;

    end = p + strlen(p);   

    while (p < end) {
        int n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(ext, p, n) == 0)) return true;
        p += (n + 1);
    }
    return false;
}

void utils::drawSphere(double r, int lats, int longs) {
    int i, j;
	double pi = 3.1415927;
    for(i = 0; i <= lats; i++) {
       double lat0 = pi * (-0.5 + (double) (i - 1) / lats);
       double z0  = sin(lat0);
       double zr0 =  cos(lat0);

       double lat1 = pi * (-0.5 + (double) i / lats);
       double z1 = sin(lat1);
       double zr1 = cos(lat1);

       glBegin(GL_QUAD_STRIP);
       for(j = 0; j <= longs; j++) {
           double lng = 2 * pi * (double) (j - 1) / longs;
           double x = cos(lng);
           double y = sin(lng);

           glVertex3f(x * zr0*r, y * zr0*r, z0*r);
           glVertex3f(x * zr1*r, y * zr1*r, z1*r);
       }
       glEnd();
   }
 }

float utils::calcLightRadius(pointLight l, float threshold)
{
	if (l.attenuation.exp == 0.0f) return (threshold*l.intensity.i*glm::max(glm::max(l.color.r,l.color.g),l.color.b)-l.attenuation.constant)/l.attenuation.linear;
	else return (sqrt(-4*l.attenuation.constant*l.attenuation.exp+l.attenuation.linear*l.attenuation.linear+4*l.attenuation.exp*threshold*l.intensity.i*glm::max(glm::max(l.color.r,l.color.g),l.color.b))-l.attenuation.linear)/(2*l.attenuation.exp);
}