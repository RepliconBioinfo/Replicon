/***************************************************************************
 *   Copyright (C) 2006 by Sven Bilke   *
 *   bilkes@mail.nih.gov   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "kineticmeas.h"
#include "cell.h"
#include <algorithm>
#include <assert.h>
float kineticMeas_avgHist(const vector<unsigned int> &a) {
  int N = 0;
  double f = 0.;
  for(unsigned int i=0; i < a.size(); ++i) {
    f += i * a[i];
    N += a[i];
  }
  if(!N) return -1;
  return f / (double) N;
}

kineticMeas::kineticMeas(const cell &c)
{
  _frags = _rfrags = 0;
  int s  = 0;
  for(unsigned int i=0; i < c.nChromosomes(); ++i) {
    int l = c.chromosome(i).length();
    s = (l > s) ? l : s;
  };
  _eye2eye.resize(s);
  _eye.resize(s);
  _hole.resize(s);


  std::fill(_eye2eye.begin(), _eye2eye.end(), 0);
  std::fill(_eye.begin(), _eye.end(), 0);
  std::fill(_hole.begin(), _hole.end(), 0);
  const unsigned int N = c.nFragment();
  if(!N) return;

  _fraglistT F;
  F.reserve(N);
  int curChr = c.nacentDna(0).chromosome;
  for(unsigned int n=0; n < N;  ++n) {
    const dna::fragment &f = c.nacentDna(n);
    if(curChr != f.chromosome ) {
      curChr = f.chromosome;
      _doneChr(F);
      F.clear();
    }
    F.push_back(f);
  }
  _doneChr(F);
  return;
}


kineticMeas::~kineticMeas()
{
}


 
typedef struct {
    bool operator()(const dna::fragment &a, const dna::fragment &b) {
      if(a.chromosome != b.chromosome) 
          return a.chromosome < b.chromosome;
      return a.begin < b.begin;
    }
} fragSort;

void kineticMeas::_doneChr(_fraglistT &F) {
  const unsigned int N=F.size();  
  std::sort(F.begin(), F.end(), fragSort());
  const int maxl = _eye2eye.size();
  for(unsigned int i=0; i < N - 1; ++i) {
    unsigned int eye   = F[i].end - F[i].begin;
    unsigned int hole  = F[i+1].begin - F[i].end;
    unsigned int e2e   = (F[i+1].begin + F[i+1].end - F[i].end - F[i].begin) / 2;
 //   assert( (eye < maxl) && (e2e < maxl) && (hole < maxl));
    _eye2eye[e2e]++;
    _hole[hole]++;
    _eye[eye]++;
  }
  _eye[F[N-1].end - F[N-1].begin]++;
  _frags  += N;
  _rfrags += N - 1;
  return;
}




