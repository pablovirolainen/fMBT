/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2011, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "end_condition.hh"

#include <stdlib.h>

#include <stdio.h> // DEBUG

#ifndef DROI
#include <glib.h>
#endif

End_condition::End_condition(Verdict::Verdict v, Counter c, std::string* p)
  : verdict(v), counter(c), param(p),
    param_float(-1.0), param_long(-1), param_time(-1)
{
    switch (counter) {

    case STEPS:
      param_long = atol(param->c_str());
      fprintf(stderr, "end_condition(STEPS: '%s' == %d", param->c_str(), param_long);
      status = true;
      break;

    case COVERAGE:
      param_float = atof(param->c_str());
      status = true;
      break;

    case STATETAG:
      // param already contains the state tag string, but it cannot be
      // converted into index (param_long) because the model is not
      // initialized
      status = true;
      break;

    case DURATION:
    {
      param_time = -1;
#ifndef DROI
      char* out = NULL;
      int stat;
      std::string ss = "date --date='" + *param + "' +%s";

      if (g_spawn_command_line_sync(ss.c_str(), &out, NULL, &stat, NULL)) {
        if (!stat) {
          param_time = atoi(out);
          status = true;
        } else {
          errormsg = "Parsing 'duration' parameter '" + *param + "' failed.";
          errormsg += " Date returned an error when executing '" + ss + "'";
          status = false;
        }
      } else {
        errormsg = "Parsing 'duration' parameter '" + *param + "' failed, could not execute '";
        errormsg += ss + "'";
        status = false;
      }
#else
      char* endp;
      long r = strtol(param->c_str(), &endp, 10);
      if (*endp == 0) {
        param_time = r;
        status = true;
      } else {
        // Error on str?
        errormsg = "Parsing duration '" + param + "' failed.";
        status = false;
      }
#endif
    }

    } /* switch (counter) ... */
    
}
