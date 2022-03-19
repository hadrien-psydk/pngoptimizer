// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef CHUSTD_UT_STDAFX_H
#define CHUSTD_UT_STDAFX_H

#define _HAS_EXCEPTIONS 0
#include <gtest/gtest.h>

#include <chustd/chustd.h>

using namespace chustd;

inline std::ostream& operator<<(std::ostream& stream, const String& str)
{
    char tmp[200];
    str.ToUtf8Z(tmp);
    return stream << tmp;
}
#endif
