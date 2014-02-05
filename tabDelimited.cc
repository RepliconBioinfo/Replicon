/*************************************************************************
 tabdelimited.cpp  -  description
 --------------------------------
 begin                : Sun Nov 19 2000
 copyright            : (C) 2000-2003 by Sven Bilke
 email                : sven@thep.lu.se
 *************************************************************************/
/*__ LICENSE_BLABLA*/
#include <string.h>
#include "stdinc.h"
#include "tabDelimited.h"

namespace GULI {
	class _tabLineParser {
	typedef char *CP;
	public:
		_tabLineParser(tabDelimited &master) : _M(master), _ntokens(0) {
			_alloc = _M.expectedLineMax();
			_Linebuffer =  _alloc ? new char[_alloc] : 0;
			_token = _alloc ? new CP[_alloc] : 0;
		}

		~_tabLineParser() {
			if(_token) delete [] _token;
			if(_Linebuffer) delete [] _Linebuffer;
		}

		void parseLine(string &line) {
			const char DELIMITER = _M.DELIMITER();
//			const char STDELIMITER = _M.STDELIMITER();

			size_t l = line.length() + 1;
			if (l  > _alloc) {
				_alloc = l;
				if (_token) delete[] _token;
				if (_Linebuffer) delete[] _Linebuffer;
				_token = new CP[_alloc];
				_Linebuffer = new char[_alloc];
			}
			strcpy(_Linebuffer, line.c_str());
			if ((l>=1) && (_Linebuffer[l - 1] == '\n')) _Linebuffer[l - 1] = 0;

			_ntokens = 0;
			if (_Linebuffer[0] && (_Linebuffer[0] != '#' || !_M.ignoreComment())) {
				_token[_ntokens++] = _Linebuffer;
				for (int i = 0; _Linebuffer[i]; i++) {
					if (_Linebuffer[i] == DELIMITER) {
						_Linebuffer[i] = 0;
						if (_Linebuffer[i + 1]) _token[_ntokens++] = _Linebuffer + i + 1;
					}
				}
			}
			return;
		}

		CP *argv() {return _token; }
		int argc(){ return _ntokens; }
	private:
		tabDelimited &_M;
		size_t _alloc;
		int  _ntokens;
		char *_Linebuffer;
		CP    *_token;
	};



	tabDelimited::tabDelimited(const int LM, const char DEL) :
		_lines_read(0) {
		_linemax = LM;
		_delimiter = DEL;
		_stDelimiter= '"';
		_ignoreComment = true;
		_observeStringDelimiter = false; // not even implemented, don't bother changing this
	}

	tabDelimited::~tabDelimited() {}

	int tabDelimited::consumeData(int nSource, argcT argc[], argvT argv[]) {
		std::cerr << "One day you will wake up and ask yourself \"How did I get here?\" (Calling pure virtual, that is....)\n";
		assert(0);
		return -1;
	}

	int tabDelimited::parse(vector<istream *> &in) {
		int retval = 0;
		const unsigned int nStream = in.size();
		if(!nStream) return retval;

		argvT argv[nStream];
		argcT argc[nStream];

		vector<_tabLineParser *> P(nStream);
		for(unsigned int i =0; i < nStream; ++i) P[i] = new _tabLineParser(*this);

		while (1) {
			int eof = 0;
			for(unsigned int i=0; i < nStream; ++i) {
				std::getline(*in[i], _line);
				P[i]->parseLine(_line);
				argv[i] = P[i]->argv();
				argc[i] = P[i]->argc();
				if(in[i]->eof()) ++ eof;
			}
			if(consumeData(nStream, argc, argv) ) {retval = -1; break; }
			++_lines_read;
			if(eof) break;
		}
		for(unsigned int i=0; i<nStream; ++i) delete P[i];
		return retval;
	}

	istream &tabDelimited::parse(istream &in) {
		_tabLineParser  P(*this);
		while (1) {
			std::getline(in, _line);
			P.parseLine(_line);
			consumeData(P.argc(), P.argv());
			++_lines_read;
			if(in.eof()) return in;
		}
		return in;
	}


	int _getline(const char *in, int len, string &l) {
		const char *t = in;
		while(len--) if(*t++ == '\n') break;
		int l1 = t - in;    // skip \n
		l.assign(in, l1-1); // but don;t include it in the line
		return l1;
	}


	void tabDelimited::parse(const char *in, int len) {
		_tabLineParser  P(*this);
		while (len > 0 && *in) {
			int l = _getline(in,  len, _line);
			P.parseLine(_line);
			consumeData(P.argc(), P.argv());
			++_lines_read;
			in += l;
			len -=l;
		}
	}


	int tabDelimited::parse(vector<tabDelimited::memoryFile> in) {
		int retval = 0;
		const unsigned int nStream = in.size();
		if(!nStream) return retval;

		argvT argv[nStream];
		argcT argc[nStream];

		vector<_tabLineParser *> P(nStream);
		for(unsigned int i =0; i < nStream; ++i) P[i] = new _tabLineParser(*this);

		while (1) {
			int eof = 0;
			for(unsigned int i=0; i < nStream; ++i) {
				int l = _getline(in[i].p, in[i].l, _line);
				P[i]->parseLine(_line);
				argv[i] = P[i]->argv();
				argc[i] = P[i]->argc();
				in[i].p  += l;
				in[i].l  -= l;
				if(!(in[i].l && *in[i].p)) {
					++eof;
				}
			}
			if(consumeData(nStream, argc, argv) ) {retval = -1; break; }
			++_lines_read;
			if(eof) break;
		}
		for(unsigned int i=0; i<nStream; ++i) delete P[i];
		return retval;
	}
} // NAMESPACE


