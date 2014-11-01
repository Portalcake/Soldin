/*
 * Soldin - Lunia Server Emulator 
 * Copyright (c) 2010 Seipheroth
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE. 
 */
#include <crypt.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

bool Crypto::m_bInitialized = false;
byte Crypto::m_iTableEncrypt[256][256];
byte Crypto::m_iTableDecrypt[256][256];

/* Initializes a new instance of the crypto class. */
Crypto::Crypto(): m_iCryptKey(0)
{
	if (!m_bInitialized)
		Initialize();
}

/* Initializes the encryption and decryption tables. */
void Crypto::Initialize()
{
	memset(m_iTableEncrypt, 0, 65536);
	memset(m_iTableDecrypt, 0, 65536);
	for (uint32_t i = 0; i < 256; ++i)
	{
		byte t1 = (i * 0x49) ^ 0x15;
		byte t2 = (t1 * 0x49) ^ 0x15;

		for (uint32_t j = 0; j < 256; ++j)
		{
			byte val = (((j * 0x49) ^ 0x15) + t2) ^ 0x14;
			m_iTableEncrypt[t1][j]   = val;
			m_iTableDecrypt[t1][val] = j;
		}
	}
	m_bInitialized = true;
}

/* Decrypts the specified data. */
void Crypto::Decrypt(byte *data, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		byte offset = ((m_iCryptKey++) & 0xFF) + 4;
		for (byte j = 1; j < 4; ++j)
			offset += (((m_iCryptKey >> (j * 8)) & 0xFF) * 0x49) ^ 0x15;
		
		data[i] = m_iTableDecrypt[offset][data[i]];
	}
}

/* Encrypts the specified data. */
void Crypto::Encrypt(byte *data, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		byte offset = ((m_iCryptKey++) & 0xFF) + 4;
		for (byte j = 1; j < 4; ++j)
			offset += (((m_iCryptKey >> (j * 8)) & 0xFF) * 0x49) ^ 0x15;
		
		data[i] = m_iTableEncrypt[offset][data[i]];
	}
}

/* Generates a random 32-bit encryption key. */
uint32_t Crypto::GenerateKey()
{
	srand((uint32_t)time(NULL));

	return (((rand() % 0xFF) << 24) | ((rand() % 0xFF) << 16) | ((rand() % 0xFF) << 8)  | (rand() % 0xFF));
}
