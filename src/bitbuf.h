class bf_read
{
public:
	template< typename T >
	inline T& Peek() { return *(T*)it; }

	template< typename T >
	inline T& Read() { return *((T*&)it)++; }

	template< typename T >
	inline bool Read(T& t)
	{
		if ((it + sizeof(T)) <= (raw + bytes))
		{
			t = *((T*&)it)++;
			return true;
		}
		else
		{
			return false;
		}
	}

	inline char* ReadString()
	{
		char* str = it;
		while (*it++)
		{
			// Overflowed
			if (it >= (raw + bytes))
				return nullptr;
		}
		return str;
	}

	inline bool Skip(unsigned long c)
	{
		it += c;
		return it<(raw + bytes);
	}

	// Accessors
	inline long size() const { return bytes; }

public:
	char* it;
	long bytes;
	char raw[1260];
};

// Pad a number so it lies on an N byte boundary.
// So PAD_NUMBER(0,4) is 0 and PAD_NUMBER(1,4) is 4
#define PAD_NUMBER(number, boundary) \
	( ((number) + ((boundary)-1)) / (boundary) ) * (boundary)

//-----------------------------------------------------------------------------
// Helpers.
//-----------------------------------------------------------------------------

inline int BitByte(int bits)
{
	return PAD_NUMBER(bits, 8) >> 3;
}

class bf_write
{
public:
	bf_write();

	// nMaxBits can be used as the number of bits in the buffer. 
	// It must be <= nBytes*8. If you leave it at -1, then it's set to nBytes * 8.
	bf_write(void* pData, int nBytes, int nMaxBits = -1);
	bf_write(const char* pDebugName, void* pData, int nBytes, int nMaxBits = -1);

	// Start writing to the specified buffer.
	// nMaxBits can be used as the number of bits in the buffer. 
	// It must be <= nBytes*8. If you leave it at -1, then it's set to nBytes * 8.
	void			StartWriting(void* pData, int nBytes, int iStartBit = 0, int nMaxBits = -1);

	// Restart buffer writing.
	void			Reset();

	// Get the base pointer.
	unsigned char* GetBasePointer() const { return m_pData; }

	// Enable or disable assertion on overflow. 99% of the time, it's a bug that we need to catch,
	// but there may be the occasional buffer that is allowed to overflow gracefully.
	void			SetAssertOnOverflow(bool bAssert);

	// This can be set to assign a name that gets output if the buffer overflows.
	const char* GetDebugName();
	void			SetDebugName(const char* pDebugName);


	// Seek to a specific position.
public:

	void			SeekToBit(int bitPos);


	// Bit functions.
public:

	void			WriteOneBit(int nValue);
	void			WriteOneBitNoCheck(int nValue);
	void			WriteOneBitAt(int iBit, int nValue);

	// Write signed or unsigned. Range is only checked in debug.
	void			WriteUBitLong(unsigned int data, int numbits, bool bCheckRange = true);
	void			WriteSBitLong(int data, int numbits);

	// Tell it whether or not the data is unsigned. If it's signed,
	// cast to unsigned before passing in (it will cast back inside).
	void			WriteBitLong(unsigned int data, int numbits, bool bSigned);

	// Write a list of bits in.
	bool			WriteBits(const void* pIn, int nBits);

	// Copy the bits straight out of pIn. This seeks pIn forward by nBits.
	// Returns an error if this buffer or the read buffer overflows.
	bool			WriteBitsFromBuffer(bf_read* pIn, int nBits);




	// Byte functions.
public:

	void			WriteChar(int val);
	void			WriteByte(int val);
	void			WriteShort(int val);
	void			WriteWord(int val);
	void			WriteLong(long val);
	void			WriteFloat(float val);
	bool			WriteBytes(const void* pBuf, int nBytes);

	// Returns false if it overflows the buffer.
	bool			WriteString(const char* pStr);


	// Status.
public:

	// How many bytes are filled in?
	int				GetNumBytesWritten();
	int				GetNumBitsWritten();
	int				GetMaxNumBits();
	int				GetNumBitsLeft();
	int				GetNumBytesLeft();
	unsigned char* GetData();

	// Has the buffer overflowed?
	bool			CheckForOverflow(int nBits);
	inline bool		IsOverflowed() { return m_bOverflow; }

	inline void		SetOverflowFlag();


public:
	// The current buffer.
	unsigned char* m_pData;
	int				m_nDataBytes;
	int				m_nDataBits;

	// Where we are in the buffer.
	int				m_iCurBit;

private:

	// Errors?
	bool			m_bOverflow;

	bool			m_bAssertOnOverflow;
	const char* m_pDebugName;
};


//-----------------------------------------------------------------------------
// Inlined methods
//-----------------------------------------------------------------------------

/*bf_write::bf_write()
{
}
*/

// How many bytes are filled in?
inline int bf_write::GetNumBytesWritten()
{
	return BitByte(m_iCurBit);
}

inline int bf_write::GetNumBitsWritten()
{
	return m_iCurBit;
}

inline int bf_write::GetMaxNumBits()
{
	return m_nDataBits;
}

inline int bf_write::GetNumBitsLeft()
{
	return m_nDataBits - m_iCurBit;
}

inline int bf_write::GetNumBytesLeft()
{
	return GetNumBitsLeft() >> 3;
}

inline unsigned char* bf_write::GetData()
{
	return m_pData;
}

inline bool bf_write::CheckForOverflow(int nBits)
{
	if (m_iCurBit + nBits > m_nDataBits)
	{
		SetOverflowFlag();
	}

	return m_bOverflow;
}

inline void bf_write::SetOverflowFlag()
{
	if (m_bAssertOnOverflow)
	{
		//Assert(false);
	}

	m_bOverflow = true;
}

inline void bf_write::WriteOneBitNoCheck(int nValue)
{
	if (nValue)
		m_pData[m_iCurBit >> 3] |= (1 << (m_iCurBit & 7));
	else
		m_pData[m_iCurBit >> 3] &= ~(1 << (m_iCurBit & 7));

	++m_iCurBit;
}

inline void bf_write::WriteOneBit(int nValue)
{
	if (!CheckForOverflow(1))
		WriteOneBitNoCheck(nValue);
}


inline void	bf_write::WriteOneBitAt(int iBit, int nValue)
{
	if (iBit + 1 > m_nDataBits)
	{
		SetOverflowFlag();
		return;
	}

	if (nValue)
		m_pData[iBit >> 3] |= (1 << (iBit & 7));
	else
		m_pData[iBit >> 3] &= ~(1 << (iBit & 7));
}


inline void bf_write::WriteUBitLong(unsigned int curData, int numbits, bool bCheckRange)
{


	extern unsigned long g_BitWriteMasks[32][33];

	// Bounds checking..
	if ((m_iCurBit + numbits) > m_nDataBits)
	{
		m_iCurBit = m_nDataBits;
		SetOverflowFlag();
		return;
	}

	int nBitsLeft = numbits;
	int iCurBit = m_iCurBit;

	// Mask in a dword.
	unsigned int iDWord = iCurBit >> 5;

	//Assert((iDWord * 4 + sizeof(long)) <= (unsigned int)m_nDataBytes);

	unsigned long iCurBitMasked = iCurBit & 31;
	((unsigned long*)m_pData)[iDWord] &= g_BitWriteMasks[iCurBitMasked][nBitsLeft];
	((unsigned long*)m_pData)[iDWord] |= curData << iCurBitMasked;

	// Did it span a dword?
	int nBitsWritten = 32 - iCurBitMasked;
	if (nBitsWritten < nBitsLeft)
	{
		nBitsLeft -= nBitsWritten;
		iCurBit += nBitsWritten;
		curData >>= nBitsWritten;

		unsigned long iCurBitMasked = iCurBit & 31;
		((unsigned long*)m_pData)[iDWord + 1] &= g_BitWriteMasks[iCurBitMasked][nBitsLeft];
		((unsigned long*)m_pData)[iDWord + 1] |= curData << iCurBitMasked;
	}

	m_iCurBit += numbits;
}


//-----------------------------------------------------------------------------
// This is useful if you just want a buffer to write into on the stack.
//-----------------------------------------------------------------------------

template<int SIZE>
class bf_write_static : public bf_write
{
public:
	inline bf_write_static() : bf_write(m_Data, SIZE) {}

	char	m_StaticData[SIZE];
};