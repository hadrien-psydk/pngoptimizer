///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_EVENT3_H
#define CHUSTD_EVENT3_H

#define ARGS_COUNT		3
#define ARGS_TEMPLATE	typename ARG1, typename ARG2, typename ARG3
#define ARGS_TYPE		ARG1, ARG2, ARG3
#define ARGS_NAME		arg1, arg2, arg3
#define ARGS_TYPENAME	ARG1 arg1, ARG2 arg2, ARG3 arg3

#include "EventTemplate.h"
#include "EventTemplateImpl.h"

#undef ARGS_COUNT
#undef ARGS_TEMPLATE
#undef ARGS_TYPE
#undef ARGS_NAME
#undef ARGS_TYPENAME

#endif // ndef CHUSTD_EVENT3_H
