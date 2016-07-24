///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_EVENT1_H
#define CHUSTD_EVENT1_H

#define ARGS_COUNT		1
#define ARGS_TEMPLATE	typename ARG
#define ARGS_TYPE		ARG
#define ARGS_NAME		arg
#define ARGS_TYPENAME	ARG arg

#include "EventTemplate.h"
#include "EventTemplateImpl.h"

#undef ARGS_COUNT
#undef ARGS_TEMPLATE
#undef ARGS_TYPE
#undef ARGS_NAME
#undef ARGS_TYPENAME

#endif // ndef CHUSTD_EVENT1_H
