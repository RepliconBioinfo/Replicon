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
#ifndef KINETICMEAS_H
#define KINETICMEAS_H
#include <vector>
#include "dna.h"
class cell;

/**
Measure "hole", "eye" and "eye2eye" which were  used in Jerrick, J., et al, JMB 320, 741-750 to investigate kinetic parameters in real cells.


	@author Sven Bilke <bilkes@mail.nih.gov>
*/
float kineticMeas_avgHist(const vector<unsigned int> &f);

class kineticMeas{
public:
  kineticMeas(const cell &);
  ~kineticMeas();

  int eye2eye(int dist) { return _eye2eye[dist]; }
  int hole(int len)     { return _hole[len]; }
  int eye(int len)      { return _eye[len];  }
  float eye2eye()       { return kineticMeas_avgHist(_eye2eye); }
  float hole()          { return kineticMeas_avgHist(_hole); }
  float eye()           { return kineticMeas_avgHist(_eye); }

  private:
    typedef vector<dna::fragment> _fraglistT;
    vector<unsigned int> _eye2eye, _hole, _eye;
    unsigned int _frags, _rfrags;
    void _doneChr(_fraglistT &);
};

#endif
