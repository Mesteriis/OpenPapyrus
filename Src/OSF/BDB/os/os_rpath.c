/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#include "dbinc/db_page.h"
#include "dbinc/lock.h"
#include "dbinc/mp.h"
#include "dbinc/crypto.h"
#include "dbinc/btree.h"
#include "dbinc/hash.h"
#pragma hdrstop
/*
 * __db_rpath --
 *	Return the last path separator in the path or NULL if none found.
 *
 * PUBLIC: char *__db_rpath __P((const char *));
 */
char * __db_rpath(const char * path)
{
	const char * s = path;
	const char * last = NULL;
	if(PATH_SEPARATOR[1] != '\0') {
		for(; s[0] != '\0'; ++s)
			if(strchr(PATH_SEPARATOR, s[0]) != NULL)
				last = s;
	}
	else
		for(; s[0] != '\0'; ++s)
			if(s[0] == PATH_SEPARATOR[0])
				last = s;
	return (char *)last;
}