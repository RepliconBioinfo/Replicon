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
#ifndef THREADVARAIBLES_H
#define THREADVARAIBLES_H
#include "kiss.h"

/**
This class is a container for variables we want to keep separate between threads.

	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class threadVariables
{
	public:
		threadVariables();

		~threadVariables();

		double gauss() { return kss.gauss(); }

		/** return a random double number [0..1[
		**/
		double ran() { return kss.ran(); }

		/** return a random integer number [0..MAX[
		 **/
		int ranN ( int max ) {  return kss.ranN ( max );  }


		void seed ( unsigned long int S1, unsigned long int S2, unsigned long int S3 ) {kss.seed ( S1, S2, S3 ); }
	private:
		kiss kss;
};

#endif
