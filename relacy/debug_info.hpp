/*  Relacy Race Detector
 *  Copyright (c) 2008-2013, Dmitry S. Vyukov
 *  All rights reserved.
 *  This software is provided AS-IS with no warranty, either express or implied.
 *  This software is distributed under a license and may not be copied,
 *  modified or distributed except as expressly authorized under the
 *  terms of the license contained in the file LICENSE in this distribution.
 */

#ifndef RL_DEBUG_INFO_HPP
#define RL_DEBUG_INFO_HPP
#ifdef _MSC_VER
#   pragma once
#endif


namespace rl {
struct debug_info
{
    char const* func_;
    char const* file_;
    unsigned line_;

    debug_info(char const* func = "", char const* file = "", unsigned line = 0)
        : func_(func)
        , file_(file)
        , line_(line)
    {
    }
};

typedef debug_info const& debug_info_param;

}

#define RL_INFO ::rl::debug_info(__FUNCTION__, __FILE__, __LINE__)
#define $ RL_INFO

#endif
