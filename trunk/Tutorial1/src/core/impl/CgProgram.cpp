#include <Log.h>
#include "CgProgram.h"

CgProgram::CgProgram() : 
	m_cgContext(0), m_cgVertexShader(0), m_cgFragmentShader(0), 
	m_cgVertexProfile(CG_PROFILE_UNKNOWN), m_cgFragmentProfile(CG_PROFILE_UNKNOWN)
{
}

CgProgram::CgProgram(File* sourceVS, File* sourceFS)
{
	CgProgram();
	(void)this->Load(sourceVS, sourceFS);
}

bool CgProgram::Load(File* sourceVS, File* sourceFS)
{
	// Am incarcat deja un program
	if (m_cgVertexShader || m_cgFragmentShader)
	{
		return false;
	}

	// Nu am primit nici un fisier valid
	if ((!sourceVS || !sourceVS->IsOpen()) || (!sourceFS || !sourceFS->IsOpen()))
	{
		return false;
	}

	// Cream contextul Cg
	m_cgContext = cgCreateContext();
	if (m_cgContext == NULL)
	{
		return false; // A esuat crearea contextului
	}
	
	// Nu vrem sa se autocompileze programele
	cgSetAutoCompile(m_cgContext, CG_COMPILE_MANUAL);

	// Citim continutul programul vertex din fisier si cream vertex shader-ul
	if (sourceVS && sourceVS->IsOpen())
	{
		// Cream intai un vertex profile
		m_cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
		if (m_cgVertexProfile == CG_PROFILE_UNKNOWN)
		{
			// Nu exista un profil valid
			return false;
		}

		int size = sourceVS->GetSize();
		char* sourceBuffer = new char[size + 1];
		sourceVS->Read(sourceBuffer, 1, size);
		sourceBuffer[size] = 0;	// Adaugam terminatorul de sir
									
		// Cream programul Cg
		m_cgVertexShader = 
			cgCreateProgram(m_cgContext, CG_SOURCE, sourceBuffer, m_cgVertexProfile, "vertex", NULL);

		// Eliberam resursele
		delete[] sourceBuffer;

		if (m_cgVertexShader == NULL)
		{
			// A esuat crearea programului
			CGerror error = cgGetError();
			
			// Logam eroarea si iesim
			LogError("Cg vertex shader creation failed: %s\n", cgGetErrorString(error));
			return false;
		}
	}

	// Citim continutul programul vertex din fisier si cream fragment shader-ul
	if (sourceFS && sourceFS->IsOpen())
	{
		// Cream intai un fragment profile
		m_cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
		if (m_cgFragmentProfile == CG_PROFILE_UNKNOWN)
		{
			// Nu exista un profil valid
			return false;
		}

		int size = sourceFS->GetSize();
		char* sourceBuffer = new char[size + 1];
		sourceFS->Read(sourceBuffer, 1, size);
		sourceBuffer[size] = 0;	// Adaugam terminatorul de sir

		// Cream programul Cg
		m_cgFragmentShader = 
			cgCreateProgram(m_cgContext, CG_SOURCE, sourceBuffer, m_cgFragmentProfile, "fragment", NULL);
		
		if (m_cgFragmentShader == NULL)
		{
			// A esuat crearea programului
			CGerror error = cgGetError();
			
			// Logam eroarea si iesim
			LogError("Cg fragment shader creation failed: %s\n", cgGetErrorString(error));
			return false;
		}

		// Eliberam resursele
		delete[] sourceBuffer;
	}

	return true; // succes
}

bool CgProgram::Compile()
{
	// Compilam intai vertex shader-ul daca exista
	if (m_cgVertexShader && !cgIsProgramCompiled(m_cgVertexShader))
	{
		// Optiuni compilator pe baza profilului
		cgGLSetOptimalOptions(m_cgVertexProfile);
		
		// COmpilam shader-ul
		cgCompileProgram(m_cgVertexShader);
		
		// Verificam compilarea
		CGerror error = cgGetError();
		if (error != CG_NO_ERROR)
		{
			// Logam eroarea si iesim
			LogError("Cg vertex shader compilation failed: %s\n", cgGetErrorString(error));
			return false;
		}
		else
		{
			LogMessage("Cg vertex shader compilation successful\n");
		}
	}
	
	// Compilam fragment shader-ul
	if (m_cgFragmentShader && !cgIsProgramCompiled(m_cgFragmentShader))
	{
		// Optiuni compilator pe baza profilului
		cgGLSetOptimalOptions(m_cgFragmentProfile);
		
		// Compilam shader-ul
		cgCompileProgram(m_cgFragmentShader);
		
		// Verificam compilarea
		CGerror error = cgGetError();
		if (error != CG_NO_ERROR)
		{
			// Logam eroarea si iesim
			LogError("Cg fragment shader compilation failed: %s\n", cgGetErrorString(error));
			return false;
		}
		else
		{
			LogMessage("Cg fragment shader compilation successful\n");
		}
	}

	return true; // Succes
}

bool CgProgram::Link()
{
	// Combinam vertex si fragment shader-ul intr-un singur shader(program);
	// numai daca exista si vertex si fragment shader
	if (m_cgVertexShader && m_cgFragmentShader)
	{
		CGprogram shaders[] = { m_cgVertexShader, m_cgFragmentShader };
		m_cgCombinedShader = cgCombinePrograms(2, shaders);

		CGerror error = cgGetError();
		if (error != CG_NO_ERROR)
		{
			LogError("Cg program link error: %s\n", cgGetErrorString(error));
			return false;
		}

		// Distrugem shader-ele de care nu mai avem nevoie
		cgDestroyProgram(m_cgVertexShader);
		cgDestroyProgram(m_cgFragmentShader);
	}
	else if (m_cgVertexShader)
	{
		m_cgCombinedShader = m_cgVertexShader;
	}
	else if (m_cgFragmentShader)
	{
		m_cgCombinedShader = m_cgFragmentShader;
	}

	// Incarcam shader-ul, pregatind-ul pt. binding
	cgGLLoadProgram(m_cgCombinedShader);

	LogMessage("Cg shaders linking successful\n");
	return true;
}

void CgProgram::Use(bool use)
{
	if (!m_cgCombinedShader)
	{
		return;
	}
	
	if (use)
	{
		if (m_inUse)
		{
			return; // Deja activ
		}

		// Activam profilurile necesare
		if (m_cgVertexProfile != CG_PROFILE_UNKNOWN)
		{
			cgGLEnableProfile(m_cgVertexProfile);
		}
		if (m_cgFragmentProfile != CG_PROFILE_UNKNOWN)
		{
			cgGLEnableProfile(m_cgFragmentProfile);
		}

		// Activarea programului propriu-zisa
		cgGLBindProgram(m_cgCombinedShader);
	}
	else
	{
		// Dezactivam profilurile
		if (m_cgVertexProfile != CG_PROFILE_UNKNOWN)
		{
			cgGLDisableProfile(m_cgVertexProfile);
		}
		if (m_cgFragmentProfile != CG_PROFILE_UNKNOWN)
		{
			cgGLDisableProfile(m_cgFragmentProfile);
		}
		
		// Dezactivam programul
		cgGLUnbindProgram(m_cgVertexProfile);
		cgGLUnbindProgram(m_cgFragmentProfile);
	}
}

bool CgProgram::Validate()
{
	// API-ul de Cg pt. OpenGL nu are metode validare a programului(cum exista pt. GLSL)
	// asa ca facem o validare simpla
	LogWarning("CgProgram::Validate() calling this method is useless\n");
	return m_cgCombinedShader != NULL;
}

void CgProgram::LoadParameters()
{
	if (m_cgCombinedShader == NULL)
	{
		return; // Nu avem niciun program incarcat
	}

	CGenum namespaces[] = { CG_GLOBAL, CG_PROGRAM };
	for (int namespaceIdx = 0; namespaceIdx < 2; namespaceIdx++)
	{
		// Incarcam toti parametrii din namespace
		CGparameter param = cgGetFirstParameter(m_cgCombinedShader, namespaces[namespaceIdx]);
		while (param)
		{
			// Numele parametrului
			string name = string(cgGetParameterName(param));
			
			// Il punem in map
			m_cgParametersMap[name] = param;
			
			// Urmatorul parametru
			param = cgGetNextParameter(param);
		}
	}
}

CGparameter CgProgram::GetParameter(const char* paramName)
{
	// Folosim pointerii la parametri deja stocati
	string parameterName(paramName);
	if (m_cgParametersMap.find(parameterName) != m_cgParametersMap.end())
	{
		return m_cgParametersMap[parameterName];
	}
	else
	{
		return cgGetNamedParameter(m_cgCombinedShader, paramName);
	}
}

void CgProgram::SetParameter1f(const char* paramName, float value)
{
	CGparameter param = this->GetParameter(paramName);
	if (param)
	{
		cgSetParameter1f(param, value);
	}
	else
	{
		LogError("Invalid Cg parameter: %s\n", paramName);
	}
}

void CgProgram::SetParameter4f(const char* paramName, float* value)
{
	CGparameter param = this->GetParameter(paramName);
	if (param)
	{
		cgSetParameter4f(param, value[0], value[1], value[2], value[3]);
	}
	else
	{
		LogError("Invalid Cg parameter: %s\n", paramName);
	}
}

void CgProgram::SetParameterStateMatrix(const char* paramName, CGGLenum stateMatrixType, CGGLenum transform)
{
	CGparameter param = this->GetParameter(paramName);
	if (param)
	{
		cgGLSetStateMatrixParameter(param, stateMatrixType , transform);
	}
	else
	{
		LogError("Invalid Cg parameter: %s\n", paramName);
	}
}

void CgProgram::SetParameter1i(const char* paramName, int value)
{
	CGparameter param = this->GetParameter(paramName);
	if (param)
	{
		cgSetParameter1i(param, value);
	}
	else 
	{
		LogError("Invalid Cg parameter: %s\n", paramName);
	}
}

void CgProgram::Delete()
{
	this->Use(false);
	if (m_cgCombinedShader)
	{
		cgDestroyProgram(m_cgCombinedShader);
	}
	if (m_cgContext)
	{
		cgDestroyContext(m_cgContext);
	}
}

CgProgram::~CgProgram(void)
{
	this->Delete();	
}