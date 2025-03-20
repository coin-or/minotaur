//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file Reader.cpp
 * \brief Define Reader class for reading MPS files
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <sstream>
#include <fstream>
#include <cmath>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Function.h"
#include "LinearFunction.h"
#include "Logger.h"
#include "Operations.h"
#include "Problem.h"
#include "Reader.h"

using namespace Minotaur;
const std::string Reader::me_ = "Reader: ";

Reader::Reader(EnvPtr env)
  : env_(env)
{
  logger_ = env->getLogger();
}


Reader::~Reader()
{
}


ProblemPtr Reader::readMps(std::string fname, int &err)
{
  std::ifstream fs;
  std::string line, word, word2, word3, word4;
  std::string rhsid = "";
  std::string rangeid = "";
  std::string bndid = "";  // second word of all lines in BOUNDS section must
                           // be common across the file.
  std::string qcname;  //name of the row for QCMatrix
  std::string oldcol = "";
  ProblemPtr p = 0;
  std::istringstream iss;
  int lcnt, m, r;
  VariableType vtype = Continuous;
  std::vector<LinearFunctionPtr> lfs;
  std::vector<QuadraticFunctionPtr> qfs;
  std::vector<char> rowtypes;
  std::vector<std::string> rownamesvec;
  std::vector<double> rowrhs, rowranges;
  std::string objname;
  std::map<std::string, int> rownames;
  std::map<std::string, int>::iterator rnit;
  std::map<std::string, VariablePtr> colnames;
  std::map<std::string, VariablePtr>::iterator cnit;
  VariablePtr v = NULL, v2 = NULL;
  FunctionPtr f;
  double dval, lb, ub;
  QuadraticFunction *qfo = NULL;
  QuadraticFunction *qfc = 0;  // quadratic function being populated in
                               // QCMATRIX section
  MpsSec section = MpsNone;
  ObjectiveType ot = Minimize;
  double tstrt = env_->getTime();

  // std::ios_base::sync_with_stdio(false); 
  // std::cin.tie(NULL);


  err = 0;
  fs.open(fname.c_str());
  if (!fs.is_open()) {
    logger_->errStream() << me_ << "could not open file " << fname
                         << " for reading" << std::endl;
    err = 1;
    return 0;
  }


  logger_->msgStream(LogInfo)
      << me_ << "reading MPS file " << fname << std::endl;

  p = new Problem(env_);

  vtype = Continuous;
  m = 0;
  lcnt = 0;
  while (0 == err && MpsEnd != section && std::getline(fs, line)) {
    iss.clear();    // deletes only flags internal to iss
    iss.str(line);  // convert line into ifstream
    ++lcnt;


    if (!(iss >> word)) {
      continue;  // empty line
    }

    if ('*' == word[0]) {
      continue;  // ignore line, because it is a comment
    }

    // check if a new section starts
    if (line[0] == word[0]) {  // we have no white space in the beginning
      if (word == "NAME") {
        section = MpsNone;  // back to new unknown section. This should not be
                            // set to MpsName
        continue;
      } else if (word == "OBJSENSE") {
        if (!(iss >> word2)) {
          section = MpsSense; // sense is not written the current line, check
                              // the next line
        } else {
          toLowerCase(word2);
          if (word2 == "max") {
            ot = Maximize;
          } else if (word2 == "min") {
            ot = Minimize;
          } else {
            logger_->msgStream(LogError)
              << me_ << "warning: OBJSENSE " << word2 << " ignored on line "
              << lcnt << std::endl;
          }
        }
        continue;
      } else if (word == "ROWS") {
        section = MpsRows;
        logger_->msgStream(LogDebug) << me_ << "Reading ROWS "
          << env_->getTime() << std::endl;
        continue;
      } else if (word == "COLUMNS") {
        section = MpsCols;
        logger_->msgStream(LogDebug) << me_ << "Reading COLUMNS "
          << env_->getTime() << std::endl;
        continue;
      } else if (word == "RHS") {
        section = MpsRhs;
        logger_->msgStream(LogDebug) << me_ << "Reading RHS "
          << env_->getTime() << std::endl;
        continue;
      } else if (word == "RANGES") {
        section = MpsRang;
        logger_->msgStream(LogDebug) << me_ << "Reading RANGES "
          << env_->getTime() << std::endl;
        continue;
      } else if (word == "BOUNDS") {
        section = MpsBoun;
        logger_->msgStream(LogDebug) << me_ << "Reading BOUNDS "
          << env_->getTime() << std::endl;
        continue;
      } else if (word == "QUADOBJ") {
        section = MpsQO;
        logger_->msgStream(LogDebug) << me_ << "Reading QUADOBJ "
          << env_->getTime() << std::endl;
        continue;
      } else if (word == "QCMATRIX") {
        section = MpsQC;
        // read the name of the quad constraint now
        if (!(iss >> word2)) {
          logger_->errStream() << me_ << "ERROR: Missing row name in line "
                               << lcnt << std::endl;
          err = 10;
        } else if (rownames.find(word2) == rownames.end()) {
          rownames[word2] = m;
          rownamesvec.push_back(word2);
          lfs.push_back(new LinearFunction());
          qfc = new QuadraticFunction();
          qfs.push_back(qfc);
          ++m;
        } else {
          r = rownames[word2];
          qfc = qfs[r];
        }
        qcname = word2;
        continue;
      } else if (word == "ENDATA") {
        section = MpsEnd;
        logger_->msgStream(LogDebug) << me_ << "Reached the end "
          << env_->getTime() << std::endl;
        continue;
      }
    }

    // we are in an existing section
    switch (section) {
    case (MpsNone):
      logger_->errStream() << me_ << "error parsing the MPS file in line "
        << lcnt << std::endl
        << line << std::endl;
      err = 10;
      break;
    case (MpsSense):
      toLowerCase(word);
      if ( word == "max") {
        ot = Maximize;
      } else if (word == "min") {
        ot = Minimize;
      } else {
        logger_->errStream() << me_ << "Unexpected word " << word
                             << " in line " << lcnt 
                             << " in section OBJSENSE" << std::endl;
        err = 10;
      }
      break;
    case (MpsName):  // NAME can be ignored for now
      break;
    case (MpsRows):  // ROWS
      if (word[0] == 'N' || word[0] == 'G' || word[0] == 'L' ||
          word[0] == 'E') {
        rowtypes.push_back(word[0]);
        rowrhs.push_back(INFINITY);
        rowranges.push_back(INFINITY);

        if (!(iss >> word)) {  // read the next word
          logger_->errStream() << me_ << "ERROR: Missing row name in line "
                               << lcnt << std::endl;
          err = 10;
        } else if (rownames.find(word) == rownames.end()) {
          rownames[word] = m;
          rownamesvec.push_back(word);
          lfs.push_back(new LinearFunction());
          qfs.push_back(new QuadraticFunction());
          if ('N' == rowtypes[m] && !qfo) {
            qfo = qfs[m];
          }
          ++m;
        } else {
          logger_->errStream()
              << me_ << "ERROR: Row " << word
              << " seen more than once in the ROWS section of MPS file "
              << std::endl;
          err = 10;
        }

        if (0 == err && (iss >> word)) {  // check for eol
          logger_->errStream()
              << me_ << "ERROR: line " << lcnt << " should have ended before "
              << word << std::endl;
          err = 10;
        }
      } else {
        logger_->errStream() << me_ << "Unexpected word " << word
                             << " in line " << lcnt << std::endl;
        err = 10;
      }
      break;
    case (MpsCols):  // COLUMNS
      // Read the next two words. Every line must have 3 or 5 words
      if (!(iss >> word2) || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in column "
                             << "line " << lcnt << std::endl;
        err = 10;
        break;
      }

      if (word2 == "'MARKER'") {
        if (word3 == "'INTORG'") {
          if (Integer == vtype) {
            logger_->errStream()
                << me_ << "ERROR: 'INTORG' seen within "
                << "'INTORG' section, line " << lcnt << std::endl;
            err = 10;
          } else {
            vtype = Integer;
          }
        } else if (word3 == "'INTEND'") {
          if (Continuous == vtype) {
            logger_->errStream()
                << me_ << "ERROR: 'INTEND' seen outside "
                << "'INTORG' section, line " << lcnt << std::endl;
            err = 10;
          } else {
            vtype = Continuous;
          }
        } else {
          logger_->errStream() << me_ << "ERROR: Unknown marker " << word3
                               << " in line " << lcnt << std::endl;
          err = 10;
        }
      } else {
        rnit = rownames.find(word2);
        if (rnit == rownames.end()) {
          logger_->errStream()
            << me_ << "ERROR: rowname " << word2 << " in line " << lcnt
            << " undeclared " << std::endl;
          break;
        } else {
          if (word == oldcol) {
            // v stays the same, no need to lookup
          } else {
            cnit =  colnames.find(word);
            if (cnit == colnames.end()) {
              v = p->newVariable(0, INFINITY, vtype, word);
              colnames[word] = v;
              oldcol = word;
            } else {
              v = cnit->second;
              oldcol = word;
            }
          }
          dval = std::stod(word3);
          r = rnit->second; 
          lfs[r]->addTerm(v, dval);

          // we may have two more terms (but not one)
          if (iss >> word2) {
            if (!(iss >> word3)) {
              logger_->errStream()
                << me_ << "ERROR: not enough fields in column "
                << "line " << lcnt << std::endl;
              err = 10;
              break;
            } else {
              rnit = rownames.find(word2);
              if (rnit == rownames.end()) {
                logger_->errStream()
                  << me_ << "ERROR: rowname " << word2 << " in line " << lcnt
                  << " undeclared " << std::endl;
                err = 10;
                break;
              }
              dval = std::stod(word3);
              r = rownames[word2];
              lfs[r]->addTerm(v, dval);
            }
          }
        }
      }
      break;
    case (MpsRhs):  // RHS
      if (rhsid == "") {
        rhsid = word;
      } else if (word != rhsid) {
        logger_->msgStream(LogError)
            << me_ << rhsid << " ignored in line " << lcnt << std::endl;
        break;
      }
      if (!(iss >> word2) || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in column "
                             << "line " << lcnt << std::endl;
        err = 10;
        break;
      } else if (rownames.find(word2) == rownames.end()) {
        logger_->errStream()
            << me_ << "ERROR: rowname " << word2 << " in line " << lcnt
            << " undeclared " << std::endl;
        break;
      } else {
        dval = std::stod(word3);
        r = rownames[word2];
        if (rowrhs[r] != INFINITY) {  // if previously set, warn
          logger_->msgStream(LogExtraInfo)
              << me_ << "Warning: overwriting rhs for row " << word2
              << " in line " << lcnt << std::endl;
        }
        rowrhs[r] = dval;

        // we may have two more terms (but not one)
        if (iss >> word2) {
          if (!(iss >> word3)) {
            logger_->errStream() << me_ << "ERROR: not enough fields in rhs "
                                 << "line " << lcnt << std::endl;
            err = 10;
            break;
          } else if (rownames.find(word2) == rownames.end()) {
            logger_->errStream()
                << me_ << "ERROR: rowname " << word2 << " in line " << lcnt
                << " undeclared " << std::endl;
            err = 10;
            break;
          }
          dval = std::stod(word3);
          r = rownames[word2];
          if (rowrhs[r] != INFINITY) {  // if previously set, warn
            logger_->msgStream(LogExtraInfo)
                << me_ << "Warning: overwriting rhs for row " << word2
                << " in line " << lcnt << std::endl;
          }
          rowrhs[r] = dval;
        }
      }
      break;
    case (MpsRang):  // RANGES
      if (rangeid == "") {
        rangeid = word;
      } else if (word != rangeid) {
        logger_->msgStream(LogError)
            << me_ << rangeid << " ignored in line " << lcnt << std::endl;
        break;
      }
      if (!(iss >> word2) || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in ranges "
                             << "line " << lcnt << std::endl;
        err = 10;
        break;
      } else if (rownames.find(word2) == rownames.end()) {
        logger_->errStream()
            << me_ << "ERROR: rowname " << word2 << " in line " << lcnt
            << " undeclared " << std::endl;
        break;
      } else {
        dval = std::stod(word3);
        r = rownames[word2];
        if (rowranges[r] != INFINITY) {  // warn
          logger_->msgStream(LogExtraInfo)
              << me_ << "Warning: overwriting range for row " << word2
              << " in line " << lcnt << std::endl;
        }
        rowranges[r] = dval;
      }

      // we may have two more terms (but not one)
      if (iss >> word2) {
        if (!(iss >> word3)) {
          logger_->errStream() << me_ << "ERROR: not enough fields in ranges "
                               << "line " << lcnt << std::endl;
          err = 10;
          break;
        } else if (rownames.find(word2) == rownames.end()) {
          logger_->errStream()
              << me_ << "ERROR: rowname " << word2 << " in line " << lcnt
              << " undeclared " << std::endl;
          err = 10;
          break;
        }
        dval = std::stod(word3);
        r = rownames[word2];
        if (rowranges[r] != INFINITY) {  // warn
          logger_->msgStream(LogExtraInfo)
              << me_ << "Warning: overwriting range for row " << word2
              << " in line " << lcnt << std::endl;
        }
        rowranges[r] = dval;
      }
      break;
    case (MpsBoun):  // BOUNDS
      // If a bound on a variable is seen more than once, the later bound
      // value overrides the earlier ones
      // e.g. if we have
      // UP BND x1 40
      // UP BND x1 50
      // then the ub of x1 is set to 50.
      if (!(iss >> word2) || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in BOUNDS "
                             << "line " << lcnt << std::endl;
        err = 10;
        break;
      } else if (bndid == "") {
        bndid = word2;
      } else if (word2 != bndid) {
        logger_->msgStream(LogError)
            << me_ << "Warning: " << bndid << " ignored in line " << lcnt
            << std::endl;
        break;
      } 
      
      cnit =  colnames.find(word3);
      if (cnit == colnames.end()) {
        logger_->errStream()
            << me_ << "ERROR: column name " << word2 << " in line " << lcnt
            << " undeclared " << std::endl;
        err = 10;
        break;
      }

      dval = INFINITY;
      if (iss >> word4) {
        dval = std::stod(word4);
      } else if (word3 == "LO" || word3 == "UP" || word3 == "FX") {
        logger_->msgStream(LogError)
            << me_ << "ERROR: " << word3 << " key requires a number in line "
            << lcnt << std::endl;
        err = 10;
        break;
      }
      v = cnit->second;

      if (word == "LO") {
        p->changeBound(v, Lower, dval);
      } else if (word == "UP") {
        if (dval < 0.0) {
          if (v->getLb() == 0.0) {
            p->changeBound(v, -INFINITY, dval);
          } else {
            p->changeBound(v, Upper, dval);
          }
        } else {
          p->changeBound(v, Upper, dval);
        }
      } else if (word == "FX") {
        p->changeBound(v, dval, dval);
      } else if (word == "FR") {
        p->changeBound(v, -INFINITY, INFINITY);
      } else if (word == "MI") {
        p->changeBound(v, Lower, -INFINITY);
      } else if (word == "PL") {
        p->changeBound(v, Upper, INFINITY);
      } else if (word == "BV") {
        p->setVarType(v, Binary);
      } else if (word == "LI") {
        p->setVarType(v, Integer);
        p->changeBound(v, Lower, dval);
      } else if (word == "UI") {
        p->setVarType(v, Integer);
        p->changeBound(v, Upper, dval);
      } else {
        logger_->errStream() << me_ << "ERROR: unknown bound type " << word
                             << " in line " << lcnt << std::endl;
        err = 10;
        break;
      }
      break;
    case (MpsQO):  // Quadratic Objective
      if (!(iss >> word2) || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in QUADOBJ "
                             << "line " << lcnt << std::endl;
        err = 10;
        break;
      }
      v = colnames[word];
      v2 = colnames[word2];
      dval = std::stod(word3);
      qfo->incTerm(v, v2, dval * 0.5);
      break;
    case (MpsQC):  // Quadratic Constraint
      // We should have three words in this row (var1 var2 coeff)
      if (!(iss >> word2) || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in QCMATRIX "
                             << "line " << lcnt << std::endl;
        err = 10;
        break;
      } else {
        v = colnames[word];
        v2 = colnames[word2];
        dval = std::stod(word3);
        qfc->incTerm(v, v2, dval);
      }
      break;
    case (MpsEnd):
      break;  // out of the mps file
    default:
      logger_->errStream() << me_
                           << "Unknown Section in MPS file near for line "
                           << lcnt << std::endl;
      err = 10;
      break;
    }
  }
  fs.close();
  env_->getLogger()->msgStream(LogDebug) << me_ 
    << "File closed " <<  env_->getTime() << std::endl;

  // put all cons and obj in p
  for (int i = 0; i < m; ++i) {

    if (qfs[i]->getNumTerms() > 0) {
      f = new Function(lfs[i], qfs[i]);
    } else {
      f = new Function(lfs[i]);
      delete qfs[i];
      qfs[i] = NULL;
    }
    switch (rowtypes[i]) {
    case ('G'):
      lb = (rowrhs[i] == INFINITY) ? 0.0 : rowrhs[i];
      if (rowranges[i] < INFINITY) {
        ub = lb + fabs(rowranges[i]);
      } else {
        ub = INFINITY;
      }
      break;
    case ('L'):
      ub = (rowrhs[i] == INFINITY) ? 0.0 : rowrhs[i];
      if (rowranges[i] < INFINITY) {
        lb = ub - fabs(rowranges[i]);
      } else {
        lb = -INFINITY;
      }
      break;
    case ('E'):
      if (rowranges[i] == INFINITY) {
        ub = (rowrhs[i] == INFINITY) ? 0.0 : rowrhs[i];
        lb = ub;
      } else if (rowranges[i] > 0.0) {
        lb = (rowrhs[i] == INFINITY) ? 0.0 : rowrhs[i];
        ub = lb + rowranges[i];
      } else {
        ub = (rowrhs[i] == INFINITY) ? 0.0 : rowrhs[i];
        lb = ub - rowranges[i];
      }
      break;
    default:
      lb = -INFINITY;
      ub = INFINITY;
      break;
    }
    if (rowtypes[i] != 'N') {
      p->newConstraint(f, lb, ub, rownamesvec[i]);
    } else if (p->getObjective()) {
      logger_->msgStream(LogError) << me_ << "Warning: ignored objective "
                                   << rownamesvec[i] << std::endl;
      delete f;
      f = 0;
    } else {
      lb = (rowrhs[i] == INFINITY) ? 0.0 : -rowrhs[i];
      p->newObjective(f, lb, ot, rownamesvec[i]);
    }
  }

  env_->getLogger()->msgStream(LogInfo) << me_ 
    << "time used in reading file = " << std::fixed 
    << std::setprecision(2) << env_->getTime()-tstrt << " seconds." 
    << std::endl;

  //p->write(std::cout);
  p->calculateSize();
  return p;
}


int Reader::readSol(ProblemPtr p, std::string sname)
{
  int err = 0;
  int lcnt = 0, vcnt = 0;
  std::ifstream fs;
  std::istringstream iss;
  std::string line, word, word2;
  std::string rhsid = "";
  DoubleVector x(p->getNumVars(), 0.0);
  std::vector<std::string> names;
  std::string::size_type echars;  // size of string
  bool found = false;
  double dval;

  fs.open(sname.c_str());
  if (!fs.is_open()) {
    logger_->errStream() << me_ << "could not open file " << sname
                         << " for reading" << std::endl;
    return 1;
  }

  logger_->msgStream(LogInfo)
      << me_ << "reading solution file " << sname << std::endl;

  while (0 == err && std::getline(fs, line)) {
    iss.clear();    // deletes only flags internal to iss
    iss.str(line);  // convert line into ifstream
    ++lcnt;

    // std::cout << line <<  std::endl;

    if (!(iss >> word)) {
      continue;  // empty line
    }

    if ('#' == word[0]) {
      continue;  // ignore line, because it is a comment
    }

    if (!(iss >> word2)) {
      logger_->errStream() << me_ << "ERROR: not enough fields in line "
                           << lcnt << " of " << sname << std::endl;
      err = 1;
    } else {
      // word = name-of-variable, and word2 = value
      dval = std::stod(word2, &echars);
      found = false;
      vcnt = 0;
      for (VariableConstIterator it = p->varsBegin(); it != p->varsEnd();
           ++it) {
        if ((*it)->getName() == word) {
          found = true;
          x[vcnt] = dval;
          break;
        }
        ++vcnt;
      }
      if (false == found) {
        logger_->msgStream(LogError)
            << "Variable " << word << " not found in the solution file "
            << sname << std::endl;
        err = 1;
      }
    }
  }

  if (0 == err) {
    p->setDebugSol(x);
  }

  return err;
}
