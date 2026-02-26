#ifndef MMAP_FILE_H
#define MMAP_FILE_H

#include "Misc/Define.h"

#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#endif

namespace PktParser::IO
{
	class MMapFile
	{
	private:
		uint8 const* _data;
		size_t _size;

#ifdef _WIN32
		HANDLE _fileHandle;
		HANDLE _mappingHandle;
#else
		int _fd;
#endif

	public:
		explicit MMapFile(std::string const& filepath);
		~MMapFile();

		MMapFile(MMapFile const&) = delete;
		MMapFile& operator=(MMapFile const&) = delete;
		MMapFile(MMapFile&&) = delete;
		MMapFile& operator=(MMapFile&&) = delete;

		uint8 const* GetData() const { return _data; }
		size_t GetSize() const { return _size; }
	};
}

#endif