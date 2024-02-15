/** @file pd_include.h Include pushdown plugin headers */

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2013/14  Stefan Jacobi, Ramon Barakat, Sven Schneider, Anne-Kathrin Hess

*/

#ifndef FAUDES_PD_INCLUDE_H
#define FAUDES_PD_INCLUDE_H

#include "libfaudes.h"
#include "pd_pdgenerator.h"
#include "pd_attributes.h"
#include "pd_basics.h"
#include "pd_dotparser.h"
#include "pd_grammar.h"
#include "pd_parser.h"
#include "pd_gotogenerator.h"
#include "pd_gotoattributes.h"
#include "pd_alg_main.h"
#include "pd_alg_sub.h"
#include "pd_alg_first.h"
#include "pd_alg_lrp.h"
#include "pd_alg_lrm.h"

#include "pd_scopelogger.h"
#include "pd_debug.h"
#include "pd_lang_k.h"

#define FAUDES_PD_LAMBDA "lambda"
#define FAUDES_PD_LOOKAHEAD 2

//#define FAUDES_PD_PRINT_LOGFILE
#define FAUDES_PD_LOGFILE "pd_Logfile.txt"


#endif


/**
 

@defgroup PushdownPlugin Pushdown PlugIn


@ingroup AllPlugins

<p>
The pushdown plug-in synthesizes a minimally restrictive supervisor for 
a regular plant language (provided by a deterministic finite automaton (DFA)) 
and a context free specification language (provided by a deterministic pushdown 
automaton (DPDA)). The algorithm has been developed by Schneider and Schmuck;
see the <a href="../reference/index.html">user-reference</a> for literature. 
</p>

@section PushdownLicense License
<p>
This plug-in is distributed with libFAUDES and under the terms of the LGPL.
<br>
<br>
Copyright (c) 2013, Sven Schneider, Anne-Kathrin Schmuck, Stefan Jacobi
</p>

@section Contents
@brief



*/
