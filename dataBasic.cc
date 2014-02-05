/***************************************************************************
 basicData.cpp  -  description
 -------------------
 begin                : Tue Jul 23 2002
 copyright            : (C) 2002 by Sven Bilke
 email                : bilkes@mail.nih.gov
 ***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 by Sven Bilke                                      *
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
#include <functional>
#include <algorithm>
#include "stdinc.h"
#include "dataBasic.h"
namespace GULI {

		struct RESIZE: public std::unary_function<vector<float> &, void> {
			RESIZE(int s) :
				_s(s) {
			}
			void operator()(vector<float> &V) {
				V.resize(_s);
			}
			int _s;
		};

		dataBasic::dataBasic(int cols, int rows) :
			mutableIF(), _cols(cols), _rows(rows) {
			const string empty("");
			RESIZE R(_cols);
			_content = new vector<vector<float> > (rows);
			std::for_each(_content->begin(), _content->end(), R);
			_rowId = new vector<string> (_rows, empty);
			_colId = new vector<string> (_cols, empty);
		}

		dataBasic::dataBasic(dataIF &init) {
			_initcpy(init);
		}

		dataBasic::dataBasic(dataBasic &init) {
			_initcpy(init);
		}

		dataBasic::~dataBasic() {
			delete _content;
			delete _rowId;
			delete _colId;
		}

		void dataBasic::colPushBack(const string &id, vector<float> &D) {
			if (D.size() != _rows) throw new sizeMismatch;
			for (unsigned int i = 0; i < _rows; i++)
				(*_content)[i].push_back(D[i]);
			_colId->push_back(id);
			++_cols;
			return;
		}
		;

		void dataBasic::colPushBack(const string &id, float init) {
			for (unsigned int i = 0; i < _rows; i++)
				(*_content)[i].push_back(init);
			_colId->push_back(id);
			++_cols;
			return;
		}

		void dataBasic::colPopBack() {
			for (unsigned int i = 0; i < _rows; i++)
				(*_content)[i].pop_back();
			_colId->pop_back();
			--_cols;
		}

		void dataBasic::rowPushBack(const string &id, vector<float> &D) {
			if (D.size() != _cols) throw new sizeMismatch;
			_content->push_back(D);
			_rowId->push_back(id);
			++_rows;
		}
		;

		void dataBasic::rowPushBack(const string &id, float init) {
			vector<float> D(_cols, init);
			rowPushBack(id, D);
		}

		void dataBasic::rowPopBack() {
			_content->pop_back();
			_rowId->pop_back();
			--_rows;
		}

		void dataBasic::resize(int cols, int rows) {
			const string empty("");
			delete _content;
			delete _rowId;
			delete _colId;
			_cols = cols;
			_rows = rows;
			RESIZE R(_cols);
			_content = new vector<vector<float> > (_rows);
			std::for_each(_content->begin(), _content->end(), R);
			_rowId = new vector<string> (_rows, empty);
			_colId = new vector<string> (_cols, empty);
		}

		void dataBasic::_initcpy(dataIF &init) {
			_cols = init.cols();
			_rows = init.rows();
			RESIZE R(_cols);
			_content = new vector<vector<float> > (_rows);
			std::for_each(_content->begin(), _content->end(), R);
			_rowId = new vector<string> (_rows);
			_colId = new vector<string> (_cols);
			std::copy(init.beginRowLabel(), init.endRowLabel(), beginRowLabel());
			std::copy(init.beginColLabel(), init.endColLabel(), beginColLabel());
			for (unsigned int i = 0; i < _cols; i++) {
				std::copy(init.beginCol(i), init.endCol(i), beginCol(i));
			}
		}
} // namespace GULI::data
