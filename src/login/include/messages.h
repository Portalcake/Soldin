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
#ifndef __SOLDIN_MESSAGES_H__
#define __SOLDIN_MESSAGES_H__

/* Gateway <-> Client message ID's. */
#define MSG_CLIENTHASH          0xAA41
#define MSG_LOGIN               0xBA09
#define MSG_CHARACTER_LICENSE   0x022C // send by server only.
#define MSG_CHARACTER_LIST      0xDC2C // send by server only.
#define MSG_CHARACTER_CREATE	0xC96E
#define MSG_CHARACTER_DELETE	0x899F
#define MSG_CHARACTER_SELECT    0x356E
#define MSG_CHARACTER_DESELECT  0xF2AD
#define MSG_DISCONNECT          0xDD83 // send by client only.
#define MSG_PING	            0x482F // send by client only.
#define MSG_SQUARE_LIST         0x3A10
#define MSG_SQUARE_SELECT       0x7AE9 // send by client only.
#define MSG_SQUARE_DETAILS      0xB98B // send by server only.

/* Gateway <-> Square message ID's. */
#define MSG_SQUARE_AUTH         0x0001
#define MSG_SQUARE_UPDATE       0x0002
#define MSG_SQUARE_SESSIONINFO  0x0003

#endif /* __SOLDIN_MESSAGES_H__ */
