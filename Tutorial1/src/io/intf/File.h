#ifndef FILE_H_
#define FILE_H_

#include <cstdio>
	
/**
 * Clasa abstracta pura ce reprezinta 
 * un fisier accesibil engine-ului pe un sistem de fisiere oarecare.
 * Citirea/scrierea se face in stilul lucrului FILE*(fopen, fread...)
 * Fisierele sunt automat inchise sau explicit
 */
class File
{
public:
	/**
	* Deschide un fisier, si il pune in obiectul curent;
	* Aceasi conventie pt. mode ca la fopen("rb"...)
	*/
	virtual bool Open(const char* filename, const char* mode = "rb") = 0;
	
	/**
	* Verfica daca a fost deschis fisierul; mai ales daca obiectul a 
	* fost construit cu constructorul fara parametri
	*/
	virtual bool IsOpen() = 0;

	/**
	* Citeste din fisier si pune in buffer
	*/
	virtual size_t Read(void* data, size_t size, size_t count) = 0;
	
	/**
	* Scrie in fisier; la fel ca fwrite
	*/
	virtual size_t Write(const void* data, size_t size, size_t count) = 0;

	/**
	* Pozitioneaza cursor in fisier, la fel ca si fseek;
	* Foloseste aceasi conventie pt. origin(SEEK_SET...)
	*/
	virtual int Seek(long offset, int origin = SEEK_SET) = 0;
	
	/**
	* Intoarce pozitia curenta in fisier
	*/
	virtual long Tell() = 0;
	
	/**
	* Dimensiunea in octeti a fisierului
	*/
	virtual long GetSize() = 0;

	/**
	* Inchide fisierul
	*/
	virtual void Close() = 0;

	/**
	* Unele tipuri de fisiere contin un stream,
	* pentru acestea returneaza true
	*/
	virtual bool IsStream() = 0;
	
	/**
	* Pentru fisierele care au stream, returneaza stream-ul
	*/
	virtual void* GetInternalData() = 0;

	/**
	* Pentru a folosi polimorfismul destructorilor
	*/
	virtual ~File(){};
};

#endif /*FILE_H_*/
