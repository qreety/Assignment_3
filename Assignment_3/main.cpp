#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <glm.hpp>
#include "gtc/matrix_transform.hpp"
#include "vec3.hpp"
#include <gtc/constants.hpp>
#include <gtx/rotate_vector.hpp>
#include <AntTweakBar.h>
#include <cstdio>

using namespace std;

#define BUFFER_OFFSET(i) ((void*)(i))

struct TVertex_VC
{
	float	x, y, z;
	float	nx, ny, nz;
	float	u, v;
};

struct triangle{
	TVertex_VC v0, v1, v2;
	glm::vec3 face;
};

struct material{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float	shine;
};

struct light{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 direction;
	bool on;
};

GLuint NumTris, material_count, NumV;
GLfloat x_min = FLT_MIN, x_max = FLT_MIN,
y_min = FLT_MIN, y_max = FLT_MIN,
z_min = FLT_MIN, z_max = FLT_MIN; //To calculate the model position
GLfloat dir[3];
static int prvMs = glutGet(GLUT_ELAPSED_TIME);;
bool text, bumps, bumpb;
triangle *Tris;
material *Mate;
light dLight;
TwBar *bar;
GLuint TextureID;

std::vector<unsigned short>	pindex_triangle;
std::vector<TVertex_VC>	pvertex_triangle;
std::vector<glm::vec3> tangents;
std::vector<glm::vec3> bitangents;

std::vector<glm::vec3> indexed_vertices;
std::vector<glm::vec2> indexed_uvs;
std::vector<glm::vec3> indexed_normals;
std::vector<glm::vec3> indexed_tangents;
std::vector<glm::vec3> indexed_bitangents;

GLuint VAOID, IBOID, VBOID;
GLuint sShader, VertShader, FragShader;
GLuint texture, bump1, bump2;
enum Orientation { CCW, CW} ;
Orientation ori = CCW;
enum Display { Wire, Solid };
Display dis = Solid;
glm::mat2 b, bb;

void TW_CALL Reset(void *clientDate);
// loadFile - loads text file into char* fname
// allocates memory - so need to delete after use
// size of file returned in fSize
std::string loadFile(const char *fname)
{
	std::ifstream file(fname);
	if (!file.is_open())
	{
		cout << "Unable to open file " << fname << endl;
		exit(1);
	}

	std::stringstream fileData;
	fileData << file.rdbuf();
	file.close();

	return fileData.str();
}

// printShaderInfoLog
// From OpenGL Shading Language 3rd Edition, p215-216
// Display (hopefully) useful error messages if shader fails to compile
void printShaderInfoLog(GLint shader)
{
	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

	if (infoLogLen > 0)
	{
		infoLog = new GLchar[infoLogLen];
		// error check for fail to allocate memory omitted
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		cout << "InfoLog : " << endl << infoLog << endl;
		delete[] infoLog;
	}
}

GLuint loadBMP(const char * imagepath, int mode){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath, "rb");
	if (!file)							    { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M'){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0)         { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24)         { printf("Not a correct BMP file\n");    return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0 && mode == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (imageSize == 0 && mode == 1)	imageSize = width*height;
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	// Everything is in memory now, the file wan be closed
	fclose(file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	//glPixelStoref(GL_UNPACK_ALIGNMENT, 1);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete[] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

void InitGLStates()
{
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(TRUE);
	glDisable(GL_STENCIL_TEST);
	glStencilMask(0xFFFFFFFF);
	glStencilFunc(GL_EQUAL, 0x00000000, 0x00000001);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClearDepth(1.0);
	glClearStencil(0);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DITHER);
	TwInit(TW_OPENGL_CORE, NULL);

	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	TwGLUTModifiersFunc(glutGetModifiers);
}

void InitializeUI(){
	bar = TwNewBar("UI");
	TwDefine(" UI size='200 400' color='96 216 224' "); // change default tweak bar size and color

	{
		TwEnumVal disEV[2] = { { Wire, "Wireframe" }, { Solid, "Solid" } };
		TwType disType = TwDefineEnum("disType", disEV, 2);
		TwAddVarRW(bar, "DisplayMode", disType, &dis, " keyIncr = '<' keyDecr = '>'");
	}

	{
		TwEnumVal oriEV[2] = { { CW, "CW" }, { CCW, "CCW" } };
		TwType oriType = TwDefineEnum("oriType", oriEV, 2);
		TwAddVarRW(bar, "Orientation", oriType, &ori, " keyIncr = '<' keyDecr = '>'");
	}

	TwAddVarRW(bar, "Texture", TW_TYPE_BOOLCPP, &text, " group = Control");
	TwAddVarRW(bar, "Bump 1", TW_TYPE_BOOLCPP, &bumpb, " group = Control");
	TwAddVarRW(bar, "Bump 2", TW_TYPE_BOOLCPP, &bumps, " group = Control");
	TwAddSeparator(bar, NULL, " group= Control ");
	TwAddVarRW(bar, "Light ON", TW_TYPE_BOOL32, &dLight.on, "group = Light");
	
	
	{
		TwAddVarRW(bar, "Direction", TW_TYPE_DIR3F, &dLight.direction, "group = Light");

		TwAddVarRW(bar, "aRed", TW_TYPE_FLOAT, &dLight.ambient.r, "group = Ambient"
			" min=0 max=1.0 step=0.1");
		TwAddVarRW(bar, "aGreen", TW_TYPE_FLOAT, &dLight.ambient.g, "group = Ambient"
			" min=0 max=1.0 step=0.1");
		TwAddVarRW(bar, "aBlue", TW_TYPE_FLOAT, &dLight.ambient.b, "group = Ambient"
			" min=0 max=1.0 step=0.1");

		TwAddVarRW(bar, "dRed", TW_TYPE_FLOAT, &dLight.diffuse.r, "group = Diffuse"
			" min=0 max=1.0 step=0.1");
		TwAddVarRW(bar, "dGreen", TW_TYPE_FLOAT, &dLight.diffuse.g, "group = Diffuse"
			" min=0 max=1.0 step=0.1");
		TwAddVarRW(bar, "dBlue", TW_TYPE_FLOAT, &dLight.diffuse.b, "group = Diffuse"
			" min=0 max=1.0 step=0.1");

		TwAddVarRW(bar, "sRed", TW_TYPE_FLOAT, &dLight.specular.r, "group = Specular"
			" min=0 max=1.0 step=0.1");
		TwAddVarRW(bar, "sGreen", TW_TYPE_FLOAT, &dLight.specular.g, "group = Specular"
			" min=0 max=1.0 step=0.1");
		TwAddVarRW(bar, "sBlue", TW_TYPE_FLOAT, &dLight.specular.b, "group = Specular"
			" min=0 max=1.0 step=0.1");

		TwDefine("UI/Ambient group = Light");
		TwDefine("UI/Diffuse group = Light");
		TwDefine("UI/Specular group = Light");
	}
}

//Return the minimum value of three values
GLfloat min3(GLfloat x, GLfloat y, GLfloat z){
	return x < y ? (x < z ? x : z) : (y < z ? y : z);
}

//Return the maximum value of three values
GLfloat max3(GLfloat x, GLfloat y, GLfloat z){
	return x > y ? (x > z ? x : z) : (y > z ? y : z);
}

bool readin(char *FileName) {
	const int MAX_MATERIAL_COUNT = 255;
	glm::vec3 ambient[MAX_MATERIAL_COUNT], diffuse[MAX_MATERIAL_COUNT], specular[MAX_MATERIAL_COUNT];
	GLfloat x_temp, y_temp, z_temp;
	GLfloat shine[MAX_MATERIAL_COUNT];
	char ch;
	int m;

	FILE* fp = fopen(FileName, "r");
	if (fp == NULL){
		printf(FileName, "doesn't exist");
		exit(1);
	}

	fscanf(fp, "%c", &ch);
	while (ch != '\n')
		fscanf(fp, "%c", &ch);

	fscanf(fp, "# triangles = %d\n", &NumTris);
	fscanf(fp, "Material count = %d\n", &material_count);

	NumV = NumTris * 3;
	Mate = new material[material_count];
	for (int i = 0; i < material_count; i++){
		fscanf(fp, "ambient color %f %f %f\n", &(Mate[i].ambient.x), &(Mate[i].ambient.y), &(Mate[i].ambient.z));
		fscanf(fp, "diffuse color %f %f %f\n", &(Mate[i].diffuse.x), &(Mate[i].diffuse.y), &(Mate[i].diffuse.z));
		fscanf(fp, "specular color %f %f %f\n", &(Mate[i].specular.x), &(Mate[i].specular.y), &(Mate[i].specular.z));
		fscanf(fp, "material shine %f\n", &(Mate[i].shine));
	}

	fscanf(fp, "%c", &ch);
	while (ch != '\n') // skip documentation line
		fscanf(fp, "%c", &ch);

	fscanf(fp, "%c", &ch);
	while (ch != '\n') // skip documentation line
		fscanf(fp, "%c", &ch);

	Tris = new triangle[NumTris];
	pvertex_triangle.clear();
	for (int i = 0; i<NumTris; i++) // read triangles
	{
		fscanf(fp, "v0 %f %f %f %f %f %f %d %f %f\n",
			&(Tris[i].v0.x), &(Tris[i].v0.y), &(Tris[i].v0.z),
			&(Tris[i].v0.nx), &(Tris[i].v0.ny), &(Tris[i].v0.nz),
			&(m),
			&(Tris[i].v0.u), &(Tris[i].v0.v));
		fscanf(fp, "v1 %f %f %f %f %f %f %d %f %f\n",
			&(Tris[i].v1.x), &(Tris[i].v1.y), &(Tris[i].v1.z),
			&(Tris[i].v1.nx), &(Tris[i].v1.ny), &(Tris[i].v1.nz),
			&(m),
			&(Tris[i].v1.u), &(Tris[i].v1.v));
		fscanf(fp, "v2 %f %f %f %f %f %f %d %f %f\n",
			&(Tris[i].v2.x), &(Tris[i].v2.y), &(Tris[i].v2.z),
			&(Tris[i].v2.nx), &(Tris[i].v2.ny), &(Tris[i].v2.nz),
			&(m),
			&(Tris[i].v2.u), &(Tris[i].v2.v));
		fscanf(fp, "face normal %f %f %f\n", 
			&(Tris[i].face.x), &(Tris[i].face.y), &(Tris[i].face.z));

	//Get the minimum and maximum value to calculate the position
		x_temp = min3(Tris[i].v0.x, Tris[i].v1.x, Tris[i].v2.x);
		y_temp = min3(Tris[i].v0.y, Tris[i].v1.y, Tris[i].v2.y);
		z_temp = min3(Tris[i].v0.z, Tris[i].v1.z, Tris[i].v2.z);
		x_min = x_min < x_temp ? x_min : x_temp;
		y_min = y_min < y_temp ? y_min : y_temp;
		z_min = z_min < z_temp ? z_min : z_temp;

		x_temp = max3(Tris[i].v0.x, Tris[i].v1.x, Tris[i].v2.x);
		y_temp = max3(Tris[i].v0.y, Tris[i].v1.y, Tris[i].v2.y);
		z_temp = max3(Tris[i].v0.z, Tris[i].v1.z, Tris[i].v2.z);
		x_min = x_min > x_temp ? x_min : x_temp;
		y_min = y_min > y_temp ? y_min : y_temp;
		z_min = z_min > z_temp ? z_min : z_temp;

		pvertex_triangle.push_back(Tris[i].v0);
		pvertex_triangle.push_back(Tris[i].v1);
		pvertex_triangle.push_back(Tris[i].v2);
	}

	fclose(fp);
	return true;
}

int LoadShader(const char *pfilePath_vs, const char *pfilePath_fs, bool bindTexCoord0, bool bindNormal, bool bindColor, GLuint &shaderProgram, GLuint &vertexShader, GLuint &fragmentShader)
{
	shaderProgram = 0;
	vertexShader = 0;
	fragmentShader = 0;

	// load shaders & get length of each
	int vlen;
	int flen;
	std::string vertexShaderString = loadFile(pfilePath_vs);
	std::string fragmentShaderString = loadFile(pfilePath_fs);
	vlen = vertexShaderString.length();
	flen = fragmentShaderString.length();

	if (vertexShaderString.empty())
	{
		return -1;
	}

	if (fragmentShaderString.empty())
	{
		return -1;
	}

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char *vertexShaderCStr = vertexShaderString.c_str();
	const char *fragmentShaderCStr = fragmentShaderString.c_str();
	glShaderSource(vertexShader, 1, (const GLchar **)&vertexShaderCStr, &vlen);
	glShaderSource(fragmentShader, 1, (const GLchar **)&fragmentShaderCStr, &flen);

	GLint compiled;

	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
	if (compiled == FALSE)
	{
		cout << "Vertex shader not compiled." << endl;
		printShaderInfoLog(vertexShader);

		glDeleteShader(vertexShader);
		vertexShader = 0;
		glDeleteShader(fragmentShader);
		fragmentShader = 0;

		return -1;
	}

	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
	if (compiled == FALSE)
	{
		cout << "Fragment shader not compiled." << endl;
		printShaderInfoLog(fragmentShader);

		glDeleteShader(vertexShader);
		vertexShader = 0;
		glDeleteShader(fragmentShader);
		fragmentShader = 0;

		return -1;
	}

	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "InVertex");

	if (bindTexCoord0)
		glBindAttribLocation(shaderProgram, 1, "InTexCoord0");

	if (bindNormal)
		glBindAttribLocation(shaderProgram, 2, "InNormal");

	if (bindColor)
		glBindAttribLocation(shaderProgram, 3, "InColor");

	glLinkProgram(shaderProgram);

	GLint IsLinked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, (GLint *)&IsLinked);
	if (IsLinked == FALSE)
	{
		cout << "Failed to link shader." << endl;

		GLint maxLength;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
		if (maxLength>0)
		{
			char *pLinkInfoLog = new char[maxLength];
			glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, pLinkInfoLog);
			cout << pLinkInfoLog << endl;
			delete[] pLinkInfoLog;
		}

		glDetachShader(shaderProgram, vertexShader);
		glDetachShader(shaderProgram, fragmentShader);
		glDeleteShader(vertexShader);
		vertexShader = 0;
		glDeleteShader(fragmentShader);
		fragmentShader = 0;
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;

		return -1;
	}

	return 1;		//Success
}

bool is_near(float v1, float v2){
	return fabs(v1 - v2) < 0.01f;
}

bool getSimilarVertexIndex(
	glm::vec3 & in_vertex,
	glm::vec2 & in_uv,
	glm::vec3 & in_normal,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	unsigned short & result
	){
	// Lame linear search
	for (unsigned int i = 0; i<out_vertices.size(); i++){
		if (
			is_near(in_vertex.x, out_vertices[i].x) &&
			is_near(in_vertex.y, out_vertices[i].y) &&
			is_near(in_vertex.z, out_vertices[i].z) &&
			is_near(in_uv.x, out_uvs[i].x) &&
			is_near(in_uv.y, out_uvs[i].y) &&
			is_near(in_normal.x, out_normals[i].x) &&
			is_near(in_normal.y, out_normals[i].y) &&
			is_near(in_normal.z, out_normals[i].z)
			){
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

void indexVBO_TBN(
	std::vector<TVertex_VC> & triangles,
	std::vector<glm::vec3> & in_tangents,
	std::vector<glm::vec3> & in_bitangents,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec3> & out_tangents,
	std::vector<glm::vec3> & out_bitangents
	){
	// For each input vertex
	for (unsigned int i = 0; i<triangles.size(); i++){

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		glm::vec3 vert = glm::vec3(triangles[i].x, triangles[i].y, triangles[i].z);
		glm::vec2 uv = glm::vec2(triangles[i].u, triangles[i].v);
		glm::vec3 norm = glm::vec3(triangles[i].nx, triangles[i].ny, triangles[i].nz);

		bool found = getSimilarVertexIndex(vert, uv, norm, out_vertices, out_uvs, out_normals, index);

		if (found){ // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);

			// Average the tangents and the bitangents
			out_tangents[index] += in_tangents[i];
			out_bitangents[index] += in_bitangents[i];
		}
		else{ // If not, it needs to be added in the output data.
			out_vertices.push_back(vert);
			out_uvs.push_back(uv);
			out_normals.push_back(norm);
			out_tangents.push_back(in_tangents[i]);
			out_bitangents.push_back(in_bitangents[i]);
			out_indices.push_back((unsigned short)out_vertices.size() - 1);
		}
	}
}

void computeTangentBasis(
	// inputs
	std::vector<TVertex_VC> & triangles,
	// outputs
	std::vector<glm::vec3> & tangents,
	std::vector<glm::vec3> & bitangents
	){

	for (unsigned int i = 0; i<triangles.size(); i += 3){

		// Shortcuts for vertices
		glm::vec3 & v0 = glm::vec3(triangles[i + 0].x, triangles[i + 0].y, triangles[i + 0].z);
		glm::vec3 & v1 = glm::vec3(triangles[i + 1].x, triangles[i + 1].y, triangles[i + 1].z);
		glm::vec3 & v2 = glm::vec3(triangles[i + 2].x, triangles[i + 2].y, triangles[i + 2].z);

		// Shortcuts for UVs
		glm::vec2 & uv0 = glm::vec2(triangles[i + 0].u, triangles[i + 0].v);
		glm::vec2 & uv1 = glm::vec2(triangles[i + 1].u, triangles[i + 1].v);
		glm::vec2 & uv2 = glm::vec2(triangles[i + 2].u, triangles[i + 2].v);

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);

	}

	// See "Going Further"
	for (unsigned int i = 0; i<triangles.size(); i += 1)
	{
		glm::vec3 & n = glm::vec3(triangles[i].nx, triangles[i].ny, triangles[i].nz);
		glm::vec3 & t = tangents[i];
		glm::vec3 & b = bitangents[i];

		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));

		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f){
			t = t * -1.0f;
		}

	}


}

void SetUniform(int programID, glm::vec3 camPos, glm::mat4 ModelMatrix, glm::mat4 ViewMatrix, glm::mat4	MVPMatrix, light dLight, glm::mat3 mv33){
	//MVP
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, FALSE, &MVPMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, FALSE, &ViewMatrix[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(programID, "MV3x3"), 1, GL_FALSE, &mv33[0][0]);

	//Camera position
	glUniform3f(glGetUniformLocation(programID, "camPos"), camPos.x, camPos.y, camPos.z);

	//Light
	glUniform3f(glGetUniformLocation(programID, "light.direction"), -dLight.direction.x, -dLight.direction.y, dLight.direction.z);
	glUniform3f(glGetUniformLocation(programID, "light.ambient"), dLight.ambient.r, dLight.ambient.g, dLight.ambient.b);
	glUniform3f(glGetUniformLocation(programID, "light.diffuse"), dLight.diffuse.r, dLight.diffuse.g, dLight.diffuse.b);
	glUniform3f(glGetUniformLocation(programID, "light.specular"), dLight.specular.r, dLight.specular.g, dLight.specular.b);
	glUniform1i(glGetUniformLocation(programID, "light.on"), dLight.on);
	glUniform3f(glGetUniformLocation(programID, "LightDirection_worldspace"), dLight.direction.x, dLight.direction.y, dLight.direction.z);

	//Material
	glUniform3f(glGetUniformLocation(programID, "material.ambient"), Mate[0].ambient.r, Mate[0].ambient.g, Mate[0].ambient.b);
	glUniform3f(glGetUniformLocation(programID, "material.diffuse"), Mate[0].diffuse.r, Mate[0].diffuse.g, Mate[0].diffuse.b);
	glUniform3f(glGetUniformLocation(programID, "material.specular"), Mate[0].specular.r, Mate[0].specular.g, Mate[0].specular.b);
	glUniform1f(glGetUniformLocation(programID, "material.shine"), Mate[0].shine);

	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(programID, "wall.text"), 1);
	glUniform1i(glGetUniformLocation(programID, "bump1.text"), 2);
	glUniform1i(glGetUniformLocation(programID, "bump2.text"), 3);

	glUniform1i(glGetUniformLocation(programID, "wall.on"), text);
	glUniform1i(glGetUniformLocation(programID, "bump1.on"), bumpb);
	glUniform1i(glGetUniformLocation(programID, "bump2.on"), bumps);
}

void CreateGeometry(){

	for (int i = 0; i < NumV; i++){
		pindex_triangle.push_back(i);
	}

	computeTangentBasis(pvertex_triangle, tangents, bitangents);

	indexVBO_TBN(
		pvertex_triangle, tangents, bitangents,
		pindex_triangle, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents
		);

	//Create the IBO for the triangle
	//16 bit indices
	//We could have actually made one big IBO for both the quad and triangle.
	glGenBuffers(1, &IBOID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumV * sizeof(GLushort), &pindex_triangle[0], GL_STATIC_DRAW);

	GLenum error = glGetError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	error = glGetError();

	//Create VBO for the triangle
	glGenBuffers(1, &VBOID);
	glBindBuffer(GL_ARRAY_BUFFER, VBOID);
	glBufferData(GL_ARRAY_BUFFER, NumV * sizeof(TVertex_VC), &pvertex_triangle[0], GL_STATIC_DRAW);

	GLuint tangentbuffer;
	glGenBuffers(1, &tangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof(glm::vec3), &indexed_tangents[0], GL_STATIC_DRAW);

	GLuint bitangentbuffer;
	glGenBuffers(1, &bitangentbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof(glm::vec3), &indexed_bitangents[0], GL_STATIC_DRAW);

	//Just testing
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	error = glGetError();

	//Second VAO setup *******************
	//This is for the triangle
	glGenVertexArrays(1, &VAOID);
	glBindVertexArray(VAOID);

	//Bind the VBO and setup pointers for the VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBOID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TVertex_VC), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TVertex_VC), BUFFER_OFFSET(sizeof(float)* 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TVertex_VC), BUFFER_OFFSET(sizeof(float)* 6));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
	glVertexAttribPointer(
		3,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// 5th attribute buffer : bitangents
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
	glVertexAttribPointer(
		4,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);
	//Bind the IBO for the VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOID);

	//Just testing
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void display()
{
	glm::mat4 V;
	glm::vec3 camPos;
	glm::vec3 objPos;

	switch (ori)
	{
	case CCW:
		glFrontFace(GL_CCW);
		break;
	case CW:
		glFrontFace(GL_CW);
		break;
	}
	
		V = glm::lookAt(glm::vec3(0.0f, 0.0, 0.0f),
			glm::vec3(0.0f, 0.0f, -2.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		camPos = glm::vec3(0.0, 0.0, 0.0f);
		objPos = glm::vec3(0.0f, 0.0f, -2.0f);

	CreateGeometry();

	glm::mat4 P = glm::perspective(45.0f, 4.0f / 3.0f, 1.0f, 1000.0f);
	glm::mat4 MVP = P * V;
	glm::mat3 MV3 = glm::mat3(MVP);

	//Clear all the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//Bind the shader that we want to use
	GLuint Shader;
	
		Shader = sShader;
	glUseProgram(Shader);
	//Setup all uniforms for your shader
	SetUniform(Shader, camPos, glm::mat4(1.0f), V, MVP, dLight, MV3);
	//Bind the VAO
	glBindVertexArray(VAOID);
	//At this point, we would bind textures but we aren't using textures in this example
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, bump2);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bump1);
	
	//Draw command
	//The first to last vertex is 0 to 3
	//6 indices will be used to render the 2 triangles. This make our quad.
	//The last parameter is the start address in the IBO => zero
	glDrawElements(GL_TRIANGLES, NumV, GL_UNSIGNED_SHORT, 0);

	TwDraw();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	TwWindowSize(w, h);
}

void ExitFunction(int value)
{
	cout << "Exit called." << endl;

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);

	glDeleteBuffers(1, &VBOID);
	glDeleteVertexArrays(1, &VAOID);

	glDetachShader(sShader, VertShader);
	glDetachShader(sShader, FragShader);
	glDeleteShader(VertShader);
	glDeleteShader(FragShader);
	glDeleteProgram(sShader);
	glDeleteTextures(1, &TextureID);
}

int main(int argc, char* argv[])
{
	int i;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	//We want to make a GL 3.3 context
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE);
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(800, 600);
	__glutCreateWindowWithExit("Assignment 3", ExitFunction);

	//Currently, GLEW uses glGetString(GL_EXTENSIONS) which is not legal code
	//in GL 3.3, therefore GLEW would fail if we don't set this to TRUE.
	//GLEW will avoid looking for extensions and will just get function pointers for all GL functions.
	glewExperimental = TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		//Problem: glewInit failed, something is seriously wrong.
		cout << "glewInit failed, aborting." << endl;
		exit(1);
	}

	InitGLStates();
	InitializeUI();

	LoadShader("TextShader.vert", "Texture.frag", false, false, true, sShader, VertShader, FragShader);
	readin("cube_texture.in");
	texture = loadBMP("texture_wall.bmp", 0);
	bump1 = loadBMP("bump1.bmp", 1);
	bump2 = loadBMP("bump2.bmp", 1);
	
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);

	glutMainLoop();
	TwTerminate();
	return 0;
}

void TW_CALL Reset(void *clientDate){
	dLight.direction = { 0.0f, 0.0f, 0.0f };
}