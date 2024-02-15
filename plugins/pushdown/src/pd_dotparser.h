/** @file pd_dotparser.h  parser functions for DOT language */

/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2014 Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess


  (DOT language see: http://www.graphviz.org/content/dot-language )

  The following is an abstract grammar defining the DOT language for Pushdowngenerator.
  Terminals are shown in quotes.
  Literal characters are given in single quotes.
  Parentheses ( and ) indicate grouping when needed.
  Square brackets [ and ] enclose optional items.
  Vertical bars | separate alternatives


  graph 		: 	[ "strict" ] ("graph" | "digraph") [ ID ] '{' stmt_list '}'
  stmt_list		: 	state_stmts ';' init_stmt ';' sb_stmt [ ';' event_stmts] [ ';' trans_stmts ]
  state_stmts	:	ID [marked_stmt] [ ';' state_stmt ]
  marked_stmt	:	'[' "double" ']'
  init_stmt		:	'->' ID
  sb_stmt		:	ID	'[' "attr" '=' "stackbottom" ']'
  event_stmts	:	ID '[' "attr" '=' attr_stmts ']' [ ';' event_stmts]
  attr_stmts	:	('c' | 'C') [('o' | 'O')] [('f' | 'F')][('a' | 'A')]
  trans_stmts	:	ID '->' ID '[' "label" '=' ID ',' '[' stack_sym ']' '[' stack_sym ']' ]
  stack_sym	:	[ID [ ',' ID] ]

 */

#ifndef FAUDES_PD_DOTPARSER_H
#define FAUDES_PD_DOTPARSER_H


#include "pd_pdgenerator.h"

namespace faudes {

// split string by separator
extern FAUDES_API std::vector<std::string> split(const std::string& rStr, const std::string& rSep,
		const std::string& rFilename, const int lineNr);

/**
 * Get string literal between quotations ("ide") to &resText and return the position after the second quotation (").
 *
 * @param rPos
 * 	position where to search from
 *
 * @param rInput
 * 	input string
 *
 * @param rRestString
 * 	return the string after the second quotation
 *
 * @param rFilename
 * 	name of current file
 *
 * @param lineNr
 * 	current line number
 *
 * @return std::string::size_type
 * 	position after the second quotation
 */
extern FAUDES_API std::string::size_type getIde(std::string::size_type& rPos,
		const std::string& rInput, std::string& rRestString,
		const std::string& rFilename, const int lineNr);


/**
 * check if given string is a replacement for lambda
 * Replacements are empty string (""),
 * underline ("_") ,
 * html lambda code ("&lambda;")
 * or "lambda"
 *
 * @param str
 * 		string to check
 *
 * @return
 * 		true, if given string is a replacement for lambda
 */
extern FAUDES_API bool IsLambdaReplacement(const std::string& str);


/**
 * Parse a System (TcGenerator) from given dot-file.
 * For this, a temporary file will be created which annotates empty pop and push
 * operations to each transition and a default stack bottom.
 * The temporary file will be parsed by TpdGenerator::DotRead().
 *
 * @param filename
 *		 file to parse
 *
 * @return
 * 		system from given file
 */
extern FAUDES_API System SystemFromDot(const std::string& filename);


#ifndef TEMP
#define TEMP template <class GlobalAttr, class StateAttr, class EventAttr, class TransAttr>
#endif

#ifndef THIS
#define THIS TpdGenerator<GlobalAttr, StateAttr, EventAttr, TransAttr>
#endif


//insert all stack symbols from given vector to generator
TEMP extern FAUDES_API void ParserInsStackSymbols(std::vector<std::string>& rVec,const std::string& rFileName,const int lineNr,THIS& rPd) {
	if(rVec.empty()){
		//add lambda to generator
		rPd.InsLambdaStackSymbol();

		//add lambda to vector cause empty vector not allowed in pop or push
		rVec.push_back(FAUDES_PD_LAMBDA);
	}else{
		for (std::vector<std::string>::const_iterator itstr = rVec.begin(); itstr != rVec.end(); ++itstr) {
			std::string name = *itstr;
			//check if lambda
			if(IsLambdaReplacement(name)){
				if(rVec.size() == 1){
					name = FAUDES_PD_LAMBDA;
					//add lambda to generator
					rPd.InsLambdaStackSymbol();

					//add lambda to vector
					rVec.clear();
					rVec.push_back(FAUDES_PD_LAMBDA);
				}
				#ifdef FAUDES_CHECKED
				else{
					std::stringstream errstr;
					errstr << rFileName << " L:"<< lineNr<< " pop/push lambda vector can't contain more than exactly one symbol(=lambda) ";
					throw Exception("pd_dotparser::ParserInsStackSymbols", errstr.str(), 200);
				}
				#endif

			}else{
				//add stack symbol to generator
				if(!rPd.ExistsStackSymbol(name))
					rPd.InsStackSymbol(name);
			}
		}
	}
}



//parse transition (like "x1" -> "x2" [label = ev,[pop],[push] )
TEMP extern FAUDES_API bool ParseTransition(const std::string input,const std::string& rFilename,const int linenr,THIS& rPd) {

	std::string srcState, eventName, trgState, label;
	std::vector<std::string> vPop, vPush;
	std::string::size_type end_position,pos= 0;

	//get src state
	pos = getIde(pos,input,srcState,rFilename,linenr);
	//get trg state
	pos = getIde(++pos,input,trgState,rFilename,linenr);

	//find lable of the transition
	pos = input.find("label",pos);

	#ifdef FAUDES_CHECKED
		// check if src state exists
		if(!rPd.ExistsState(srcState)) {
				std::stringstream errstr;
				errstr << rFilename << " L:"<< linenr<< " state not exists : " << srcState  << " !";
				throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
		}
		// check if trg state exists
		if(!rPd.ExistsState(trgState)) {
				std::stringstream errstr;
				errstr << rFilename << " L:"<< linenr<< " state not exists : " << trgState << " !";
				throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
		}

		//will probably never happen
		if(pos == std::string::npos) {
			std::stringstream errstr;
			errstr << rFilename << " L:"<< linenr<< " missing label for transition : " << srcState << "->" << trgState << " !";
			throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
		}
	#endif

	//get ID
	pos = getIde(pos,input,label,rFilename,linenr);

	//parse event
	end_position = std::min(label.find(','),label.find('['));

	if(end_position != std::string::npos )
	{
		//get event name
		eventName = label.substr(0,end_position);

		//check if event is lambda
		if(IsLambdaReplacement(eventName))
			eventName = FAUDES_PD_LAMBDA;

		//add event to generator
		if(! rPd.ExistsEvent(eventName))
			rPd.InsEvent(eventName);

		//get pop/push symbols
		pos = label.find("[");
		if (pos != std::string::npos)
		{
			//get pop symbols
			end_position = label.find("]",++pos);

			//check syntax
			#ifdef FAUDES_CHECKED
				std::string::size_type tmp = label.find('[',pos);
				if(tmp < end_position){
					std::stringstream errstr;
					errstr << rFilename << " L:"<< linenr<< " Expected ']' find '[' .";
					throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
				}
			#endif

			// get popped symbols
			if (end_position != std::string::npos)
			{
				std::string found_text = label.substr(pos, end_position-pos);
				vPop = split(found_text,",",rFilename,linenr);

				//add stack symbols to generator
				ParserInsStackSymbols(vPop,rFilename,linenr,rPd);

			}else{
				#ifdef FAUDES_CHECKED
					std::stringstream errstr;
					errstr << rFilename << " L:"<< linenr <<  " Missing ']'." ;
					throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
				#endif
			}

			//get push symbols
			pos = label.find('[',++pos);
			if (pos != std::string::npos)
			{
				end_position = label.find("]",++pos);
				if (end_position != std::string::npos)
				{
					std::string found_text = label.substr(pos, end_position-pos);
					vPush.push_back(FAUDES_PD_LAMBDA);
					vPush = split(found_text,",",rFilename,linenr);

					//add stack symbols to generator
					ParserInsStackSymbols(vPush,rFilename,linenr,rPd);

					//add transition to generator
					return rPd.SetTransition(srcState,eventName,trgState,vPop,vPush);
				}else{
					#ifdef FAUDES_CHECKED
						std::stringstream errstr;
						errstr << rFilename << " L:"<< linenr <<  " Missing ']'." ;
						throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
					#endif
				}
			}else{
				#ifdef FAUDES_CHECKED
					std::stringstream errstr;
					errstr << rFilename << " L:"<< linenr <<  " Missing '[' for pushed symbols." ;
					throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
				#endif
			}
		}else{
			#ifdef FAUDES_CHECKED
				std::stringstream errstr;
				errstr << rFilename << " L:"<< linenr <<  " Missing '[' for pop and pushed symbols." ;
				throw Exception("pd_dotparser::ParseTransition", errstr.str(), 200);
			#endif
		}
	}

	return false;
}

//parse events with given attributes
//The default attribute is non-controllable, non-forcible, observable and high-level.
//(see http://www.rt.techfak.fau.de/FGdes/faudes/reference/corefaudes_alphabet.html#EventSet)
TEMP extern FAUDES_API void ParseEvent(std::string input,const std::string& rFileName,const int lineNr,THIS& rPd) {
	std::string::size_type pos= 0;
	std::string eventName, attr;

	//get event name
	getIde(pos,input,eventName,rFileName,lineNr);

	//get attributes
	pos = input.find("attr",pos);
	getIde(pos,input,attr,rFileName,lineNr);

	#ifdef FAUDES_CHECKED
		if(rPd.ExistsEvent(eventName)) {
			std::stringstream errstr;
			errstr << rFileName << " L:"<< lineNr<< " event : " << eventName << " already exists !";
			throw Exception("pd_dotparser::ParseEvent", errstr.str(), 200);
		}
	#endif

	//add event to generator
	Idx event = rPd.InsEvent(eventName);

	//set attributes
	//(see http://www.rt.techfak.fau.de/FGdes/faudes/reference/corefaudes_alphabet.html#EventSet)
	for(unsigned i = 0; i < attr.size(); i++) {
		// controllability
		if(attr[i] == 'C') rPd.SetControllable(event);
		if(attr[i] == 'c') rPd.ClrControllable(event);
		// observability
		if(attr[i] == 'O') rPd.SetObservable(event);
		if(attr[i] == 'o') rPd.ClrObservable(event);
		// forcibility
		if(attr[i] == 'F') rPd.SetForcible(event);
		if(attr[i] == 'f') rPd.ClrForcible(event);
		// high-level alphabet membership
		if(attr[i] == 'A') rPd.SetHighlevel(event);
		if(attr[i] == 'a') rPd.SetLowlevel(event);
	}
}

//parse states
TEMP extern FAUDES_API void ParseState(const std::string input,const std::string& rFileName,const int lineNr,THIS& rPd) {
	std::string::size_type pos= 0;
	std::string state;

	//get state name
	getIde(pos,input,state,rFileName,lineNr);

	//add state to generator
	if(!rPd.ExistsState(state))
		rPd.InsState(state);

	//set initial state, if so
	if(input.find("->")!= std::string::npos) {
		rPd.SetInitState(state);
	}

	//set marked state, if so
	pos = input.find("shape=double",++pos);
	if(pos != std::string::npos)
		rPd.SetMarkedState(state);
}

//parse stack bottom
TEMP extern FAUDES_API void ParseStackBottom(const std::string input,const std::string& rFilename,const int linenr,THIS& rPd) {
	std::string::size_type pos= 0;
	std::string bottom;

	//get stack bottom name
	getIde(pos,input,bottom,rFilename,linenr);

	//add and set stack bottom
	rPd.InsStackSymbol(bottom);
	rPd.SetStackBottom(bottom);
}

// parsing and create a generator from given dot file
TEMP extern FAUDES_API void CreatPdFromDot(const std::string& fileName,THIS& pd) {
	// get filestream
	//std::fstream file (fileName);
	std::ifstream file (fileName.c_str(), std::ios::in | std::ios::binary);


	if (file.is_open())
	{
		//remember linenr. for error handling
		int lineNr = 0;

		// parsing line by line
		std::string line;
		while ( getline(file,line) )
		{
			++lineNr;
			std::string sub = line;

			// remove spaces
			sub.erase(std::remove_if(sub.begin(), sub.end(), ::isspace), sub.end());

			// remove comments
			sub = sub.substr(0,sub.find_first_of("//"));

			if(sub.empty())
				continue;

			if(sub.find("shape=none") != std::string::npos ||
					sub.find("shape=point") != std::string::npos )
				continue;

			// if M=(Q, Sigma, Gamma, init state , stack bottom, Qm, delta) is included in file
			// skip lines
			if(sub.find("label=<")!= std::string::npos ||
					sub.find("<br")!= std::string::npos)
							continue;

			if(sub.find("digraph")!= std::string::npos)
				sub = sub.substr(sub.find("{"));

			//parse event or stack bottom
			if(sub.find("attr=\"")!= std::string::npos) {
				//get attributes
				std::string attr = sub.substr(sub.find("attr=\""));
				//make lower case for comparing
				std::for_each(attr.begin(), attr.end(), tolower);
				// check if attribute is stack bottom
				if(attr.find("stackbottom")!= std::string::npos)
					ParseStackBottom(sub,fileName,lineNr,pd);
				else
					ParseEvent(sub,fileName,lineNr,pd);
			} else {
				// parse transition
				if(sub.find("->")!= std::string::npos &&
				   sub.find("label")!= std::string::npos) {
					ParseTransition(sub,fileName,lineNr,pd);
				} else {
					if(sub.find('"') != std::string::npos)
						ParseState(sub,fileName,lineNr,pd);
				}
			}
		}
		//close file
		file.close();

		//warn if initial state or stack bottom don't exists
		#ifdef FAUDES_CHECKED
				if(pd.InitStates().Empty()){
					std::stringstream errstr;
					errstr << "Missing initial State : " << fileName;
					FD_WARN("pd_dotparser::CreatePdFromDot : " + errstr.str());
					//throw Exception("pd_dotparser::CreatePdFromDot", errstr.str(), 200);
				}

				if(!pd.ExistsStackSymbol(pd.StackBottom())){
					std::stringstream errstr;
					errstr << "Missing stack bottom : " << fileName;
					FD_WARN("pd_dotparser::CreatePdFromDot : " + errstr.str());
					//throw Exception("pd_dotparser::CreatePdFromDot", errstr.str(), 200);
				}
		#endif
	}
	else
	{
		#ifdef FAUDES_CHECKED
			std::stringstream errstr;
			errstr << "Unable to open file : " << fileName;
			throw Exception("pd_dotparser::CreatePdFromDot", errstr.str(), 1);
		#endif
	}
}

#undef THIS
#undef TEMP
}
 //namespace

#endif
