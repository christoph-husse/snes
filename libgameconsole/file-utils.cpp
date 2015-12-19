#include "S9xBridge.hpp"

	// WINE implementation
	void _splitpath(const char* path, char* drv, char* dir, char* name, char* ext)
	{
		const char* end; /* end of processed string */
		const char* p;   /* search pointer */
		const char* s;   /* copy pointer */

		/* extract drive name */
		if (path[0] && path[1] == ':') {
			if (drv) {
				*drv++ = *path++;
				*drv++ = *path++;
				*drv = '\0';

			}

		}
		else if (drv)
			* drv = '\0';

		/* search for end of string or stream separator */
		for (end = path; *end && *end != ':';)
			end++;

		/* search for begin of file extension */
		for (p = end; p > path && *--p != '\\' && *p != '/';)
		if (*p == '.') {
			end = p;
			break;

		}

		if (ext)
		for (s = end; (*ext = *s++);)
			ext++;

		/* search for end of directory name */
		for (p = end; p > path;)
		if (*--p == '\\' || *p == '/') {
			p++;
			break;

		}

		if (name) {
			for (s = p; s < end;)
				* name++ = *s++;

			*name = '\0';

		}

		if (dir) {
			for (s = path; s < p;)
				* dir++ = *s++;

			*dir = '\0';

		}
	}

	// ReactOS
	void _makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext)
	{
		char *p = path;

		if (!path)
			return;

		if (drive && drive[0])
		{
			*p++ = drive[0];
			*p++ = ':';
		}
		if (dir && dir[0])
		{
			uint64_t len = strlen(dir);
			memmove(p, dir, len);
			p += len;
			if (p[-1] != '/' && p[-1] != '\\')
				* p++ = '\\';
		}
		if (fname && fname[0])
		{
			uint64_t len = strlen(fname);
			memmove(p, fname, len);
			p += len;
		}
		if (ext && ext[0])
		{
			if (ext[0] != '.')
				* p++ = '.';
			strcpy(p, ext);
		}
		else
			*p = '\0';
	}
