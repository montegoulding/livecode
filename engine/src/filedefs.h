/* Copyright (C) 2003-2013 Runtime Revolution Ltd.

This file is part of LiveCode.

LiveCode is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License v3 as published by the Free
Software Foundation.

LiveCode is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with LiveCode.  If not see <http://www.gnu.org/licenses/>.  */

//
// Structures used for reading/writing
//
#ifndef	FILEDEFS_H
#define	FILEDEFS_H

#define SIGNATURE "# MetaCard "

#define READ_PIPE_SIZE 16384

#define SAVE_ENCRYPTED 0x80000000
#define SAVE_LARGE     0x40000000

enum IO_stat {
    IO_NORMAL,
    IO_NONE,
    IO_ERROR,
    IO_EOF,
    IO_TIMEOUT
};

enum Open_mode {
    OM_APPEND,
    OM_NEITHER,
    OM_VCLIP,
    OM_READ,
    OM_WRITE,
    OM_UPDATE,
    OM_TEXT,
    OM_BINARY
};

enum Object_type {
    OT_END,
    OT_HOME,
    OT_NOTHOME,
    OT_STACK,
    OT_CARD,
    OT_GROUP,
    OT_GROUPEND,
    OT_PTR,
    OT_BUTTON,
    OT_BDATA,
    OT_FIELD,
    OT_FDATA,
    OT_PARAGRAPH,
    OT_BLOCK,
    OT_IMAGE,
    OT_SCROLLBAR,
    OT_MAGNIFY,
    OT_COLORS,
    OT_GRAPHIC,
    OT_MCEPS,
    OT_AUDIO_CLIP,
    OT_VIDEO_CLIP,
    OT_ENCRYPT_STACK,
    OT_PLAYER,
    OT_CUSTOM,
	OT_STYLED_TEXT,
	// MW-2012-03-03: [[ StackFile5500 ]] The extended paragraph tag.
	OT_PARAGRAPH_EXT,
	// MW-2012-03-04: [[ StackFile5500 ]] The extended block tag.
	OT_BLOCK_EXT,
};

#define IO_WRITTEN    (1UL << 0)
#define IO_SEEKED     (1UL << 1)
#define IO_ATEOF      (1UL << 2)
#define IO_FAKE       (1UL << 3)
#define IO_FAKEWRITE	(IO_FAKE | (1UL << 4))
#define IO_FAKECUSTOM (IO_FAKE | (1UL << 5))

#define BIONB_CLEAR
#define BIONB_TESTREAD    (1UL << 1)
#define BIONB_TESTWRITE    (1UL << 2)
#define BOONB_TESTCONNECT    (1UL << 3)

struct MCSystemFileHandle;

class IO_header
{
public:
#if defined(_WINDOWS_DESKTOP)
	MCWinSysHandle fhandle;
	char *buffer;
	// MW-2013-05-02: [[ x64 ]] This is used to store a pointer in 'fake' mode so
	//   upgrade to size_t.
	size_t len;
	char *ioptr;
	MCWinSysHandle mhandle; //memory map file handle
	int putback;
	// MW-2012-09-10: [[ Bug 10230 ]] If true, it means this IO handle is a pipe.
	bool is_pipe;
	IO_header(MCWinSysHandle fp, char *b, uint4 l, uint2 f)
	{
		fhandle = fp;
		ioptr = buffer = b;
		len = l;
		mhandle = NULL;
		flags = f;
		putback = -1;
		is_pipe = false;
	}
	int getfd()
	{
		return -1;
	}
#elif defined(_MAC_DESKTOP)
	FILE *fptr;
	short serialIn;  //serial port Input reference number
	short serialOut; //serial port output reference number
	MCMacSysHandle hserialInputBuff; //handle to serial input buffer
	char *buffer;   //buffer for read data
	// MW-2013-05-02: [[ x64 ]] 'len' is used to store a pointer in 'fake' mode so
	//   upgrade to size_t.
	size_t len;      //file length
	char *ioptr;    //the starting point of a read
	
	// MW-2013-05-02: [[ x64 ]] 'l' is passed a pointer in 'fake' mode so upgrade
	//   to size_t.
	IO_header(FILE *f, short in, short out, MCMacSysHandle serialbuf, char *b, size_t l, uint2 iflags)
	{
		fptr = f;
		serialIn = in;
		serialOut = out;
		hserialInputBuff = serialbuf;
		ioptr = buffer = b;
		len = l;
		flags = iflags;
	}
	int getfd()
	{
		return -1;
	}
#elif defined(_LINUX_DESKTOP)
	FILE *fptr;
	char *buffer;
	// MW-2013-05-02: [[ x64 ]] 'len' is used to store a pointer in 'fake' mode so
	//   upgrade to size_t.
	size_t len;
	char *ioptr;
	int fd;
	
	// MW-2013-05-02: [[ x64 ]] 'l' is passed a pointer in 'fake' mode so upgrade
	//   to size_t.
	IO_header(FILE *f, char *b, size_t l, int ifd, uint2 iflags)
	{
		fptr = f;
		ioptr = buffer = b;
		len = l;
		fd = ifd;
		flags = iflags;
	}
	int getfd()
	{
		return fd;
	}
#elif defined(_MOBILE) || defined(_SERVER)
	MCSystemFileHandle *handle;
	// MW-2013-05-02: [[ x64 ]] 'len' is used to store a pointer in 'fake' mode so
	//   upgrade to size_t.
	size_t len;
	char *buffer;
	
	IO_header(MCSystemFileHandle *p_handle, uint2 iflags)
	{
		handle = p_handle;
		flags = iflags;
	}
	int getfd(void)
	{
		return -1;
	}
#else
#error IO_header not defined for this platform/mode
#endif
	uint2 flags;
};

typedef IO_header * IO_handle;

typedef struct _Streamnode
{
	char *name;
	Open_mode mode;
	IO_handle ihandle;
	IO_handle ohandle;
	int4 pid;
#if defined(_WINDOWS_DESKTOP)
	MCWinSysHandle phandle;  //process handle
	MCWinSysHandle thandle;  //process handle
#elif defined(_MAC_DESKTOP)
	MCMacProcessSerialNumber sn;
#endif
	int4 retcode;
	Boolean textmode;
}
Streamnode;

extern IO_handle IO_stdin;
extern IO_handle IO_stdout;
extern IO_handle IO_stderr;

#endif
