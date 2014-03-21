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
#define _RANDOM_INTERNAL_
#include "random_proxy.hh"
#include "params.hh"
#include <cstdlib>

Random_Proxy::Random_Proxy(Random* _r): r(_r) {
  callback_proxy.add_call(std::string("random"),this,(Proxy::func_ptr_t) & Random_Proxy::call);
  add_call(std::string("get"),this,(Proxy::func_ptr_t) & Random_Proxy::get);
  add_call(std::string("set"),this,(Proxy::func_ptr_t) & Random_Proxy::set);  
}

std::string Random_Proxy::stringify() {
  if (r)
    return r->stringify();

  return Random::stringify();
}

#include "random_c.hh"

bool Random_Proxy::get(std::string params,std::string& ret_str)
{
  Random_C* rr;
  if (r && r->status && (rr=dynamic_cast<Random_C*>(r))) {
    ret_str=to_string(rr->seed);
    return true;
  }
  return false;
}

bool Random_Proxy::set(std::string params,std::string& ret_str)
{
  Random_C* rr;
  int seed = atoi(params.c_str());
  if (r && r->status && (rr=dynamic_cast<Random_C*>(r))) {
    rr->initial_seed=rr->seed=seed;
    return true;
  }
  return false;
}

unsigned long Random_Proxy::rand() {
  return r->rand();
}

double Random_Proxy::drand48() {
  return r->drand48();
}
