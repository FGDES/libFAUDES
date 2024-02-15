/** @file pd_dotparser.cpp parser functions for DOT language */


/* Pushdown plugin for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2014 Ramon Barakat, Stefan Jacobi, Sven Schneider, Anne-Kathrin Hess

*/

#include "pd_dotparser.h"

namespace faudes {

// create temporary copy of file (fileName) and
// annotate dot labels with '[][]' to create transitions that pop and push lambda
std::string RewriteDotFile(const std::string& fileName) {

	std::string::size_type pos = 0;

	//create temporary file
	std::string tmpFileName = CreateTempFile();

	if (tmpFileName == "") {
		std::stringstream errstr;
		errstr << "Exception opening temp file";
		throw Exception("pd_dotparser::RewriteDotFile", errstr.str(), 2);
	}
	try {
		std::ofstream tmp;
		tmp.open(tmpFileName.c_str());

		//open file
		//std::fstream file(fileName);
		std::ifstream file (fileName.c_str(), std::ios::in | std::ios::binary);
		if (file.is_open()) {
			std::string line = "";
			int nr = 0;

			// parsing line by line
			while (getline(file, line)) {
				nr++;
				//remove spaces
				line.erase(std::remove_if(line.begin(), line.end(), ::isspace),
						line.end());

				// rewrite transition
				if (line.find("->") != std::string::npos
						&& line.find("label") != std::string::npos) {
					pos = line.find("label");
					std::string label = "";
					getIde(pos, line, label, fileName, nr);

					std::string post = "\"];";

					#ifdef FAUDES_CHECKED
						// check '[' or ']' not in ID
						if (label.find('[') != std::string::npos
								|| label.find(']') != std::string::npos) {
							file.close();
							std::stringstream errstr;
							errstr	<< "Transition containing '[' or ']' as part of label, which is not allowed ! Line "
									<< nr << " in " << fileName;
							throw Exception("pd_dotparser::RewriteDotFile",
									errstr.str(), 300);
						}

						//check if end of line exists
						if (line.find(post) == std::string::npos) {
							file.close();
							std::stringstream errstr;
							errstr	<< "Can not find end of label ( ' \"]; ' ) ! Line "
									<< nr << " in " << fileName;
							throw Exception("pd_dotparser::RewriteDotFile",
									errstr.str(), 300);
						}
					#endif

					//add annotation
					line.replace(line.find(post), post.length(),",[],[]" + post);
				}

				//write line to temporary file
				tmp << line << std::endl;
			}

			//add default stack bottom
			tmp << std::endl << "\"" << "default_stackbottom"
					<< "\" [style=\"invis\", attr=\"stackbottom\"];"
					<< std::endl;

			//close file
			file.close();

		} else {
			#ifdef FAUDES_CHECKED
				std::stringstream errstr;
				errstr << "Unable to open file : " << fileName;
				throw Exception("pd_dotparser::RewriteDotFile", errstr.str(), 1);
			#endif
		}

		//close temporary file
		tmp.close();
	} catch (faudes::Exception& exception) {
		//remove temporary file
		RemoveFile(tmpFileName);
		std::stringstream errstr;
		errstr << "Exception writing dot input file";
		throw Exception("pd_dotparser::RewriteDotFile", errstr.str(), 2);
	}

	return tmpFileName;
}

// parse System from *.dot
System SystemFromDot(const std::string& filename) {
	FD_DF("ParseSystemFromDot(...): " << filename);

	//resulting system
	System res;

	// generate temporary dot input file
	std::string dotin = RewriteDotFile(filename);
	if (dotin == "") {
		std::stringstream errstr;
		errstr << "Exception opening temporary file";
		throw Exception("pd_dotparser::ParseSystemFromDot", errstr.str(), 2);
	}
	try {
		//create generator
		PushdownGenerator pd;

		//parse file
		pd.DotRead(dotin);

		//get System
		res = System(pd);
	} catch (faudes::Exception& exception) {
		RemoveFile(dotin);
		std::stringstream errstr;
		errstr << "Exception writing dot input file";
		throw Exception("pd_dotparser::ParseSystemFromDot", errstr.str(), 2);
	}

	//remove file
	RemoveFile(dotin);

	return res;
}

//check if lanmda replacement
bool IsLambdaReplacement(const std::string& s) {
	return ((s == "_") || (s == "&lambda;") || (s == "lambda"));
}

// split string by separator
std::vector<std::string> split(const std::string& rStr, const std::string& rSep,
		const std::string& rFilename, const int lineNr) {

	//resulting tokens
	std::vector<std::string> vTokens;

	if (!rStr.empty()) {
		std::string::size_type start = 0, end = 0, curr = 0;

		std::string str = rStr;
		std::string sub;

		//if no separator in string
		if ((end = str.find(rSep, start)) == std::string::npos) {
			//push whole string to result
			vTokens.push_back(str);
			return vTokens;
		}

		curr = start;

		//while separator from current position exists
		while ((end = str.find(rSep, curr)) != std::string::npos) {
			//substring from current position to separator
			sub = str.substr(curr, end - curr);

			//ensure that tuple (x,y) will seen as one ID
			if (sub.find("(") != std::string::npos) {
				end = str.find(")", curr);
				#ifdef FAUDES_CHECKED
					if (end == std::string::npos) {
						std::stringstream errstr;
						errstr << "Missing ')' in " << rFilename << " L:" << lineNr
								<< " " << rStr;
						throw Exception("pd_dotparser::split(inp,sep)", errstr.str(),
								200);
					}
				#endif
				end++;

				//get whole tuple
				sub = str.substr(curr, end - curr);
			}

			//push ID to token
			vTokens.push_back(sub);
			//continue after separator
			curr = end + 1;
		}

		//add last token
		if (curr < str.size())
			vTokens.push_back(str.substr(curr));
	}

	return vTokens;
}

// set string literal between "..." to &resText and return the position after the second quotation.
std::string::size_type getIde(std::string::size_type& rPos,
		const std::string& rInput, std::string& rRestString,
		const std::string& rFilename, const int lineNr) {
	std::string::size_type end_position = rPos;

	rPos = rInput.find('"', rPos);
	if (rPos != std::string::npos) {
		//get end position;
		end_position = rInput.find('"', ++rPos);
		if (end_position != std::string::npos) {
			//get string literal
			rRestString = rInput.substr(rPos, end_position - rPos);
			return end_position + 1;
		} else {
			#ifdef FAUDES_CHECKED
				std::stringstream errstr;
				errstr << "Missing \" in " << rFilename << " L:" << lineNr << " "
						<< rInput << std::endl;
				throw Exception("pd_dotparser::getIde for" + rInput.substr(rPos),
						errstr.str(), 200);
			#endif
		}
	}

	return rInput.size();
}

}
