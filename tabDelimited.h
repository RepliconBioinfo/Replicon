/*************************************************************************
 tabdelimited.h  -  description
 --------------------------------
 begin                : Sun Nov 19 2000
 copyright            : (C) 2000-2008 by Sven Bilke
 email                : sven@thep.lu.se
 *************************************************************************/
/*__ LICENSE_BLABLA*/

#ifndef __TAB_DELIMITED_H
#define __TAB_DELIMITED_H
#include "stdinc.h"
namespace GULI {
	/** A Class to read tab-delimited files (or any other delimiter
	 the client has to implement a call-back (consumeData) to make
	 use of the tokenized data
	 */

	class tabDelimited {
	public:
		tabDelimited(const int LMAX = 2048, const char DEL = '\t');
		virtual ~tabDelimited();

		/** Parse a single  in-stream, will deliver via consumeData(int, char **)
		 */
		istream &parse(istream &);

		/** Parse multiple  in-streams, line by line. Results are delivered via consumeData(int, int[], **argv[])
		 */
		int parse(vector<istream *> &);


		// Allow to parse file in memory without relying on C+_ strstreams
		typedef struct MEMORY_FILE{
			MEMORY_FILE(const char *c, int L) : p(c), l(L) {}
			const char *p;
			int l;
		} memoryFile;

		/** Parse a single  in-memory file, will deliver via consumeData(int, char **)
		 */
		void parse(const char *p, int l);

		/** Parse multiple  in-streams, line by line. Results are delivered via consumeData(int, int[], **argv[])
		 */
		int parse(vector<memoryFile>);



		int linesRead() {return _lines_read;	}

		void resetLinesRead() {_lines_read = 0;	}

		bool observeStringDelimiter() { return _observeStringDelimiter; }
		bool observeStringDelimiter(bool o) { return _observeStringDelimiter = o; }
		bool ignoreComment() { return _ignoreComment; }
		bool ignoreComment(bool o) { return _ignoreComment = o; }
		char STDELIMITER()   { return _stDelimiter; }
		char STDELIMITER(char d)   { return _stDelimiter = d; }
		char DELIMITER()      { return _delimiter; }
		char DELIMITER(char d)      { return _delimiter = d; }
		unsigned int expectedLineMax() { return _linemax; }

		const string lastLineRead() { return _line; }

	protected:
		typedef char **argvT;
		typedef int  argcT;

		/** The client routine is invoked for each line read in with the
		 number of tokens (argc) and a list of tokens argv
		 */
		virtual void consumeData(int argc, char **argv)=0;

		/* Implementation for reading from multiple streams simultaneously */
		virtual int consumeData(int nSource, argcT argc[], argvT argv[]);

	private:
		string _line;
		char _delimiter;
		char _stDelimiter;
		int  _lines_read;
		int _linemax;

		bool _observeStringDelimiter, _ignoreComment;
	};
} // NAMESPACE
#endif

