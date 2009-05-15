#ifndef LOCALFILE_H_
#define LOCALFILE_H_

#include <File.h>

/**
 * Clasa ce implementeaza interfata File,
 * modeleaza un fisier pe sistemul de fisiere
 * local; este un wrapper pt. functiile fopen, fread, fopen,
 * deoarece acestea sunt independente de platforma
 */
class LocalFile : public File
{
private:
	/**
	* Fisierul deschis in obiectul curent
	*/
	FILE* m_file;

public:
	/**
	* Constructorul implicit, va trebui apelat Open() dupa crearea obiectului
	*/
	LocalFile();
	
	/**
	* Creeaza obiectul si deschide un fisier
	*/
	LocalFile(const char* filename, const char* mode = "rb");
	
	// Mostenite din File
	bool Open(const char* filename, const char* mode = "rb");

	size_t Read(void* data, size_t size, size_t count);

	int Seek(long offset, int origin = SEEK_SET);

	long Tell();

	long GetSize();

	size_t Write(const void* data, size_t size, size_t count);
	
	bool IsOpen();

	void Close();
	
	bool IsStream();

	void* GetInternalData();

	/**
	* Verficica daca exista una numit fisier
	*/
	static bool Exists(const char* filename);

	/**
	* Inchide fisierul
	*/
	virtual ~LocalFile();
};

#endif /*LOCALFILE_H_*/
