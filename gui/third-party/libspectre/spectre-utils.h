/* This file is part of Libspectre.
 * (c)GPL2+
 *
 * Copyright (C) 2007 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 * Libspectre is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Libspectre is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SPECTRE_UTILS_H
#define SPECTRE_UTILS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>

#include <libspectre/spectre-macros.h>

SPECTRE_BEGIN_DECLS

/* Checks. Based on dbus-internals */
void _spectre_warn               (const char *format,
				  ...);
void _spectre_warn_check_failed  (const char *format,
				  ...);

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define _SPECTRE_FUNCTION_NAME __func__
#elif defined(__GNUC__) || defined(_MSC_VER)
#define _SPECTRE_FUNCTION_NAME __FUNCTION__
#else
#define _SPECTRE_FUNCTION_NAME "unknown function"
#endif

/* Define SPECTRE_VA_COPY() to do the right thing for copying va_list variables. 
 * config.h may have already defined SPECTRE_VA_COPY as va_copy or __va_copy. 
 */
#if !defined (SPECTRE_VA_COPY)
#  if defined (__GNUC__) && defined (__PPC__) && (defined (_CALL_SYSV) || defined (_WIN32))
#    define SPECTRE_VA_COPY(ap1, ap2)   (*(ap1) = *(ap2))
#  elif defined (SPECTRE_VA_COPY_AS_ARRAY)
#    define SPECTRE_VA_COPY(ap1, ap2)   memcpy ((ap1), (ap2), sizeof (va_list))
#  else /* va_list is a pointer */
#    define SPECTRE_VA_COPY(ap1, ap2)   ((ap1) = (ap2))
#  endif /* va_list is a pointer */
#endif /* !SPECTRE_VA_COPY */


/*
 * (code from GLib)
 * 
 * The _SPECTRE_LIKELY and _SPECTRE_UNLIKELY macros let the programmer give hints to 
 * the compiler about the expected result of an expression. Some compilers
 * can use this information for optimizations.
 *
 * The _SPECTRE_BOOLEAN_EXPR macro is intended to trigger a gcc warning when
 * putting assignments in the macro arg
 */
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _SPECTRE_BOOLEAN_EXPR(expr)                \
	__extension__ ({			   \
		int _spectre_boolean_var_;	   \
		if (expr)		           \
			_spectre_boolean_var_ = 1; \
		else				   \
			_spectre_boolean_var_ = 0; \
		_spectre_boolean_var_;		   \
	})
#define _SPECTRE_LIKELY(expr) (__builtin_expect (_SPECTRE_BOOLEAN_EXPR(expr), 1))
#define _SPECTRE_UNLIKELY(expr) (__builtin_expect (_SPECTRE_BOOLEAN_EXPR(expr), 0))
#else
#define _SPECTRE_LIKELY(expr) (expr)
#define _SPECTRE_UNLIKELY(expr) (expr)
#endif


#ifdef SPECTRE_DISABLE_ASSERT
#define _spectre_assert(condition)
#else
void _spectre_real_assert (int          condition,
			   const char  *condition_text,
			   const char  *file,
			   int          line,
			   const char  *func);
#define _spectre_assert(condition)                                         \
	_spectre_real_assert ((condition) != 0, #condition, __FILE__, __LINE__, _SPECTRE_FUNCTION_NAME)
#endif /* SPECTRE_DISABLE_ASSERT */

#ifdef SPECTRE_DISABLE_CHECKS
#define _spectre_return_if_fail(condition)
#define _spectre_return_val_if_fail(condition, val)
#else /* SPECTRE_DISABLE_CHECKS */
#define _spectre_return_if_fail(condition) do {				                             \
	_spectre_assert ((*(const char*)_SPECTRE_FUNCTION_NAME) != '_');                             \
	if (!(condition)) {                                                                          \
		_spectre_warn_check_failed ("%s: assertion `%s' failed (%s:%d)\n",                   \
					    _SPECTRE_FUNCTION_NAME, #condition, __FILE__, __LINE__); \
		return;                                                                              \
	} } while (0)

#define _spectre_return_val_if_fail(condition, val) do {                                             \
	_spectre_assert ((*(const char*)_SPECTRE_FUNCTION_NAME) != '_');                             \
	if (!(condition)) {                                                                          \
		_spectre_warn_check_failed ("%s: assertion `%s' failed (%s:%d)\n",                   \
					    _SPECTRE_FUNCTION_NAME, #condition, __FILE__, __LINE__); \
		return (val);                                                                        \
	} } while (0)
#endif /* SPECTRE_DISABLE_CHECKS */

/* String handling helpers */
char  *_spectre_strdup_printf (const char *format,
			       ...);
char  *_spectre_strdup        (const char *str);
int    _spectre_strncasecmp   (const char *s1,
			       const char *s2,
			       size_t      n);
int    _spectre_strcasecmp    (const char *s1,
			       const char *s2);
double _spectre_strtod        (const char *nptr,
			       char      **endptr);

SPECTRE_END_DECLS

#endif /* SPECTRE_UTILS_H */

