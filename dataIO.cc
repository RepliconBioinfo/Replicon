/***************************************************************************
 dataIO.cpp  -  description
 -------------------
 begin                : Tue Jul 23 2002
 copyright            : (C) 2002 by sven
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
#include <string.h>
#include <fstream>
#include "stdinc.h"
#include "dataIO.h"
#include "dataIF.h"
#include "tabDelimited.h"
#include "gzstream.h"

namespace GULI {
	namespace IO {
		unsigned int maxrowlen = 204800; /**< The size of the input buffer for read-operations */

		/** This class implements reading of TAB-delimited files, where the
		 *  first line contains the column ID's. The "filer" argument allows
		 * to select specific rows/columns from the datafile. Without this
		 * argument it is assumed that the first column contains a label for the row,
		 * and the remaining entries are numbers. Note in this automatic mode, the numbers
		 * of columns is determined from the number of columns in the first line.
		 * An warning message is sent to STDERR and the line is ignored if the length of
		 * any subsequent line differs from the first line in the datafile
		 **/
		class TDL: public tabDelimited {
		public:
			TDL(mutableIF &t, filter *f) :
				tabDelimited(maxrowlen), FI(f), TA(t) {
				_first_line_is_header = true;
			}

			/** Read data from a stream
			 **/
			istream &read(istream &INS) {
				_lines_read = 0;
				return parse(INS);
			}

			/** Read data from file "filename". The special filename "-" is interpreted
			 *  as STDIN
			 **/
			int read(const char *filename) {
				_lines_read = 0;
				if (!strcmp("-", filename)) {
					parse(std::cin);
				} else {
					igzstream INS(filename);
//					ifstream INS(filename);
					_lines_read = 0;

					if (!INS.is_open()) return -1;
					parse(INS);
					INS.close();
				}
				return 0;
			}

		protected:
			virtual void consumeData(int argc, char **argv) {
				if (!_lines_read) {
					if (FI) {
						TA.resize(FI->dataCols(), 0);
					} else {
						// For automatic files, extract number of columns from the data just read
						if (!argc) return;
						TA.resize(argc - 1, 0);
					}
				}
				if (!_lines_read++ && _first_line_is_header) {
					if (argc - 1 >= (int) TA.cols()) {
						for (unsigned int i = 0; i < TA.cols(); i++)
							TA.colId(i) = argv[i + 1];
						return;
					} else {
						string unknown = "UCID";
						for (unsigned int i = 0; i < TA.cols(); i++)
							TA.colId(i) = unknown;
					}
				}

				if (!FI) { // Automatic Format. Col 0 is ID, rest is data
					if (argc - 1 != (int) TA.cols()) { // Mismatch in length detected
						_line_length_error();
						return;
					}
					string id = argv[0];
					float *buf = new float[argc - 1];
					for (int i = 1; i < argc; i++)
						buf[i - 1] = (float) atof(argv[i]);
					TA.rowPushBack(id, buf, buf + argc - 1);
					delete[] buf;
					return;
				}

				if (!FI->qualCheck(argc, argv)) return;
				if (argc < FI->dataFileCols()) {
					_line_length_error();
					return;
				}
				string id;
				if (FI->rowId() > -1) id = argv[FI->rowId()];
				else id = "URID";

				float *buf = new float[TA.cols()];
				int ex, nc;
				for (ex = 0, nc = 0; nc < FI->dataFileCols(); nc++) {
					if (FI->colFilter(nc)) {
						buf[ex++] = (float) atof(argv[nc]);
					}
				}
				TA.rowPushBack(id, buf, buf + TA.cols());
				delete[] buf;
				return;
			}
		private:
			void _line_length_error() {
				std::cerr << "Differing number of columns detected in line "
						<< tabDelimited::linesRead() << "\n";
			}
			filter *FI;
			mutableIF &TA;
			bool _first_line_is_header;
			int _lines_read;
		};

		istream &read(std::istream &INS, mutableIF &t, filter *f) {
			TDL method(t, f);
			return method.read(INS);
		}

		int read(const char *filename, mutableIF &t, filter *f) {
			TDL method(t, f);
			return method.read(filename);
		}

		int read(const string &filename, mutableIF &t, filter *f) {
			return read(filename.c_str(), t, f);
		}

		int write(const char *filename, const dataIF &s) {
			ostream *out;
			ofstream OUTS;

			if (!strcmp(filename, "-")) {
				out = &std::cout;
			} else {
				OUTS.open(filename);
				if (!OUTS) return -1;
				out = &OUTS;
			}
			for (unsigned int i = 0; i < s.cols(); i++)
				*out << "\t" << s.colId(i);
			*out << std::endl;
			for (unsigned int i = 0; i < s.rows(); i++) {
				*out << s.rowId(i);
				for (unsigned int j = 0; j < s.cols(); j++)
					*out << "\t" << s.Data(j, i);
				*out << std::endl;
			}
			if (strcmp(filename, "-")) OUTS.close();
			return 0;
		}

		int write(const string &filename, const dataIF &s) {
			return write(filename.c_str(), s);
		}
	}
} // NAMESPACE

