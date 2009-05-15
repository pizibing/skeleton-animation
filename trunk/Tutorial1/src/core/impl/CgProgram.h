#ifndef CGPROGRAM_H_
#define CGPROGRAM_H_

#include <cg\cg.h>							// Header Cg
#include <cg\cggl.h>						// Header Cg specific OpenGL
#include <string>
#include <map>
#include <GPUProgram.h>

using namespace std;

/**
* Implementeaza folosind Cg GPUProgram
*/
class CgProgram : public GPUProgram
{
private:
	/**
	* Context Cg; poate contine mai multe shadere vertex/fragment
	*/
	CGcontext m_cgContext;
	
	/**
	* Vertex shader(program)
	*/
	CGprogram m_cgVertexShader;
	
	/**
	* Fragment shader(program)
	*/
	CGprogram m_cgFragmentShader;
	
	/**
	* Obiectul program ce contine atat fragment shaderul cat si vertex shaderul
	*/
	CGprogram m_cgCombinedShader;

	/**
	* Profil Cg, folosit pentru crearea vertex shader pe GPU
	*/
	CGprofile m_cgVertexProfile;

	/**
	* Profil Cg, folosit pentru crearea fragment shader pe GPU
	*/
	CGprofile m_cgFragmentProfile;
	
	/**
	* Pentru a evita activarea repetata a programului
	*/
	bool m_inUse;

	/**
	* Map pentru pastrarea parametrilor programului Cg
	*/
	map<string, CGparameter> m_cgParametersMap;

public:
	/**
	* Constructor fara parametri; se poate incarca un program cu Load()
	*/
	CgProgram();
	
	/**
	* Constructor ce creeaza si incarca un program Cg
	* dat prin vertex si/sau fragment shader
	*/
	CgProgram(File* sourceVS, File* sourceFS);
	
	/**
	* Incarca parametrii programului Cg si ii pune in map-ul m_cgParametersMap
	*/
	void LoadParameters();
	
	/**
	* Obtinem pointerul la un parametrul dat
	*/
	CGparameter GetParameter(const char* paramName);

	// Mostenite din GPUProgram
	bool Load(File* sourceVS, File* sourceFS);

	bool Compile();

	bool Link();

	bool Validate();

	void SetParameter1i(const char* paramName, int value);

	virtual void SetParameter1f(const char* paramName, float value);

	virtual void SetParameter4f(const char* paramName, float* value);

	virtual void SetParameterStateMatrix(const char* paramName, CGGLenum stateMatrixType, CGGLenum transform);

	void Use(bool use = true);

	void Delete();

	virtual ~CgProgram();
};

#endif