/*
 * fMBT, free Model Based Testing tool
 * Copyright (c) 2015, Intel Corporation.
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

#include "alphabet.hh"

void Alphabet_updater::update(Alphabet* alpha) {
  Alphabet* a=alpha?alpha:dynamic_cast<Alphabet*>((Alphabet*)(this));
  for (std::list<Alphabet_update*>::iterator it=alphabet_update_callbacks.begin();
       it != alphabet_update_callbacks.end(); ++it) {
    (*it)->alphabet_update(a);
  }
}

Alphabet_updater::Alphabet_updater() {
  all_alphabet_updaters.push_back(this);
}

Alphabet_update::~Alphabet_update() {
  Alphabet_updater::global_remove_alphabet_update(this);
}

Alphabet_updater::~Alphabet_updater() {
  all_alphabet_updaters.remove(this);
}

int Alphabet::action_number(const std::string& s)
{
  for(size_t i=0;i<action_names.size();i++) {
    if (action_names[i]==s) {
      return i;
    }
  }
  return -1;
}

std::list<Alphabet_updater*> Alphabet_updater::all_alphabet_updaters;

void Alphabet_updater::global_remove_alphabet_update(Alphabet_update* u) {
  for(std::list<Alphabet_updater*>::iterator i=all_alphabet_updaters.begin();
      i!=all_alphabet_updaters.end();++i) {
    (*i)->remove_alphabet_update(u);
  }
}
