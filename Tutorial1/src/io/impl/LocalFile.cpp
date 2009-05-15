#include <Log.h>
#include "LocalFile.h"

LocalFile::LocalFile() : m_file(NULL)
{
}

LocalFile::LocalFile(const char* filename, const char* mode)
{
	this->Open(filename, mode);
}

bool LocalFile::Open(const char* filename, const char* mode)
{
	m_file = fopen(filename, mode);
	if (!m_file)
	{
		LogError("Failed to open local file %s", filename);
		return false;
	}
	else
	{
		return true;
	}
}

inline bool LocalFile::IsOpen()
{
	return m_file != NULL;
}

inline size_t LocalFile::Read(void* data, size_t size, size_t count)
{
	return fread(data, size, count, m_file);
}

inline size_t LocalFile::Write(const void* data, size_t size, size_t count)
{
	return fwrite(data, size, count, m_file);
}

inline int LocalFile::Seek(long offset, int origin)
{
	return fseek(m_file, offset, origin);
}

inline long LocalFile::Tell()
{
	return ftell(m_file);
}

long LocalFile::GetSize()
{
	// Salvam pozitia curenta in fisier
	long savedPos = this->Tell();

	// Pozitionam la sfarsitul fisierului
	// si obtinem dimensiunea
	this->Seek(0, SEEK_END);
	long size = this->Tell();

	// Revenim la pozitia anterioara
	this->Seek(savedPos);

	return size;
}

bool LocalFile::Exists(const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (file)
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}

inline void LocalFile::Close()
{
	// Inchide fisierul
	if (m_file)
	{
		fclose(m_file);
		m_file = NULL;
	}
}

inline bool LocalFile::IsStream()
{
	return false;
}

inline void* LocalFile::GetInternalData()
{
	return m_file;
}

LocalFile::~LocalFile()
{
	// Inchide fisierul
	if (m_file)
	{
		fclose(m_file);
	}
}

