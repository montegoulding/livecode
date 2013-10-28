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

#include "prefix.h"

#include "globdefs.h"
#include "filedefs.h"
#include "objdefs.h"
#include "parsedef.h"
#include "mcio.h"

#include "uidc.h"
#include "param.h"
#include "mcerror.h"
#include "execpt.h"
#include "util.h"
#include "object.h"
#include "socket.h"
#include "globals.h"
#include "text.h"
#include "system.h"
#include "eventqueue.h"
#include "osspec.h"

////////////////////////////////////////////////////////////////////////////////

void MCS_log(MCStringRef p_message)
{
	MCsystem -> Debug(p_message);
}

////////////////////////////////////////////////////////////////////////////////

static bool is_whitespace(char p_char)
{
	return p_char == ' ' || p_char == '\n';
}

bool MCSystemStripUrl(MCStringRef p_url, MCStringRef &r_stripped)
{
	uindex_t t_start = 0;
	uindex_t t_end = MCStringGetLength(p_url);
	while (t_start < t_end && is_whitespace(MCStringGetNativeCharAtIndex(p_url, t_start)))
		t_start++;
	while (t_end > t_start && is_whitespace(MCStringGetNativeCharAtIndex(p_url, t_end - 1)))
		t_end--;
	
	return MCStringCopySubstring(p_url, MCRangeMake(t_start, t_end - t_start), r_stripped);
}

bool MCSystemProcessUrl(MCStringRef p_url, MCSystemUrlOperation p_operations, MCStringRef &r_processed_url)
{
	bool t_success = true;
	
	MCStringRef t_processed;
    t_processed = nil;
	
	if (t_success && (p_operations & kMCSystemUrlOperationStrip))
	{
		MCAutoStringRef t_stripped;
		t_success = MCSystemStripUrl(t_processed != nil ? t_processed : p_url, &t_stripped);
		if (t_success)
		{
            if (t_processed != nil)
                MCValueRelease(t_processed);
            
			t_processed = MCValueRetain(*t_stripped);
		}
	}
	
	// if no processing, just return a copy of the input url
	if (t_success && t_processed == nil)
		t_success = MCStringCopy(p_url, t_processed);
	
	if (t_success)
		r_processed_url = t_processed;
	else
		MCValueRelease(t_processed);
	
	return t_success;
}

////////////////////////////////////////////////////////////////////////////////

class MCUrlProgressEvent : public MCCustomEvent
{
public:
	static MCUrlProgressEvent *CreateUrlProgressEvent(MCObjectHandle *object, const char *url, MCSystemUrlStatus status, uint32_t amount, uint32_t total, const char *error);

	void Destroy(void);
	void Dispatch(void);
	
private:
	char *m_url;
	MCObjectHandle *m_object;
	MCSystemUrlStatus m_status;
	union
	{
		struct
		{
			int32_t amount;
			int32_t total;
		} m_transferred;
		char *m_error;
	};
};

MCUrlProgressEvent *MCUrlProgressEvent::CreateUrlProgressEvent(MCObjectHandle *p_object, const char *p_url, MCSystemUrlStatus p_status, uint32_t p_amount, uint32_t p_total, const char *p_error)
{
	MCUrlProgressEvent *t_event;
	t_event = new MCUrlProgressEvent();
	if (t_event == nil)
		return nil;
	
	t_event->m_url = nil;
	t_event->m_object = nil;
	t_event->m_status = kMCSystemUrlStatusNone;
	t_event->m_error = nil;
	
	bool t_success;
	t_success = true;
	
	if (t_success)
		t_success = MCCStringClone(p_url, t_event->m_url);
	if (t_success && p_status == kMCSystemUrlStatusError)
		t_success = MCCStringClone(p_error, t_event->m_error);
	
	if (t_success)
	{
		t_event->m_status = p_status;
		t_event->m_object = p_object;
		t_event->m_object->Retain();
		if (t_event->m_status == kMCSystemUrlStatusUploading || t_event->m_status == kMCSystemUrlStatusLoading)
		{
			t_event->m_transferred.amount = p_amount;
			t_event->m_transferred.total = p_total;
		}
	}
	else
	{
		MCCStringFree(t_event->m_url);
		MCCStringFree(t_event->m_error);
		delete t_event;
		return nil;
	}
	
	return t_event;
}

void MCUrlProgressEvent::Destroy(void)
{
	MCCStringFree(m_url);
	if (m_status == kMCSystemUrlStatusError)
		MCCStringFree(m_error);
	if (m_object != nil)
		m_object->Release();
	delete this;
}

void MCUrlProgressEvent::Dispatch(void)
{
	MCObject *t_object;
	t_object = m_object -> Get();
	if (t_object != nil)
	{
		switch (m_status)
		{
			case kMCSystemUrlStatusStarted:
				t_object -> message_with_args(MCM_url_progress, m_url, "contacted");
				break;
			case kMCSystemUrlStatusNegotiated:
				t_object -> message_with_args(MCM_url_progress, m_url, "requested");
				break;
			case kMCSystemUrlStatusUploading:
			{
				char t_amount[U4L], t_total[U4L];
				sprintf(t_amount, "%u", m_transferred.amount);
				sprintf(t_total, "%u", m_transferred.total);
				t_object -> message_with_args(MCM_url_progress, m_url, "uploading", t_amount, t_total);
			}
				break;
			case kMCSystemUrlStatusUploaded:
				t_object -> message_with_args(MCM_url_progress, m_url, "uploaded");
				break;
			case kMCSystemUrlStatusLoading:
			{
				char t_amount[U4L], t_total[U4L];
				sprintf(t_amount, "%u", m_transferred.amount);
				if (m_transferred.total != -1)
					sprintf(t_total, "%u", m_transferred.total);
				else
					t_total[0] = 0;
				t_object -> message_with_args(MCM_url_progress, m_url, "loading", t_amount, t_total);
			}
				break;
			case kMCSystemUrlStatusFinished:
				t_object -> message_with_args(MCM_url_progress, m_url, "downloaded");
				break;
			case kMCSystemUrlStatusError:
				t_object -> message_with_args(MCM_url_progress, m_url, "error", m_error);
				break;
		}
	}
}

static void send_url_progress(MCObjectHandle *p_object, MCSystemUrlStatus p_status, const char *p_url, int32_t p_amount, int32_t& x_total, const char *p_data)
{
	if (p_status == kMCSystemUrlStatusNegotiated)
		x_total = *(int32_t*)p_data;
	
	MCUrlProgressEvent *t_event;
	t_event = MCUrlProgressEvent::CreateUrlProgressEvent(p_object, p_url, p_status, p_amount, x_total, p_data);
	if (t_event)
		MCEventQueuePostCustom(t_event);
}

//////////

struct MCSGetUrlState
{
	const char *url;
	MCSystemUrlStatus status;
	MCStringRef data;
	MCObjectHandle *object;
	int32_t total;
    MCStringRef error;
};

static bool MCS_geturl_callback(void *p_context, MCSystemUrlStatus p_status, const void *p_data)
{
	MCSGetUrlState *context;
	context = static_cast<MCSGetUrlState *>(p_context);
	
	context -> status = p_status;
	
	if (p_status == kMCSystemUrlStatusError)
    {
        if (context -> error != nil)
            MCValueRelease(context -> error);
		MCStringCreateWithCString((const char *)p_data, context -> error);
        send_url_progress(context -> object, p_status, context -> url, MCStringGetLength(context -> error), context -> total, (const char *)p_data);
    }
	else
    {
        if (p_status == kMCSystemUrlStatusLoading)
        {
            MCAutoStringRef t_new_data;
            if (context -> data != nil)
            {
                /* UNCHECKED */ MCStringMutableCopy(context -> data, &t_new_data);
                MCValueRelease(context -> data);
            }
            else
                /* UNCHECKED */ MCStringCreateMutable(0, &t_new_data);
            
            /* UNCHECKED */ MCStringAppendFormat(*t_new_data, "%s", (const char *)p_data);
            /* UNCHECKED */ MCStringCopy(*t_new_data, context -> data);
        }
        send_url_progress(context -> object, p_status, context -> url, MCStringGetLength(context -> data), context -> total, (const char *)p_data);
    }
	return true;
}

void MCS_geturl(MCObject *p_target, MCStringRef p_url)
{
	MCAutoStringRef t_processed_url;
	
	// IM-2013-07-30: [[ Bug 10800 ]] strip whitespace chars from url before attempting to fetch
	if (!MCSystemProcessUrl(p_url, kMCSystemUrlOperationStrip, &t_processed_url))
		return;
	
	MCSGetUrlState t_state;
	t_state . url = strclone(MCStringGetCString(*t_processed_url));
	t_state . status = kMCSystemUrlStatusNone;
	t_state . object = p_target -> gethandle();
	
	if (!MCSystemLoadUrl(*t_processed_url, MCS_geturl_callback, &t_state))
	{
		t_state . object -> Release();
		return;
	}
	
	MCurlresult -> clear();
	
	while(t_state . status != kMCSystemUrlStatusFinished && t_state . status != kMCSystemUrlStatusError)
		MCscreen -> wait(60.0, True, True);
	
    if (t_state . data != nil)
    {
        MCurlresult -> setvalueref(t_state . data);
        MCValueRelease(t_state . data);
    }
    else
        MCurlresult -> clear();
    
	if (t_state . status == kMCSystemUrlStatusFinished || t_state . error == nil)
		MCresult -> clear();
	else
        MCresult -> setvalueref(t_state . error);

    if (t_state . url != nil)
        delete[] t_state . url;
    
    if (t_state . error != nil)
        MCValueRelease(t_state . error);
	
	t_state . object -> Release();
}

//////////

class MCUrlLoadEvent : public MCCustomEvent
{
public:
	static MCUrlLoadEvent *CreateUrlLoadEvent(MCObjectHandle *object, MCNameRef p_message, const char *url, MCSystemUrlStatus status, void *data, uint32_t size, const char *error);
	
	void Destroy(void);
	void Dispatch(void);
	
private:
	char *m_url;
	MCObjectHandle *m_object;
	MCSystemUrlStatus m_status;
	MCNameRef m_message;
	union
	{
		struct
		{
			void *bytes;
			uint32_t size;
		} m_data;
		char *m_error;
	};
};

MCUrlLoadEvent *MCUrlLoadEvent::CreateUrlLoadEvent(MCObjectHandle *p_object, MCNameRef p_message, const char *p_url, MCSystemUrlStatus p_status, void *p_data, uint32_t p_size, const char *p_error)
{
	MCUrlLoadEvent *t_event;
	t_event = new MCUrlLoadEvent();
	if (t_event == nil)
		return nil;
	
	t_event->m_url = nil;
	t_event->m_object = nil;
	t_event->m_status = kMCSystemUrlStatusNone;
	t_event->m_error = nil;
	t_event->m_message = nil;
	
	bool t_success;
	t_success = true;
	
	if (t_success)
		t_success = MCCStringClone(p_url, t_event->m_url);
	if (t_success)
		t_success = MCNameClone(p_message, t_event->m_message);
	if (t_success && p_status == kMCSystemUrlStatusError)
		t_success = MCCStringClone(p_error, t_event->m_error);
	
	if (t_success)
	{
		t_event->m_status = p_status;
		t_event->m_object = p_object;
		t_event->m_object->Retain();
		if (t_event->m_status == kMCSystemUrlStatusFinished)
		{
			t_event->m_data.bytes = p_data;
			t_event->m_data.size = p_size;
		}
	}
	else
	{
		MCCStringFree(t_event->m_url);
		MCNameDelete(t_event->m_message);
		MCCStringFree(t_event->m_error);
		delete t_event;
		return nil;
	}
	
	return t_event;
}

void MCUrlLoadEvent::Destroy(void)
{
	MCCStringFree(m_url);
	MCNameDelete(m_message);
	if (m_status == kMCSystemUrlStatusFinished)
		MCMemoryDelete(m_data.bytes);
	else if (m_status == kMCSystemUrlStatusError)
		MCCStringFree(m_error);
	if (m_object != nil)
		m_object->Release();
	delete this;
}

void MCUrlLoadEvent::Dispatch(void)
{
	MCObject *t_object;
	t_object = m_object -> Get();
	if (t_object != nil)
	{
		switch (m_status)
		{
			case kMCSystemUrlStatusFinished:
				t_object -> message_with_args(m_message, m_url, "downloaded", MCString(static_cast<char*>(m_data.bytes), m_data.size));
				break;
			case kMCSystemUrlStatusError:
				t_object -> message_with_args(m_message, m_url, "error", m_error);
				break;
		}
	}
}

struct MCSLoadUrlState
{
	char *url;
	MCSystemUrlStatus status;
	struct
	{
		void *bytes;
		uint32_t size;
	} data;
	MCObjectHandle *object;
	int32_t total;
	MCNameRef message;
};

static bool MCS_loadurl_callback(void *p_context, MCSystemUrlStatus p_status, const void *p_data)
{
	MCSLoadUrlState *context;
	context = static_cast<MCSLoadUrlState *>(p_context);
	
	context -> status = p_status;
	
	if (p_status == kMCSystemUrlStatusError)
		MCMemoryDelete(context->data.bytes);
	else if (p_status == kMCSystemUrlStatusLoading)
	{
		const MCString *t_data = static_cast<const MCString*>(p_data);
		MCMemoryReallocate(context->data.bytes, context->data.size + t_data->getlength(), context->data.bytes);
		MCMemoryCopy(static_cast<uint8_t*>(context->data.bytes) + context->data.size, t_data->getstring(), t_data->getlength());
		context->data.size += t_data->getlength();
	}
	
	send_url_progress(context -> object, p_status, context -> url, context -> data . size, context -> total, (const char *)p_data);
	
	if (p_status == kMCSystemUrlStatusError || p_status == kMCSystemUrlStatusFinished)
	{
		MCUrlLoadEvent *t_event;
		t_event = MCUrlLoadEvent::CreateUrlLoadEvent(context->object, context->message, context->url, p_status, context->data.bytes, context->data.size, static_cast<const char *>(p_data));
		if (t_event)
			MCEventQueuePostCustom(t_event);
		context->object->Release();
		MCCStringFree(context->url);
		MCNameDelete(context->message);
		MCMemoryDelete(context);
	}
	return true;
}

void MCS_loadurl(MCObject *p_object, MCStringRef p_url, MCNameRef p_message)
{
	bool t_success = true;
	MCSLoadUrlState *t_state = nil;
	MCAutoStringRef t_processed;
	t_success = MCMemoryNew(t_state) &&
			MCSystemProcessUrl(p_url, kMCSystemUrlOperationStrip, &t_processed);
	
	if (t_success)
	{
		t_state->url = strclone(MCStringGetCString(*t_processed));
		t_state->message = p_message;
		t_state->status = kMCSystemUrlStatusNone;
		t_state->object = p_object -> gethandle();
		t_state->data . bytes = nil;
		t_state->data . size = 0;
		
		t_success = MCSystemLoadUrl(*t_processed, MCS_loadurl_callback, t_state);
	}
	
	if (t_success)
		MCresult->clear();
	else
	{
        delete[] t_state->url;
		MCNameDelete(p_message);
        MCMemoryDelete(t_state);
		MCurlresult -> clear();
		MCresult->sets("error: load URL failed");
	}
}

//////////

struct MCSPostUrlState
{
	const char *url;
	MCSystemUrlStatus status;
	MCStringRef data;
	MCObjectHandle *object;
	int32_t post_sent;
	int32_t post_length;
	int32_t total;
};

static bool MCS_posturl_callback(void *p_context, MCSystemUrlStatus p_status, const void *p_data)
{
	MCSPostUrlState *context;
	context = static_cast<MCSPostUrlState *>(p_context);
	
	context -> status = p_status;
	
	if (p_status == kMCSystemUrlStatusError)
		/* UNCHECKED */ MCStringCreateWithCString((const char *)p_data, context -> data);
	else if (p_status == kMCSystemUrlStatusLoading)
    {
        MCAutoStringRef t_data;
        if (context -> data != nil)
            /* UNCHECKED */ MCStringMutableCopy(context -> data, &t_data);
        else
            MCStringCreateMutable(0, &t_data);

		/* UNCHECKED */ MCStringAppendFormat(*t_data, "%s", (const char *)p_data);
        MCStringCopy(*t_data, context -> data);
    }
	if (p_status == kMCSystemUrlStatusUploading || p_status == kMCSystemUrlStatusUploaded)
	{
		context -> post_sent = *(uint32_t*)p_data;
		send_url_progress(context -> object, p_status, context -> url, context -> post_sent, context -> post_length, nil);
	}
	else
		send_url_progress(context -> object, p_status, context -> url, MCStringGetLength(context -> data), context -> total, (const char *)p_data);
	
	return true;
}

void MCS_posttourl(MCObject *p_target, MCDataRef p_data, MCStringRef p_url)
{
	bool t_success = true;
	
	MCAutoStringRef t_processed;
	MCObjectHandle *t_obj = nil;
	MCSPostUrlState t_state;
	
	t_success = MCSystemProcessUrl(p_url, kMCSystemUrlOperationStrip, &t_processed);
	if (t_success)
		t_success = nil != (t_obj = p_target->gethandle());
	
	if (t_success)
	{
		t_state . url = strclone(MCStringGetCString(*t_processed));
		t_state . status = kMCSystemUrlStatusNone;
		t_state . object = t_obj;
		t_state . post_sent = 0;
		t_state . post_length = MCDataGetLength(p_data);
        
		t_success = MCSystemPostUrl(*t_processed, p_data, MCDataGetLength(p_data), MCS_posturl_callback, &t_state);
	}
	
	if (t_success)
	{
		MCurlresult -> clear();
        MCresult -> clear();
		
		while(t_state . status != kMCSystemUrlStatusFinished && t_state . status != kMCSystemUrlStatusError)
			MCscreen -> wait(60.0, True, True);
		
		if (t_state . status == kMCSystemUrlStatusFinished)
		{
			MCresult -> clear();
            if (t_state . data != nil)
                MCurlresult -> setvalueref(t_state . data);
		}
		else
		{
			MCurlresult -> clear();
            if (t_state . data != nil)
                MCresult -> setvalueref(t_state . data);
		}
	}
	
    if (t_state . data != nil)
        MCValueRelease(t_state . data);
    
    if (t_state . url != nil)
        delete[] t_state . url;
    
	if (t_obj != nil)
		t_obj -> Release();
}

//////////

struct MCSPutUrlState
{
	const char *url;
	MCSystemUrlStatus status;
	MCObjectHandle *object;
	int32_t put_sent;
	int32_t put_length;
	char *error;
};

static bool MCS_puturl_callback(void *p_context, MCSystemUrlStatus p_status, const void *p_data)
{
	MCSPutUrlState *context;
	context = static_cast<MCSPutUrlState*>(p_context);
	
	context->status = p_status;
	
	if (p_status == kMCSystemUrlStatusError)
		MCCStringClone((const char*)p_data, context->error);
	
	if (p_status == kMCSystemUrlStatusUploading || p_status == kMCSystemUrlStatusUploaded)
	{
		context->put_sent = *(uint32_t*)p_data;
		send_url_progress(context->object, p_status, context->url, context->put_sent, context->put_length, nil);
	}
	else
		send_url_progress(context->object, p_status, context->url, context->put_sent, context->put_length, (const char*)p_data);

	return true;
}

void MCS_putintourl(MCObject *p_target, MCDataRef p_data, MCStringRef p_url)
{
	bool t_success = true;
	
	MCAutoStringRef t_processed;
	MCObjectHandle *t_obj = nil;
	MCSPutUrlState t_state;
	
	t_success = MCSystemProcessUrl(p_url, kMCSystemUrlOperationStrip, &t_processed);
	if (t_success)
		t_success = nil != (t_obj = p_target->gethandle());
	
	if (t_success)
	{
		t_state.url = strclone(MCStringGetCString(*t_processed));
		t_state.status = kMCSystemUrlStatusNone;
		t_state.object = t_obj;
		t_state.put_sent = 0;
		t_state.put_length = MCDataGetLength(p_data);
        
		t_success = MCSystemPutUrl(*t_processed, p_data, MCDataGetLength(p_data), MCS_puturl_callback, &t_state);
	}
	
	if (t_success);
	{
		MCurlresult->clear();
		
		while (t_state.status != kMCSystemUrlStatusUploaded && t_state.status != kMCSystemUrlStatusError)
			MCscreen->wait(60.0, True, True);
		
		if (t_state.status == kMCSystemUrlStatusUploaded)
			MCresult->clear();
		else
			MCresult->sets(MCString(t_state.error));
	}
	
	if (t_state . url != nil)
        delete[] t_state . url;
	if (t_obj != nil)
		t_obj->Release();
}

//////////

struct MCSDownloadUrlState
{
	const char *url;
	MCSystemUrlStatus status;
	IO_handle output;
	MCObjectHandle *object;
	int32_t length;
	int32_t total;
};

static bool MCS_downloadurl_callback(void *p_context, MCSystemUrlStatus p_status, const void *p_data)
{
	MCSDownloadUrlState *context;
	context = static_cast<MCSDownloadUrlState *>(p_context);
	
	context -> status = p_status;
	
	if (p_status == kMCSystemUrlStatusError)
		MCresult -> sets((const char *)p_data);
	else if (p_status == kMCSystemUrlStatusLoading)
	{
		context -> length += ((const MCString *)p_data) -> getlength();
		MCS_write(((const MCString *)p_data) -> getstring(), ((const MCString *)p_data) -> getlength(), 1, context -> output);
	}
	
	send_url_progress(context -> object, p_status, context -> url, context -> length, context -> total, (const char *)p_data);
	
	return true;
}

void MCS_downloadurl(MCObject *p_target, MCStringRef p_url, MCStringRef p_file)
{
	bool t_success = true;
	
	MCAutoStringRef t_processed;
	MCObjectHandle *t_obj = nil;
	IO_handle t_output = nil;
	MCSDownloadUrlState t_state;
	
	t_output = MCS_open(p_file, kMCSOpenFileModeWrite, False, False, 0);
	if (t_output == nil)
	{
		MCresult -> sets("can't open that file");
		return;
	}
	
	t_success = MCSystemProcessUrl(p_url, kMCSystemUrlOperationStrip, &t_processed);
	if (t_success)
		t_success = nil != (t_obj = p_target->gethandle());
	
	if (t_success)
	{
		t_state . url = strclone(MCStringGetCString(*t_processed));
		t_state . status = kMCSystemUrlStatusNone;
		t_state . object = t_obj;
		t_state . output = t_output;
		t_state . length = 0;
		t_state . total = 0;
		
		t_success = MCSystemLoadUrl(*t_processed, MCS_downloadurl_callback, &t_state);
	}
	
	if (t_success)
	{
		while(t_state . status != kMCSystemUrlStatusFinished && t_state . status != kMCSystemUrlStatusError)
			MCscreen -> wait(60.0, True, True);
		
		if (t_state . status == kMCSystemUrlStatusFinished)
			MCresult -> clear();
	}

	if (t_state . url != nil)
        delete[] t_state . url;
	if (t_output != nil)
		MCS_close(t_output);
	if (t_obj != nil)
		t_obj->Release();
}

//////////

void MCS_deleteurl(MCObject *p_object, MCStringRef p_url)
{
	MCurlresult -> clear();
	MCresult -> sets("not implemented");
}

void MCS_unloadurl(MCObject *p_object, MCStringRef p_url)
{
	MCurlresult -> clear();
	MCresult -> sets("not implemented");
}

//////////

void MCS_seturlsslverification(bool p_enabled)
{
	MCSystemSetUrlSSLVerification(p_enabled);
}

////////////////////////////////////////////////////////////////////////////////


bool MCS_put(MCExecPoint& ep, MCSPutKind p_kind, MCStringRef p_data)
{
	/* UNCHECKED */ ep . setvalueref(p_data);

	switch(p_kind)
	{
	case kMCSPutOutput:
	case kMCSPutBeforeMessage:
	case kMCSPutIntoMessage:
	case kMCSPutAfterMessage:
		MCS_log(p_data);
		break;

	default:
		break;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void MCS_set_errormode(MCSErrorMode mode)
{
}

MCSErrorMode MCS_get_errormode(void)
{
	return kMCSErrorModeNone;
}

////////////////////////////////////////////////////////////////////////////////

void MCS_set_outputtextencoding(MCSOutputTextEncoding encoding)
{
}

MCSOutputTextEncoding MCS_get_outputtextencoding(void)
{
	return kMCSOutputTextEncodingNative;
}

void MCS_set_outputlineendings(MCSOutputLineEndings ending)
{
}

MCSOutputLineEndings MCS_get_outputlineendings(void)
{
	return kMCSOutputLineEndingsNative;
}

////////////////////////////////////////////////////////////////////////////////

bool MCS_set_session_save_path(MCStringRef p_path)
{
	return true;
}

bool MCS_get_session_save_path(MCStringRef& r_path)
{
	r_path = MCValueRetain(kMCEmptyString);
	return true;
}

bool MCS_set_session_lifetime(uint32_t p_lifetime)
{
	return true;
}

uint32_t MCS_get_session_lifetime(void)
{
	return 0;
}

bool MCS_set_session_name(MCStringRef p_name)
{
	return true;
}

bool MCS_get_session_name(MCStringRef &r_name)
{
	r_name = MCValueRetain(kMCEmptyString);
	return true;
}

bool MCS_set_session_id(MCStringRef p_id)
{
	return true;
}

bool MCS_get_session_id(MCStringRef &r_id)
{
	r_id = MCValueRetain(kMCEmptyString);
	return true;
}

////////////////////////////////////////////////////////////////////////////////

int MCA_file(MCStringRef p_title, MCStringRef p_prompt, MCStringRef p_filter, MCStringRef p_initial, unsigned int p_options, MCStringRef &r_value, MCStringRef &r_result)
{
	return 0;
}

int MCA_ask_file(MCStringRef p_title, MCStringRef p_prompt, MCStringRef p_filter, MCStringRef p_initial, unsigned int p_options, MCStringRef &r_value, MCStringRef &r_result)
{
	return 0;
}

int MCA_file_with_types(MCStringRef p_title, MCStringRef p_prompt, MCStringRef *p_types, uint4 p_type_count, MCStringRef p_initial, unsigned int p_options, MCStringRef &r_value, MCStringRef &r_result)
{
	return 0;
}

int MCA_ask_file_with_types(MCStringRef p_title, MCStringRef p_prompt, MCStringRef *p_types, uint4 p_type_count, MCStringRef p_initial, unsigned int p_options, MCStringRef &r_value, MCStringRef &r_result)
{
	return 0;
}

int MCA_folder(MCStringRef p_title, MCStringRef p_prompt, MCStringRef p_initial, unsigned int p_options, MCStringRef &r_value, MCStringRef &r_result)
{
	return 0;
}

bool MCA_color(MCStringRef title, MCColor initial_color, bool as_sheet, bool& r_chosen, MCColor& r_chosen_color)
{
	return true;
}

// MERG-2013-08-18: Stubs for colorDialogColors.
void MCA_setcolordialogcolors(MCExecPoint& p_ep)
{
    
}

void MCA_getcolordialogcolors(MCExecPoint& p_ep)
{
	p_ep.clear();
}


////////////////////////////////////////////////////////////////////////////////

