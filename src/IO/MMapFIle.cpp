#include "MMapFIle.h"
#include "Misc/Exceptions.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

namespace PktParser::IO
{
#ifdef _WIN32
	MMapFile::MMapFile(std::string const& filepath) : _data{ nullptr }, _size{ 0 }, _fileHandle{ INVALID_HANDLE_VALUE }, _mappingHandle{ nullptr }
	{
		_fileHandle = CreateFileA(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

		if (_fileHandle == INVALID_HANDLE_VALUE)
			throw ParseException{ "Failed to open: " + filepath };

		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(_fileHandle, &fileSize))
		{
			CloseHandle(_fileHandle);
			throw ParseException{ "Failed to get size: " + filepath };
		}

		_size = static_cast<size_t>(fileSize.QuadPart);

		if (_size == 0)
		{
			CloseHandle(_fileHandle);
			throw ParseException{ "Empty file: " + filepath };
		}

		_mappingHandle = CreateFileMappingA(_fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);

		if (_mappingHandle == nullptr)
		{
			CloseHandle(_fileHandle);
			throw ParseException{ "Failed to create file mapping: " + filepath };
		}

		void* mapped = MapViewOfFile(_mappingHandle, FILE_MAP_READ, 0, 0, 0);

		if (mapped == nullptr)
		{
			CloseHandle(_mappingHandle);
			CloseHandle(_fileHandle);
			throw ParseException{ "Failed to map file: " + filepath };
		}

		_data = static_cast<uint8 const*>(mapped);
	}

	MMapFile::~MMapFile()
	{
		if (_data)
			UnmapViewOfFile(_data);

		if (_mappingHandle)
			CloseHandle(_mappingHandle);

		if (_fileHandle != INVALID_HANDLE_VALUE)
			CloseHandle(_fileHandle);
	}

#else
	MMapFile::MMapFile(std::string const& filepath) : _data{ nullptr }, _size{ 0 }, _fd{ -1 }
	{
		_fd = open(filepath.c_str(), O_RDONLY);
		if (_fd < 0)
			throw ParseException{ "Failed to open: " + filepath };

		struct stat sb;
		if (fstat(_fd, &sb) < 0)
		{
			close(_fd);
			throw ParseException{ "Failed to stat: " + filepath };
		}

		_size = static_cast<size_t>(sb.st_size);

		if (_size == 0)
		{
			close(_fd);
			throw ParseException{ "Empty file: " + filepath };
		}

		void* mapped = mmap(nullptr, _size, PROT_READ, MAP_PRIVATE, _fd, 0);
		if (mapped == MAP_FAILED)
		{
			close(_fd);
			throw ParseException{ "Failed to mmap: " + filepath };
		}

		_data = static_cast<uint8 const*>(mapped);

		madvise(const_cast<uint8*>(_data), _size, MADV_SEQUENTIAL);
	}

	MMapFile::~MMapFile()
	{
		if (_data)
			munmap(const_cast<uint8*>(_data), _size);

		if (_fd >= 0)
			close(_fd);
	}
#endif
}
