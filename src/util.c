#include <xmllib.h>

#include "../external/stb_ds.h"

char* xl_util_trim(const char* str) {
	char* s	 = malloc(strlen(str) + 1);
	int   nl = 1;
	int   i;
	int   inc = 0;
	int*  l	  = NULL;

	s[0] = 0;

	i = 0;
	while(1) {
		int cp;
		int new	 = xl_unicode_8_to_32(str + i, &cp);
		int  len = 0;
		char buf[4];

		if(cp == 0) break;

		if(!nl && cp == '\n') {
			len = xl_unicode_32_to_8(cp, &buf[0]);

			nl = 1;
		} else if(nl && XL_SKIPPABLE(cp)) {
		} else {
			len = xl_unicode_32_to_8(cp, &buf[0]);
			nl  = 0;
		}

		if(len > 0) {
			memcpy(s + inc, buf, len);
			inc += len;
		}

		i += new;
	}

	s[inc] = 0;

	i = 0;
	while(1) {
		int cp;
		int new = xl_unicode_8_to_32(s + i, &cp);

		if(cp == 0) break;

		arrput(l, new);

		i += new;
	}

	i   = strlen(s);
	inc = arrlen(l) - 1;
	if(inc > 0) {
		while(1) {
			int cp, new;

			i -= l[inc];
			new = xl_unicode_8_to_32(s + i, &cp);

			if(XL_SKIPPABLE(cp)) {
				s[i] = 0;
			} else {
				break;
			}

			inc--;
			if(inc < 0) break;
		}
	}

	arrfree(l);

	return s;
}
