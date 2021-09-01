/*
Author: Emre Demircan
Date: 2021-08-31
Github: emrecpp
Version: 1.0.2

*/
#ifndef PACKET_H
#define PACKET_H
#define _WINSOCKAPI_    // stops windows.h including winsock.h

#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <iomanip> // Print: setfill, setw
#include <sstream> // Print: std::stringstream
#pragma comment(lib, "Ws2_32.lib")

#ifdef _WIN32
#define INLINE __forceinline
#else
#define INLINE inline
#endif
using std::string; using std::vector;
#pragma warning(disable:4996) //#define _CRT_SECURE_NO_WARNINGS
class ByteBuffer
{

public:
	bool isLittleEndian = false;

	const static size_t DEFAULT_SIZE = 32;
	const static int INDEX_OF_FLAG = 2;
	const static int INDEX_OF_COUNT_ELEMENTS = 3;



	ByteBuffer() : _rpos(0), _wpos(0) { _storage.reserve(DEFAULT_SIZE); }
	ByteBuffer(size_t res, bool enabledLittleEndian = false) : _rpos(0), _wpos(0), isLittleEndian(enabledLittleEndian) { _storage.reserve(res <= 0 ? DEFAULT_SIZE : res); }
	ByteBuffer(const ByteBuffer& buf) : _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage) { }
	virtual ~ByteBuffer() {}



	template <typename T> void append(T value) {
		// Hard coded, no reversing bytes
		append((uint8_t*)&value, sizeof(value));
	}
	template <typename T> void put(size_t pos, T value) { put(pos, (uint8_t*)&value, sizeof(value)); }

	ByteBuffer& operator<<(bool value) { append(&value, 1); increaseItemCount(); return *this; }

	ByteBuffer& operator<<(uint8_t value)
	{
		if (!isLittleEndian) reverseBytes(&value);
		append<uint8_t>(value); return *this;
	}
	ByteBuffer& operator<<(uint16_t value) { if (!isLittleEndian) reverseBytes(&value); append<uint16_t>(value); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(uint32_t value) { if (!isLittleEndian) reverseBytes(&value); append<uint32_t>(value); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(uint64_t value) { if (!isLittleEndian) reverseBytes(&value); append<uint64_t>(value); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(int8_t value) { append<int8_t>(value); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(int16_t value) { if (!isLittleEndian) reverseBytes(&value); append<int16_t>(value); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(int32_t value) { if (!isLittleEndian) reverseBytes(&value); append<int32_t>(value); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(int64_t value) { if (!isLittleEndian) reverseBytes(&value); append<int64_t>(value); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(float value) { append<float>(value); increaseItemCount(); return *this; }
	INLINE void increaseItemCount() {
		if (this->size() < INDEX_OF_COUNT_ELEMENTS) return;

		if (_storage.at(INDEX_OF_COUNT_ELEMENTS) + 1 < 255)
			_storage[INDEX_OF_COUNT_ELEMENTS] += 1;
	}

	template <typename T> /* Custom Struct/Class, don't use std::string because maximum allowed character size is 16. bigger than 16 bytes will be corrupted.*/
	ByteBuffer& operator<<(T value)
	{
		/*if (!isLittleEndian)
			reverseBytes(&value, sizeof(value));*/
		append<T>(value); return *this;
	}

	ByteBuffer& operator<<(ByteBuffer& value)
	{
		if (!isLittleEndian) reverseBytes(&value);
		if (value.wpos())
			append(value.contents(), value.wpos());
		return *this;
	}

	const uint8_t* contents() const { return &_storage[0]; }

	ByteBuffer& operator>>(bool& value) { value = read<char>() > 0 ? true : false; return *this; }
	ByteBuffer& operator>>(uint8_t& value) { value = read<uint8_t>(); if (!isLittleEndian) reverseBytes(&value); return *this; }
	ByteBuffer& operator>>(uint16_t& value) { value = read<uint16_t>(); if (!isLittleEndian) reverseBytes(&value); return *this; }
	ByteBuffer& operator>>(uint32_t& value) { value = read<uint32_t>(); if (!isLittleEndian) reverseBytes(&value); return *this; }
	ByteBuffer& operator>>(uint64_t& value) { value = read<uint64_t>(); if (!isLittleEndian) reverseBytes(&value); return *this; }
	ByteBuffer& operator>>(int8_t& value) { value = read<int8_t>(); if (!isLittleEndian) reverseBytes(&value); return *this; }
	ByteBuffer& operator>>(int16_t& value) { value = read<int16_t>(); if (!isLittleEndian) reverseBytes(&value); return *this; }
	ByteBuffer& operator>>(int32_t& value) { value = read<int32_t>();  if (!isLittleEndian) reverseBytes(&value); return *this; }
	ByteBuffer& operator>>(int64_t& value) { value = read<int64_t>(); if (!isLittleEndian) reverseBytes(&value); return *this; }

	ByteBuffer& operator>>(float& value) { value = read<float>(); if (!isLittleEndian) reverseBytes(&value); return *this; }


	ByteBuffer& operator<<(vector<string>& value)
	{
		uint16_t len = (uint16_t)value.size();
		append<uint16_t>(len);

		for (size_t i = 0; i < value.size(); ++i) {
			string element = value.data()[i];
			int elementLength = element.size();

			append<uint16_t>(elementLength);
			append(element.c_str(), elementLength);
		}
		return *this;
	}


	ByteBuffer& operator>>(vector<string>& value) {
		uint16_t len = read<uint16_t>();

		if (_rpos + len <= size())
		{
			for (uint8_t i = 0; i < len; i++)
			{
				uint16_t sizeElement = read<uint16_t>();

				char* buffer = (char*)malloc(sizeElement + 1);
				memset(buffer, 0, sizeElement + 1);
				read(buffer, sizeElement);
				value.push_back(buffer);
				delete[]buffer;
			}
		}
		return *this;
	}
	template <typename T>
	ByteBuffer& operator<<(vector<T>& value)
	{
		if (!isLittleEndian) reverseBytes(&value);
		uint16_t len = value.size();
		append<uint16_t>(len);

		for (int i = 0; i < value.size(); ++i) {
			T element = value.data()[i];
			//int elementLength = sizeof(element);

			//append<uint16_t>(elementLength);
			append<T>(element);
		}
		return *this;
	}

	template <typename T>
	ByteBuffer& operator>>(vector<T>& value) {
		uint16_t len = read<uint16_t>();

		if (_rpos + len <= size())
		{
			for (uint8_t i = 0; i < len; i++)
			{
				//uint16_t sizeElement = read<uint16_t>();
				uint16_t sizeElement = sizeof(T);

				T* buffer = (T*)malloc(sizeElement);
				memset(buffer, 0, sizeElement);
				read(buffer, sizeElement);
				value.push_back(*buffer);
				delete buffer;

			}
		}
		return *this;
	}

	ByteBuffer& operator<<(const std::string& value) { *this << value.c_str(); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(std::string& value) { *this << value.c_str(); increaseItemCount(); return *this; }
	ByteBuffer& operator<<(const char* str)
	{
		increaseItemCount();
		uint32_t len = (uint32_t)strlen(str);

		auto var = ntohl(len);
		append((uint16_t*)&var, 4);

		append((uint16_t*)str, len);
		return *this;
	}

	ByteBuffer& operator<<(char* str) { *this << (const char*)str; increaseItemCount(); return *this; }

	ByteBuffer& operator>>(std::string& value)
	{
		//uint16_t len;		
		uint32_t len;
		if (!value.empty())
			value.clear();
		len = htonl(read<uint32_t>());

		if (_rpos + len <= size())
		{
			for (uint32_t i = 0; i < len; i++)
				value.push_back(read<char>());
		}
		return *this;
	}


	template <typename T>
	ByteBuffer& operator>>(T& value)
	{
		uint16_t len = sizeof(T);
		value = T();

		if (_rpos + len <= size())
		{
			for (uint8_t i = 0; i < len; i++)
				*((uint8_t*)&value + i) = read<uint8_t>();
		}
		return *this;
	}
	uint8_t operator[](size_t pos) { return read<uint8_t>(pos); }

	INLINE size_t rpos() const { return _rpos; };
	INLINE size_t rpos(size_t rpos) { return _rpos = rpos; };
	INLINE size_t wpos() const { return _wpos; };
	INLINE size_t wpos(size_t wpos) { return _wpos = wpos; };

	template <typename T> T read()
	{
		if (_rpos + sizeof(T) > size())
			return NULL;
		T r = read<T>(_rpos);

		_rpos += sizeof(T);
		return r;
	}



	template <typename T> T read(size_t pos) const
	{
		//ASSERT(pos + sizeof(T) <= size());
		if (pos + sizeof(T) > size())
			return (T)0;

		return *((T*)&_storage[pos]);
	}

	void read(void* dest, size_t len)
	{
		if (_rpos + len <= size())
			memcpy(dest, &_storage[_rpos], len);
		else // throw error();
			memset(dest, 0, len);
		_rpos += len;
	}

	void readString(std::string& dest)
	{
		readString(dest, true);
	}

	void readString(std::string& dest, size_t len)
	{
		dest.clear();
		dest.assign(len, '\0');

		if (_rpos + len <= size())
			read(&dest[0], len);
	}

	INLINE size_t size() const { return _storage.size(); }


	void resize(size_t newsize)
	{
		_storage.resize(newsize);
		_rpos = 6;
		_wpos = size();
	}

	void reserve(size_t ressize) { if (ressize > size()) _storage.reserve(ressize); };


	INLINE void append(const std::string& str) { append((uint8_t*)str.c_str(), str.size() + 1); }
	INLINE void append(const char* src, size_t cnt) { return append((const uint8_t*)src, cnt); }
	INLINE void append(const void* src, size_t cnt)
	{
		if (!cnt)
			return;

		if (_storage.size() < _wpos + cnt)
			_storage.resize(_wpos + cnt);
		/*if (!isLittleEndian) {

			reverseBytes((void*)(src), cnt);
		}*/

		memcpy(&_storage[_wpos], src, cnt);
		_wpos += cnt;
	}
	INLINE void reverseBytes(void* start, int size = 0) {
		unsigned char* lo = (unsigned char*)start;
		if (size == 0)
			size = sizeof(start);
		unsigned char* hi = (unsigned char*)start + size - 1;
		unsigned char swap;
		while (lo < hi) {
			swap = *lo;
			*lo++ = *hi;
			*hi-- = swap;
		}
	}
	INLINE void append(const ByteBuffer& buffer) { if (buffer.size() > 0) append(buffer.contents(), buffer.size()); }
	INLINE void append(const ByteBuffer& buffer, size_t len)
	{
		//ASSERT(buffer.rpos() + len <= buffer.size());
		append(buffer.contents() + buffer.rpos(), len);
	}

	void readFrom(ByteBuffer& buffer, size_t len)
	{
		//ASSERT(buffer.rpos() + len <= buffer.size());
		append(buffer.contents() + buffer.rpos(), len);
		buffer.rpos(buffer.rpos() + len);
	}

	void put(size_t pos, const void* src, size_t cnt)
	{
		//ASSERT(pos + cnt <= size());
		memcpy(&_storage[pos], src, cnt);
	}

protected:
	// read and write positions
	size_t _rpos, _wpos;
	std::vector<uint8_t> _storage;
};
class Packet : public ByteBuffer
{
public:
	INLINE Packet() : ByteBuffer()
	{
		*this = Packet(0);
	}
	bool operator==(Packet TargetPacket) {
		if (TargetPacket.size() != this->size())
			return false;
		vector<uint8_t>& TargetStorage = TargetPacket.storage();
		for (size_t i = 0; i < this->size(); i++) {
			if (_storage.at(i) != TargetStorage.at(i))
				return false;
		}
		return true;
	}
	INLINE Packet(uint16_t opcode, bool enabledLittleEndian = false) : ByteBuffer(4096, enabledLittleEndian)
	{
		reverseBytes(&opcode, 2);
		append(&opcode, 2);
		append(new const char[] {0, 0, 0, 0}, 4);
		_rpos = 6;
	}

	INLINE Packet(uint16_t opcode, size_t res) : ByteBuffer(res)
	{
		reverseBytes(&opcode, 2);
		append(&opcode, 2);
		append(new const char[] {0, 0, 0, 0}, 4);
		_rpos = 6;
	}

	INLINE Packet(const Packet& packet) : ByteBuffer(packet)
	{
	}
	void clear(uint16_t opcode = 0)
	{
		//if (size() > 0 && GetOpcode() != 0)
		if (opcode == 0)
			opcode = this->GetOpcode();
		_storage.clear();
		_rpos = _wpos = 0;

		reverseBytes(&opcode, 2);
		append(&opcode, 2);
		append(new const char[] {0, 0, 0, 0}, 4);
		_rpos = _wpos = 6;

	}
	INLINE char* getElements(int StartIndex, int EndIndex) {
		char* data = (char*)(malloc((sizeof(char)) * (EndIndex - StartIndex + 1)));
		memset(data, '\0', EndIndex - StartIndex + 1);

		for (int i = StartIndex; i < EndIndex; i++)
			data[i - StartIndex] = _storage.at(i);

		return data;
	}
	INLINE uint32_t byteArrayToUInt(char arr[4]) {
		return (arr[3] << 24) | (arr[2] << 16) | (arr[1] << 8) | (arr[0]);
	}
	INLINE uint16_t GetOpcode()
	{
		if (size() < 2) return 0;

		uint16_t opcode = (uint16_t)byteArrayToUInt(getElements(0, 2));
		reverseBytes(&opcode, 2);
		return opcode;
	}

	INLINE Packet Initialize(uint16_t opcode)
	{
		clear(opcode); // reverseBytes(&opcode,2);
		return *this;
	}


	INLINE std::vector<uint8_t>& storage() {
		return this->_storage;
	}
	INLINE void setStorage(std::vector<uint8_t> storage) {
		this->_storage = storage;
	}
	enum SocketReturn : int {
		SUCCESS = 1,
		FAIL = 0,
		CONNECTION_LOST = -1
	};
private:
	bool Recv(SOCKET s, void* destination, int numberOfBytes, int& bytesReceived, int& SocketRet)
	{
		bytesReceived = recv(s, (char*)destination, numberOfBytes, NULL);
		if (bytesReceived == -1) {
			SocketRet = SocketReturn::CONNECTION_LOST;
			return false;
		}
		if (bytesReceived == 0) //If connection was gracefully closed		
		{
			SocketRet = SocketReturn::FAIL;
			return false;
		}


		if (bytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			SocketRet = SocketReturn::FAIL;
			return false;
		}
		SocketRet = SocketReturn::SUCCESS;
		return true;
	}
	bool RecvAll(SOCKET s, void* destination, uint32_t numberOfBytes, int& SonucRet)
	{
		uint32_t totalBytesReceived = 0;

		while (totalBytesReceived < numberOfBytes)
		{
			int bytesReceived = 0;
			char* bufferOffset = (char*)destination + totalBytesReceived;

			if (!Recv(s, bufferOffset, numberOfBytes - totalBytesReceived, bytesReceived, SonucRet))
				return false;

			totalBytesReceived += bytesReceived;
		}
		SonucRet = SocketReturn::SUCCESS;
		return true;
	}
	bool Send(SOCKET s, const void* data, int numberOfBytes, uint32_t& bytesSent)
	{
		bytesSent = send(s, (const char*)data, numberOfBytes, NULL);

		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return false;
		}

		return true;
	}
public:
	bool Recv(SOCKET s, u_long& dwPktSize, int& SonucRet)
	{
		uint32_t encodedSize = 0;
		if (!RecvAll(s, &encodedSize, sizeof(uint32_t), SonucRet))
			return false;

		uint32_t bufferSize = ntohl(encodedSize);

		dwPktSize = (u_long)bufferSize;
		this->storage().resize(bufferSize);


		if (!RecvAll(s, &this->storage()[0], bufferSize, SonucRet))
			return false;
		_wpos = size();
		return true;
	}
	bool Recv(SOCKET s) {
		u_long dwPktSize;
		int SonucRet;
		return Recv(s, dwPktSize, SonucRet);
	}
	bool Send(SOCKET s)
	{
		if (s == INVALID_SOCKET)
			return false;

		uint32_t encodedPacketSize = htonl(this->storage().size());
		if (!SendAll(s, &encodedPacketSize, sizeof(uint32_t)))
			return false;

		if (!SendAll(s, this->_storage.data(), this->size()))
			return false;

		return true;
	}

	string Print(string title = "", size_t maxPerLine = 16, bool utf_8 = true, int Flag = 1 | 2 | 4) {
		try {
			printf("\n\n*** %s (Size: %d) ***", title.c_str(), size());
			string Total = "";
			string dumpstr = "";
			for (size_t addr = 0; addr < size(); addr += maxPerLine)
			{
				string line = "";
				size_t leftBytes = (addr + maxPerLine < size()) ? maxPerLine : size() - addr;
				char* elems = getElements(addr, addr + leftBytes);
				char* buffAddress = (char*)(malloc(8)); char* buffHexBytes = (char*)(malloc(128));
				if ((Flag & 1) == 1)
					sprintf(buffAddress, "%08X ", addr);
				line += buffAddress;


				if ((Flag & 2) == 2) {
					std::stringstream ss;
					for (size_t x = 0; x < leftBytes; x++)
						ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)(elems[x] & 255) << ' ';

					dumpstr = ss.str();
				}

				line += dumpstr.substr(0, 8 * 3);
				if (leftBytes > 8 && dumpstr.length() > 24)
					line += dumpstr.substr(8 * 3);
				int pad = 2;
				if (leftBytes < maxPerLine)
					pad += 3 * (maxPerLine - leftBytes);
				if (leftBytes <= 8)
					pad += 1;
				line += string(pad, ' ');
				if ((Flag & 4) == 4)
				{
					line += ' ';
					for (size_t bytes = 0; bytes < leftBytes; bytes++)
					{
						if (elems[bytes] > 0x20 && elems[bytes] <= 0x7E)
							line += (char)elems[bytes];
						else
							line += '.';
					}
				}
				Total += line + '\n';
			}
			printf("\n%s\n", Total.c_str());
			return Total;
		}
		catch (std::exception& ex) {
			printf("\nHata: %s\n", ex.what());
			return "";
		}
	}
private:
	bool SendAll(SOCKET s, const void* data, int numberOfBytes)
	{
		int totalBytesSent = 0;

		while (totalBytesSent < numberOfBytes)
		{
			uint32_t bytesSent = 0;
			char* bufferOffset = (char*)data + totalBytesSent;
			if (!Send(s, bufferOffset, numberOfBytes - totalBytesSent, bytesSent))
				return false;

			totalBytesSent += bytesSent;
		}

		return true;
	}
	struct Flags {
	public:
		const static int Encrypted = 1;
		const static int LittleEndian = 2;
	};

};
#endif
