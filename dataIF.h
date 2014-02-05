/***************************************************************************
 DataIF.h  -  description
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
#ifndef __GULI_DATA_IF_H
#define __GULI_DATA_IF_H
#include <iterator>
#include "stdinc.h"
namespace GULI {
	using std::iterator_traits;

	/** The base class of all Data containers. It defines the (abstract) API
	 ** which allows to combine one data matrix with column and row labels.
	 ** Within this class, the API (and partial implementation) of iterators
	 ** to traverse data is also defined.
	 **/
	class dataIF {
	public:
		virtual ~dataIF() {
		}
		/** Return the number of Columns in the Data
		 **/
		virtual const unsigned int cols() const = 0;

		/** Return the number of Rows in the Data
		 **/
		virtual const unsigned int rows() const = 0;

		/** return Data entry at column \a col and row \a row
		 **/
		virtual float &Data(int col, int row) = 0;
		virtual float Data(int col, int row) const = 0;

		/** for convenience: a different way to invoke the Data -method
		 ** to access data at \a col and \a row
		 **/
		virtual float &operator()(int col, int row) {
			return Data(col, row);
		}

		float operator()(int col, int row) const {
			return Data(col, row);
		}

		/** Assignment operator, should be operator=, but I get wired polymorphic
		 * behavior if I overwrite the = operator.
		 */
		virtual dataIF &equal(const dataIF &R);


		/** return the label for row \a row
		 **/
		virtual string &rowId(int row) = 0;

		virtual const string &rowId(int row) const = 0;

		/** return the label  for  column \a col
		 **/
		virtual string &colId(int col) = 0;
		virtual const string &colId(int row) const = 0;

		/** The mode of the iterator **/
		typedef enum {
			_COL, _ROW
		} iterator_mode; /**< The mode of the iterator **/
	protected:

		/** Here we implement the generic ITERATOR following STL style to allow
		 *  to use algorithms and methods in the STL.
		 * The remplate argument \a M determines the direction in which the iterator
		 * traverses the data matrix
		 **/
		template<iterator_mode M> class _iteratorBase: public std::iterator<std::random_access_iterator_tag,
				float, int, float *, float &> {
		public:
			_iteratorBase() :
				 _cur(0), _other(0) {
			}

			/** Copy constructor **/
			_iteratorBase(const _iteratorBase<M> &i) {
				_cur = i._cur;
				_other = i._other;
			}
			_iteratorBase(int cur, int other) :
				_cur(cur), _other(other) {
			}

			/** Reset the pointer to a new (numerical) position **/
			inline void rePoint(int cur, int other) {
				_cur = cur;
				_other = other;
			}

			/** Increase to next referenced location (prefix) **/
			inline _iteratorBase<M> &operator++() {
				_cur++;
				return *this;
			}
			/** Increase to next referenced location (postfix) **/
			inline _iteratorBase<M> operator++(int) {
				_iterator<M> r(*this);
				_cur++;
				return r;
			}
			/** Decrease  location (prefix) **/
			inline _iteratorBase<M> &
			operator--() {
				_cur--;
				return *this;
			}
			/** Decrease  (postfix) **/
			inline _iteratorBase<M> operator--(int) {
				_iterator<M> r(*this);
				_cur--;
				return r;
			}
			/** Increase the iterator position by \a i steps **/
			inline _iteratorBase<M> &operator+=(int i) {
				_cur += i;
				return *this;
			}

			/** calculate iterator position after a decrease by \a i steps **/
			inline _iteratorBase<M> operator+(int i) const {
				_iteratorBase<M> r(*this);
				r._cur += i;
				return r;
			}
			/** Decerase the iterator position by \a i steps **/
			inline _iteratorBase<M> &operator-=(int i) {
				_cur -= i;
				return *this;
			}

			/** calculate iterator position after a decrease by \a i steps **/
			inline _iteratorBase<M> operator-(int i) {
				_iteratorBase<M> r(*this);
				r._cur -= i;
				return r;
			}
			/** Calculate the distance between two iterators **/
			 int operator-(const _iteratorBase<M> &B) const {
				return _cur - B._cur;
			}

			 /** true if \a B points to a location after the position pointed to by \a this **/
			inline bool operator<(const _iteratorBase<M> &B) const {
				return _cur < B._cur;
			}
			/** true if \a B points to a location after or equal to the position pointed to by \a this **/
			inline bool operator<=(const _iteratorBase<M> &B) const {
				return _cur <= B._cur;
			}
			/** true if \a B points to a location before the position pointed to by \a this **/
			inline bool operator>(const _iteratorBase<M> &B) const {
				return _cur > B._cur;
			}
			/** true if \a B points to a location before or equal to the position pointed to by \a this **/
			inline bool operator>=(const _iteratorBase<M> &B) const {
				return _cur >= B._cur;
			}
			/** true if \a B points to the same location  than pointed to by \a this **/
			inline bool operator==(const _iteratorBase<M> &B) const {
				return _cur == B._cur;
			}
			/** true if \a B points to a different location than pointed to by \a this **/
			inline bool operator!=(const _iteratorBase<M> &B) const {
				return _cur != B._cur;
			}
			/** Assign the location pointet to by \a B to \a this **/
			inline _iteratorBase<M> &operator=(const _iteratorBase<M> &B) {
				_cur = B._cur;
				_other = B._other;
				return *this;
			}

			/** Assign the location pointed to by \a B to \a this, where \a B may traverse
			 ** in a different direction than \a this
			 **/
			template<iterator_mode S> 	_iteratorBase<M> &assign(_iteratorBase<S> s) {
				assert(0); // Something was wrong here in a context of someapplication. So, if we call this function, PLEASE
				           // check it is doing what you think it is doing. For the debugged program, I rephrased the statement
				           // avoiding the "assign" funtion, but to be sure. I put in the assert(0) here....
				if (S == M) return *this = s;
				_cur = s._other;
				_other = s._cur;
				return *this;
			}

			/** Return the coordinate for the "running" direction determined by the mode \a M **/
			int curCoord() const {
				return _cur;
			}


		protected:
			int _cur;
			int _other;
		};


		template<iterator_mode M> 	class _iterator : public _iteratorBase<M> {
		public:
			_iterator() : _iteratorBase<M>(), _base(0) {}

			_iterator(const _iterator<M> &i) : _iteratorBase<M>(i){
				_base = i._base;
			}
			/** Return the label of the current position in the "running" direction determined
			 ** by the mode \a M
			 **/

			_iterator(dataIF *t, int cur, int other) : _iteratorBase<M>(cur, other) {
					_base = t;
			}


			std::string &id() const {
				return (M == _ROW) ? this->_base->rowId(this->_cur) : this->_base->colId(this->_cur);
			}

			/** dereference to a location \a i positions away from the current location */
			inline float &operator[](int i) const {
				return (M == _ROW) ? (*this->_base)(this->_other, this->_cur + i) : (*this->_base)(this->_cur + i, this->_other);
			}

			/** dereference current location */
			inline float &operator*() const {
				return (M == _ROW) ? (*this->_base)(this->_other, this->_cur) : (*this->_base)(this->_cur, this->_other);
			}

			_iterator<M> &operator=(const _iterator &B) {
				 *((_iteratorBase<M> *) this) = B;
				 _base = B._base;
				 return *this;
        	}


			/** calculate iterator position after a decrease by \a i steps **/
			inline _iterator<M> operator-(int i) {
				_iterator<M> R(*this);
				R += i;
				return R;
			}

			/** calculate iterator position after a increase by \a i steps **/
			inline _iterator<M> operator+(int i) {
				_iterator<M> R(*this);
				R += i;
				return R;
			}

			/** Calculate the distance between two iterators **/
            template <class it>  int operator-(const it &B) const {
				return static_cast<const _iteratorBase<M> *>(this)->operator-(B);
            }



			template<iterator_mode S> 	_iterator &assign(_iterator<S> s) {
 				static_cast<_iteratorBase<M> *>(this)->assign(s);
 				_base = s._base;
 				return *this;
 			}
		protected:
			mutable dataIF *_base;
		};


		template<iterator_mode M> 	class _constIterator : public _iteratorBase<M> {
		public:
			_constIterator() : _iteratorBase<M>(){}

			template <class it> _constIterator(const it &i) : _iteratorBase<M>(i){
				_base = i._base;
			}


			_constIterator(const dataIF *t, int cur, int other) : _iteratorBase<M>(cur, other) {
					_base = t;
			}

			/** Return the label of the current position in the "running" direction determined
			 ** by the mode \a M
			 **/

			const std::string &id() const {
				return (M == _ROW) ? this->_base->rowId(this->_cur) : this->_base->colId(this->_cur);
			}

			/** dereference to a location \a i positions away from the current location */
			inline float operator[](int i) const {
				return (M == _ROW) ? (*this->_base)(this->_other, this->_cur + i) : (*this->_base)(this->_cur + i, this->_other);
			}

			/** dereference current location */
			inline float operator*() const {
				return (M == _ROW) ? (*this->_base)(this->_other, this->_cur) : (*this->_base)(this->_cur, this->_other);
			}


			 template <class it> _constIterator<M> &operator=(const it &B) {
				 *((_iteratorBase<M> *) this) = B;
				 _base = B._base;
				 return *this;
			 }

 			 /** calculate iterator position after a decrease by \a i steps **/
			 inline _constIterator<M> operator-(int i) {
				_constIterator<M> R;
				static_cast<_iteratorBase<M> > (R).assign(static_cast<_iteratorBase<M> *>(this)->operator-(i));
				return R;
			 }

			 /** calculate iterator position after a decrease by \a i steps **/
			 inline _constIterator<M> operator+(int i) {
				_constIterator<M> R;
				static_cast<_iteratorBase<M> > (R).assign(static_cast<_iteratorBase<M> *>(this)->operator+(i));
				return R;
			 }


			 /** Calculate the distance between two iterators **/
             template <class it>  int operator-(const it &B) const {
				return static_cast<const _iteratorBase<M> *>(this)->operator-(B);
             }

			template<class it, iterator_mode S> _constIterator &assign(it s) {
	 			static_cast<_iteratorBase<M> *>(this)->assign<S>(s);
	 			_base = s._base;
	 			return *this;
	 		}


		protected:
			const dataIF *_base;
		};



		/** An "indexed" iterator. This one traverses a row or column (depending on the mode) along
		 ** a set of indices. This is specifically useful if one wants to select classes or subsets from
		 ** the data
		 **/
		template<iterator_mode M> class _indexedIterator: public _iterator<M> {
		public:
			_indexedIterator(const _indexedIterator &i) : 	_iterator<M> (i) {
				_index.resize(i._index.size());
				std::copy(i._index.begin(), i._index.end(), _index.begin());
			}

			template<class it> 	_indexedIterator(dataIF *t, it begin, it end, int col, int row) :
				_iterator<M> (t, col, row) {
				_index.resize(end - begin);
				std::copy(begin, end, _index.begin());
			}

			inline _indexedIterator &operator=(const _indexedIterator &B) {
				_iterator<M>::operator=(B);
				_index.resize(B._index.size());
				std::copy(B._index.begin(), B._index.end(), _index.begin());
				return *this;
			}

			inline float &operator[](int i) {
				return (M == _ROW) ? (*_iterator<M>::_base)(
						_iterator<M>::_other, _index[_iterator<M>::_cur + i])
						: (*_iterator<M>::_base)(
								_index[_iterator<M>::_cur + i],
								_iterator<M>::_other);
			}

			inline float &operator*() {
				return (M == _ROW) ? (*_iterator<M>::_base)(
						_iterator<M>::_other, _index[_iterator<M>::_cur])
						: (*_iterator<M>::_base)(_index[_iterator<M>::_cur],
								_iterator<M>::_other);
			}

			std::string &id() {
				return (M == _ROW) ? _iterator<M>::_base->rowId(_index[_iterator<M>::_cur])
						           : _iterator<M>::_base->colId(_index[_iterator<M>::_cur]);
			}

		protected:
			vector<int> _index;
		};

	public:
		/**  A ROW iterator iterates "within" a fixed ___column___, i.e. the "++" operator increases the
		 **  row-index (thats why it is called  ROW iterator) and keeps the column index fixed.
		 **/
		typedef _iterator<_ROW> rowIterator;
		typedef _constIterator<_ROW> constRowIterator;

		/** An indexed version (i.e. allowing sub-selections or "within" change of order) of the
		 above iterator.
		 **/
		typedef _indexedIterator<_ROW> indexedRowIterator;

		/**  A COLUMN iterator iterates "within" a fixed
		 ___row___, i.e. the "++" operator increases the column-index (thats why it is called
		 column iterator) and keeps the row index fixed.
		 **/
		typedef _iterator<_COL> colIterator;
		typedef _constIterator<_COL> constColIterator;

		/** An indexed version (i.e. allowing sub-selections or "within" change of order) of the
		 above iterator.
		 **/
		typedef _indexedIterator<_COL> indexedColIterator;



		/** returns a RowIterator (this is, walking within a column !) at the  beginnning of
		 *  \a column
		 **/
		rowIterator beginCol(int column) {
			return rowIterator(this, 0, column);
		}


		constRowIterator beginCol(int column) const {
			return constRowIterator(this, 0, column);
		}

		/** returns a RowIterator (this is, walking within a  column !) at the  end of
		 *  \a column
		 **/
		rowIterator endCol(int column) {
			return rowIterator(this, rows(), column);
		}

		constRowIterator endCol(int column) const {
			return constRowIterator(this, rows(), column);
		}

		/** returns a ColumnIterator visiting members of a row at the  beginning of
		 *  \a row
		 **/
		colIterator beginRow(int row) {
			return colIterator(this, 0, row);
		}

		constColIterator beginRow(int row) const {
			return constColIterator(this, 0, row);
		}
		/** returns a ColumnIterator visiting members of a row at the end of
		 *  \a row
		 **/
		colIterator endRow(int row)  {
			return colIterator(this, cols(), row);
		}

		constColIterator endRow(int row) const {
			return constColIterator(this, cols(), row);
		}

		/** LABELS **/

		/** An iterator inheriting all features from \a baseIterator, but re-directing the operator*
		 ** to return the label (corresponding to the movement direction of the underlying iterator)
		 **/
		template<class baseIterator>  class _labelIterator: public baseIterator {
		public:
			_labelIterator(const baseIterator &b) {
				baseIterator::operator=(b);
			}

			/** calculate iterator position after a increase by \a i steps **/
			inline _labelIterator<baseIterator> operator+(int i) {
				_labelIterator<baseIterator> R(*this);
				R += i;
				return R;
			}


			inline string &operator*()  {
				return baseIterator::id();
			}
		};



		/** An iterator inheriting all features from \a baseIterator, but re-directing the operator*
		 ** to return the label (corresponding to the movement direction of the underlying iterator)
		 **/
		template<class baseIterator>  class _constLabelIterator: public baseIterator {
		public:
			template <class it> _constLabelIterator(const it &b) {
				baseIterator::operator=(b);
			}

			inline const string &operator*() const {
				return baseIterator::id();
			}
		};

		typedef _labelIterator<rowIterator>      rowLabelIterator;
		typedef _constLabelIterator<constRowIterator> constRowLabelIterator;
		typedef _labelIterator<colIterator>      colLabelIterator;
		typedef _constLabelIterator<constColIterator> constColLabelIterator;



		/** An iterator pointing to the  first rowLabel
		 **/
		rowLabelIterator beginRowLabel() {
			return rowLabelIterator (rowIterator(this, 0, 0));
		}

		/** An iterator pointing to the  first rowLabel
		 **/
		constRowLabelIterator beginRowLabel() const {
			return constRowLabelIterator (constRowIterator(this, 0, 0));
		}

		/** An iterator pointing behind the last rowLabel
		 **/
		rowLabelIterator endRowLabel() {
			return rowLabelIterator (rowIterator(this, rows(), 0));
		}

		/** An const-iterator pointing behind the last rowLabel
		 **/
		constRowLabelIterator endRowLabel() const {
			return constRowLabelIterator (constRowIterator(this, rows(), 0));
		}


		/** An iterator pointing to the  first colLabel
		 **/
		colLabelIterator beginColLabel() {
			return colLabelIterator (colIterator(this, 0, 0));
		}

		/** An iterator pointing to the  first colLabel
		 **/
		constColLabelIterator beginColLabel() const {
			return constColLabelIterator (constColIterator(this, 0, 0));
		}

		/** An iterator pointing behind the last colLabel
		 **/
		colLabelIterator endColLabel() {
			return colLabelIterator (colIterator(this, cols(), 0));
		}

		/** An iterator pointing behind the last colLabel
		 **/
		constColLabelIterator endColLabel() const {
			return constColLabelIterator(constColIterator(this, cols(), 0));
		}
	};

	/** a mutable dataIF extends the dataIF API to methods which allow to manipulate the layout
	 ** of the dataIF and to append / remove rows/columns.
	 **/
	class mutableIF: public dataIF {
	public:
		/** This class gets thrown if the size of a pushed vetor does not agree with the
		 ** dimension of the dataIF
		 **/
		class sizeMismatch {
		}; // gets thrown when pushing wrong type of vectors
		/** Change the size of the dataIF */
		virtual void resize(int col, int row) = 0;
		/** Append a column \a D with label \a id */
		virtual void colPushBack(const std::string &id, std::vector<float> &D) = 0;
		/** Append a column initialized with \a init and label \a id */
		virtual void colPushBack(const std::string &id, float init = 0.) = 0;
		/** Erase the last column */
		virtual void colPopBack() = 0;
		/** Append a row \a D with label \a id */
		virtual void rowPushBack(const std::string &id, std::vector<float> &D)= 0;
		/** Append a row initialized with \a init and label \a id */
		virtual void rowPushBack(const std::string &id, float init = 0.) = 0;
		/** Erase the last row */
		virtual void rowPopBack() = 0;

		/** Append a column of data starting at \a begin, ending at \a end with label \a id */
		template<class it> 	void colPushBack(const std::string &id, it begin, it end) {
			if (end - begin != rows()) throw(new sizeMismatch);
			colPushBack(id);
			std::copy(begin, end, beginCol(cols() - 1));
		}

		/** Append a row of data starting at \a begin, ending at \a end with label \a id */
		template<class it>	void rowPushBack(const std::string &id, it begin, it end) {
			if (end - begin != cols()) throw(new sizeMismatch);
			rowPushBack(id);
			std::copy(begin, end, beginRow(rows() - 1));
		}

		virtual dataIF &equal(const dataIF &R);
	};
} // namespace GULI
#endif // defined __DATAIF
