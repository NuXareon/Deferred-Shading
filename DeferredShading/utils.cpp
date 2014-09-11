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
