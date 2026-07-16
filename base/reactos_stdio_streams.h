/* ReactOS' msvcrt breaks libstdc++'s file-backed std::basic_filebuf: merely
 * constructing a std::ifstream faults, while std::istringstream,
 * std::ostringstream, std::locale, and std::cout all work.  These drop-in
 * classes read and write whole files through C stdio (fopen/fread/fwrite) and
 * inherit the memory-backed string streams, so every existing `file >> token`
 * and `file << value` call keeps working unchanged.
 *
 * Only compiled into the RZR (Windows/ReactOS) build; other targets keep the
 * standard fstreams via the typedefs at the bottom. */
#ifndef REACTOS_STDIO_STREAMS_H
#define REACTOS_STDIO_STREAMS_H

#include <sstream>
#include <string>
#include <cstdio>

namespace reactball
{

class StdioInStream : public std::istringstream
{
public:
	StdioInStream() {}
	explicit StdioInStream(const char* path) { open(path); }

	void open(const char* path)
	{
		FILE* handle = std::fopen(path, "rb");
		if (handle == 0)
		{
			this->setstate(std::ios::failbit);
			return;
		}
		/* Read in fixed chunks rather than trusting ftell: on ReactOS ftell
		 * can return -1 for some handles, and (size_t)-1 into resize throws
		 * std::length_error.  Growing the buffer as bytes arrive is robust
		 * against non-seekable or mis-sized files. */
		std::string contents;
		char chunk[4096];
		size_t got;
		while ((got = std::fread(chunk, 1, sizeof(chunk), handle)) > 0)
			contents.append(chunk, got);
		std::fclose(handle);
		this->str(contents);
	}

	bool is_open() const { return true; }
	void close() {}
};

class StdioOutStream : public std::ostringstream
{
public:
	StdioOutStream() : m_handle(0) {}
	explicit StdioOutStream(const char* path) : m_handle(0) { open(path); }

	void open(const char* path)
	{
		m_handle = std::fopen(path, "wb");
		if (m_handle == 0)
			this->setstate(std::ios::failbit);
	}

	void close()
	{
		if (m_handle != 0)
		{
			std::string contents = this->str();
			std::fwrite(contents.data(), 1, contents.size(), m_handle);
			std::fclose(m_handle);
			m_handle = 0;
		}
	}

	bool is_open() const { return m_handle != 0; }
	~StdioOutStream() { close(); }

private:
	FILE* m_handle;
};

} // namespace reactball

#endif // REACTOS_STDIO_STREAMS_H
