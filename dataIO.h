/***************************************************************************
 dataIO.h  -  description
 -------------------
 begin                : Fri May 3 2002
 copyright            : (C) 2002 by Sven Bilke
 email                : sven@gonzo.thep.lu.se
 ***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2002-2009 by Sven Bilke                                 *
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

#ifndef __GULI__DATAIO_H
#define __GULI__DATAIO_H
#include <string>
namespace GULI {
	class mutableIF;
	class dataIF;

	namespace IO {
		/** A descriptor for the format of an input file. **/
		class filter {
		public:
			/** The number of columns in the input stream **/
			virtual int dataFileCols() const = 0;
			/** The number of columns of data to be extracted **/
			virtual int dataCols() const = 0;
			/** True if input column \a i should be used, flase if it should be ignored **/
			virtual bool colFilter(int i) const = 0;
			/** False, if the line with \a argc columns found in the stream, with \a argv
			 ** pointing to the columns, should be used or be ignored
			 **/
			virtual bool qualCheck(int argc, char **argv) const {
				return true;
			}
			/** The  index of the column in the input stream holding the row-label **/
			virtual int rowId() const = 0;

			virtual ~filter() {
			}
			;
		};

		/** A simple read-method. It should be developed further, right now it does not
		 ** support switching between different formats etc. But why should we, we support
		 ** only one format at this time, anyway
		 ** read from a file \a filename to the dataIF \a target using the format \a filter
		 **/
		int read(const char *filename, mutableIF &target, filter *f = 0);

		/** Re-implemented read(char *...) which allows to use std::string arguments
		 **/
		int read(const std::string &fnam, mutableIF &target, filter *f = 0);

		/** Re-implementation of read which allows to read from a C++ stream
		 ** read from a stream \a IN to the dataIF \a target using the format \a filter
		 **/
		std::istream &
		read(std::istream &IN, mutableIF &target, filter *f = 0);

		/** A simple write-method. It should be developed further, right now it does not
		 ** support switching between different formats etc. But why should we, we support
		 ** only one format at this time, anyway
		 ** write to a file with name  \a filename from  the dataIF \a source using
		 **/
		int write(const char *filename, const dataIF &source);

		/** Re-implemented which(char *...) which allows to use std::string arguments
		 **/
		int write(const std::string &fnam, const dataIF &source);

	} // namespace IO
} //namespace MULI

#endif // defined __DATAIO_H
