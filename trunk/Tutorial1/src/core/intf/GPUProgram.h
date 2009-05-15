#ifndef GPUPROGRAM_H_
#define GPUPROGRAM_H_

#include <File.h>

/**
 * Clasa abstracta pura ce reprezinta un program
 * pentru GPU
 */
class GPUProgram
{
public:
	/**
	* Incarca programul din fisier; primeste ca intrare
	* fisierele sursa pt. vertex si fragment shader
	*/
	virtual bool Load(File* sourceVS, File* sourceFS) = 0;
	
	/**
	* Compileaza programul
	*/
	virtual bool Compile() = 0;

	/**
	* Link-editeaza programul
	*/
	virtual bool Link() = 0;
	
	/**
	* Valideaza programul
	*/
	virtual bool Validate() = 0;
	
	/**
	* Stabileste valoarea pentru un parametru
	* al programului de tip intreg(sau sampler, bool)
	*/
	virtual void SetParameter1i(const char* paramName, int value) = 0;
	
	/**
	* Stabileste valoarea pentru un parametru
	* al programului de tip float
	*/
	virtual void SetParameter1f(const char* paramName, float value) = 0;

	/**
	* Activeaza/dezactiveaza folosirea programului
	*/
	virtual void Use(bool use = true) = 0;
	
	/**
	* Sterge programul si codul sau obiect
	*/
	virtual void Delete() = 0;
	
	/**
	* Pentru polimorfism
	*/
	virtual ~GPUProgram(){};
};

#endif /*GPUPROGRAM_H_*/
