/***************************************************************************
 DataIF.cpp  -  description
 -------------------
 begin                : Fri May 3 2002
 copyright            : (C) 2002 by sven
 email                : sven@gonzo.thep.lu.se
 ***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2002-2012 by Sven Bilke                                 *
 *   bilkes@mail.nih.gov                                                   *
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

#include "dataIF.h"

namespace GULI {
	GULI::dataIF &GULI::dataIF::equal(const dataIF &R) {
			assert(cols() == R.cols() && rows() == R.rows());
			for(size_t i=0; i < cols(); ++i) std::copy(R.beginCol(i), R.endCol(i), beginCol(i));
			std::copy(R.beginRowLabel(), R.endRowLabel(), beginRowLabel());
			std::copy(R.beginColLabel(), R.endColLabel(), beginColLabel());
			return *this;
	}


	GULI::dataIF &GULI::mutableIF::equal(const dataIF &R) {
		if(cols() != R.cols() || rows() != R.rows()) resize(R.cols(), R.rows());
		return dataIF::equal(R);
	}

}


