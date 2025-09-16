/* cfl_transset.cpp -- store a set of transitions (transition relation) */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

       
#include "cfl_transset.h"

namespace faudes {




// apply a relabeling map to a TransSet 
void ApplyRelabelMap(const RelabelMap& rMap, const TransSet& rSet, TransSet& rRes) {
  TransSet& inselem = *rSet.New();
  TransSet delelem;
  NameSet::Iterator tit, tit_end;
  TransSet::Iterator dit=rSet.Begin();
  TransSet::Iterator dit_end=rSet.End();
  for(;dit!=dit_end;++dit) {
    if(!rMap.Exists(dit->Ev)) continue;
    delelem.Insert(*dit);
    const NameSet& target=rMap.Target(dit->Ev);
    tit=target.Begin();
    tit_end=target.End();
    for(;tit!=tit_end;++tit) {
      Transition trans=*dit;
      AttributeVoid* attrp = rSet.Attribute(*dit).Copy();
      trans.Ev= *tit;
      inselem.Insert(trans);
      inselem.Attribute(trans,*attrp);
      delete attrp;      
    }
  }
  rRes.Assign(rSet);
  rRes.EraseSet(delelem);
  rRes.InsertSet(inselem);
  delete &inselem;
}


  
} //namespace
