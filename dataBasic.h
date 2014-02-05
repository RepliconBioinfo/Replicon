/***************************************************************************
 dataBasic.cc  -  description
 -------------------
 begin                : Tue Jul 23 2002
 copyright            : (C) 2002-2005 by Sven Bilke
 email                : bilkes@mail.nih.gov
 ***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2002-2005 by Sven Bilke                                 *
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
#ifndef ___GULI_DATA_BASIC_H
#define ___GULI_DATA_BASIC_H
//#include "guliconfig.h"
#include "stdinc.h"
#include "dataIF.h"
namespace GULI {
	/** A simple STL based implementation of a mutable Data-Container
	 * For annotation see the base classes dataIF and mutableIF.
	 **/
	class dataBasic: public mutableIF {
	public:
		/** instantiate a dataBasic data container with \a cols columns and \a rows rows */
		dataBasic(int cols = 0, int rows = 0);
		/** Instantiate a dataBasic data container with the geometry and content of the \a init dataIF object */
		dataBasic(dataIF &init);
		/** Copy constructor from the  \a init dataBasic object */
		dataBasic(dataBasic &init);

		virtual ~dataBasic();

		const unsigned int cols() const {
			return _cols;
		}
		const unsigned int rows() const {
			return _rows;
		}

		float & Data(int col, int row) {
			return (*_content)[row][col];
		}

		float Data(int col, int row) const {
			return (*_content)[row][col];
		}

		/** return the String ID for a row
		 **/
		string &rowId(int i) {
			return (*_rowId)[i];
		}

		/** return the String ID for a column
		 **/
		string &colId(int i) {
			return (*_colId)[i];
		}

		/** return the String ID for a row (const)
		 **/
		virtual const std::string &rowId(int i) const {
			return (*_rowId)[i];
		}

		/** return the String ID for a column (const)
		 **/
		virtual const std::string &colId(int i) const {
			return (*_colId)[i];
		}

		/** Change the size of the dataIF */
		virtual void resize(int col, int row);
		/** Append a column \a D with label \a id */
		virtual void colPushBack(const string &id, vector<float> &D);
		/** Append a column initialized with \a init and label \a id */
		virtual void colPushBack(const string &id, float init = 0.);
		/** Erase the last column */
		virtual void colPopBack();
		/** Append a row \a D with label \a id */
		virtual void rowPushBack(const string &id, vector<float> &D);
		/** Append a row initialized with \a init and label \a id */
		virtual void rowPushBack(const string &id, float init = 0.);
		/** Erase the last row */
		virtual void rowPopBack();

	private:
		vector<string> *_rowId, *_colId;
		vector<vector<float> > *_content;
		unsigned int _cols, _rows;
		void _initcpy(dataIF &init);
	};
} //namespace GULI::data
#endif
