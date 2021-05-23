//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2020 The MINOTAUR Team.
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
  std::string rhsid="";
  std::string rangeid="";
  std::string bndid="";
  ProblemPtr p = 0;
  std::istringstream iss;
  int lcnt, m, n, r;
  bool comment;
  VariableType vtype = Continuous;
  std::vector<LinearFunctionPtr> lfs;
  std::vector<char> rowtypes;
  std::vector<std::string> rownamesvec;
  std::vector<double> rowrhs, rowranges;
  std::string objname;
  std::map<std::string,int> rownames;
  std::map<std::string,int> colnames;
  VariablePtr v;
  FunctionPtr f;
  double dval, lb, ub;
  std::string::size_type echars; // to check errors is string to double

  int section = 0; // 0: None, 1: NAME, 2: ROWS, 3: COLUMNS, 4: RHS,
                   // 5: RANGES, 6: BOUNDS, 7: ENDDATA

  err = 0;
  fs.open(fname.c_str());
  if (!fs.is_open()) {
    logger_->errStream() << me_ << "could not open file " << fname
      << " for reading" << std::endl;
    err = 1;
    return 0;
  } 

  logger_->msgStream(LogInfo) << me_ << "reading MPS file " << fname 
    << std::endl;

  p = new Problem(env_);

  vtype = Continuous;
  m = n = 0;
  lcnt = 0;
  while (0==err && 7!=section && std::getline(fs, line)) {
    iss.clear();   // deletes only flags internal to iss
    iss.str(line); // convert line into ifstream
    ++lcnt;

    // std::cout << line <<  std::endl;

    if (!(iss >> word)) {
      continue; // empty line
    }

    comment = false;
    if ('*'==word[0]) {
      comment = true;
      continue; // ignore line, because it is a comment
    }

    // check if a new section starts
    if (line[0] == word[0]) { // we have no white space in the beginning
      if (word=="NAME") {
        section = 1;
        continue;
      } else if (word=="ROWS") {
        section = 2;
        continue;
      } else if (word=="COLUMNS") {
        section = 3;
        continue;
      } else if (word=="RHS") {
        section = 4;
        continue;
      } else if (word=="RANGES") {
        section = 5;
        continue;
      } else if (word=="BOUNDS") {
        section = 6;
        continue;
      } else if (word=="ENDATA") {
        section = 7;
        continue;
      }
    }

    // we are in an existing section
    switch (section) {
    case(0):
      if (!comment) {
        logger_->errStream() << me_ << "error while parsing the MPS file in line " << lcnt
          << std::endl << line  <<  std::endl;
        err = 10;
      }
      break;
    case(1): // NAME can be ignored for now
      break;
    case(2): // ROWS
      if (word[0] == 'N' || word[0] == 'G' || word[0] == 'L'
          || word[0] == 'E' ) {
        rowtypes.push_back(word[0]);
        rowrhs.push_back(INFINITY);
        rowranges.push_back(INFINITY);

        if (!(iss>>word)) { // read the next word
          logger_->errStream() << me_ << "ERROR: Missing row name in line " << lcnt
            << std::endl;
          err = 10;
        } else if (rownames.find(word)==rownames.end()) {
          rownames[word] = m;
          rownamesvec.push_back(word);
          lfs.push_back(new LinearFunction());
          ++m;
        } else {
          logger_->errStream() << me_ << "ERROR: Row " << word 
            << " seen more than once in the ROWS section of MPS file "
            << std::endl;
          err = 10;
        }

        if (0==err && (iss>>word)) { // check for eol
          logger_->errStream() << me_ << "ERROR: line " << lcnt 
            << " should have ended before " << word << std::endl;
          err = 10;
        }
      } else {
        logger_->errStream() << me_ << "Unexpected word " << word
          << " in line " << lcnt << std::endl;
        err = 10;
      }
      break;
    case(3): // COLUMNS
      // Read the next two words. Every line must have 3 or 5 words
      if (!(iss >> word2)  || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in column "
          << "line " << lcnt  << std::endl;
        err = 10;
        break;
      } 

      if (word2 == "'MARKER'") {
        if (word3 == "'INTORG'") {
          if (Integer==vtype) {
            logger_->errStream() << me_ << "ERROR: 'INTORG' seen within " 
              << "'INTORG' section, line " << lcnt << std::endl;
            err = 10;
          } else {
            vtype = Integer;
          }
        } else if (word3 == "'INTEND'") {
          if (Continuous==vtype) {
            logger_->errStream() << me_ << "ERROR: 'INTEND' seen outside " 
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
      } else if (rownames.find(word2)==rownames.end()) {
          logger_->errStream() << me_ << "ERROR: rowname " << word2
            << " in line " << lcnt << " undeclared " << std::endl;
          break;
      } else {
        if (colnames.find(word)==colnames.end()) {
          v = p->newVariable(0, INFINITY, vtype, word);
          colnames[word] = n;
          ++n;
        } else {
          v = p->getVariable(colnames[word]); 
        }
        dval = std::stod(word3, &echars);
        r = rownames[word2];
        lfs[r]->incTerm(v,dval);

        // we may have two more terms (but not one)
        if (iss >> word2) {
          if (!(iss >> word3)) {
            logger_->errStream() << me_ << "ERROR: not enough fields in column "
              << "line " << lcnt << std::endl;
            err = 10;
            break;
          } else if (rownames.find(word2)==rownames.end()) {
            logger_->errStream() << me_ << "ERROR: rowname " << word2
              << " in line " << lcnt << " undeclared " << std::endl;
            err = 10;
            break;
          }
          dval = std::stod(word3, &echars);
          r = rownames[word2];
          lfs[r]->incTerm(v,dval);
        }
      }
      break;
    case(4): // RHS
      if (rhsid=="") {
        rhsid = word;
      } else if (word != rhsid) {
        logger_->msgStream(LogError) << me_ << rhsid << " ignored in line " 
          << lcnt << std::endl;
        break;
      }
      if (!(iss >> word2)  || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in column "
          << "line " << lcnt << std::endl;
        err = 10;
        break;
      } else if (rownames.find(word2)==rownames.end()) {
        logger_->errStream() << me_ << "ERROR: rowname " << word2
          << " in line " << lcnt << " undeclared " << std::endl;
        break;
      } else {
        dval = std::stod(word3, &echars);
        r = rownames[word2];
        if (rowrhs[r] != INFINITY) { // if previously set, warn
          logger_->msgStream(LogExtraInfo) << me_ 
            << "Warning: overwriting rhs for row " << word2 << " in line " 
            << lcnt << std::endl;
        }
        rowrhs[r] = dval;

        // we may have two more terms (but not one)
        if (iss >> word2) {
          if (!(iss >> word3)) {
            logger_->errStream() << me_ << "ERROR: not enough fields in rhs "
              << "line " << lcnt << std::endl;
            err = 10;
            break;
          } else if (rownames.find(word2)==rownames.end()) {
            logger_->errStream() << me_ << "ERROR: rowname " << word2
              << " in line " << lcnt << " undeclared " << std::endl;
            err = 10;
            break;
          }
          dval = std::stod(word3, &echars);
          r = rownames[word2];
          if (rowrhs[r] != INFINITY) { // if previously set, warn
            logger_->msgStream(LogExtraInfo) << me_ 
              << "Warning: overwriting rhs for row " << word2 << " in line " 
              << lcnt << std::endl;
          }
          rowrhs[r] = dval;
        }
      }
      break;
    case(5): // RANGES
      if (rangeid=="") {
        rangeid = word;
      } else if (word != rangeid) {
        logger_->msgStream(LogError) << me_ << rangeid << " ignored in line " 
          << lcnt << std::endl;
        break;
      }
      if (!(iss >> word2)  || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in ranges "
          << "line " << lcnt << std::endl;
        err = 10;
        break;
      } else if (rownames.find(word2)==rownames.end()) {
        logger_->errStream() << me_ << "ERROR: rowname " << word2
          << " in line " << lcnt << " undeclared " << std::endl;
        break;
      } else {
        dval = std::stod(word3, &echars);
        r = rownames[word2];
        if (rowranges[r] != INFINITY) { // warn
          logger_->msgStream(LogExtraInfo) << me_ 
            << "Warning: overwriting range for row " << word2 << " in line " 
            << lcnt << std::endl;
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
        } else if (rownames.find(word2)==rownames.end()) {
          logger_->errStream() << me_ << "ERROR: rowname " << word2
            << " in line " << lcnt << " undeclared " << std::endl;
          err = 10;
          break;
        }
        dval = std::stod(word3, &echars);
        r = rownames[word2];
        if (rowranges[r] != INFINITY) { // warn
          logger_->msgStream(LogExtraInfo) << me_ 
            << "Warning: overwriting range for row " << word2 << " in line " 
            << lcnt << std::endl;
        }
        rowranges[r] = dval;
      }
      break;
    case(6): // BOUNDS
      // If a bound on a variable is seen more than once, the later bound
      // value overrides the earlier ones
      // e.g. if we have
      // UP BND x1 40
      // UP BND x1 50
      // then the ub of x1 is set to 50.
      if (!(iss >> word2)  || !(iss >> word3)) {
        logger_->errStream() << me_ << "ERROR: not enough fields in BOUNDS "
          << "line " << lcnt << std::endl;
        err = 10;
        break;
      } else if (bndid=="") {
        bndid = word2;
      } else if (word2 != bndid) {
        logger_->msgStream(LogError) << me_ << "Warning: " << bndid 
          << " ignored in line " << lcnt << std::endl;
        break;
      } else if (colnames.find(word3)==colnames.end()) {
        logger_->errStream() << me_ << "ERROR: column name " << word2
          << " in line " << lcnt << " undeclared " << std::endl;
        err = 10;
        break;
      }

      dval = INFINITY;
      if (iss >> word4) {
        dval = std::stod(word4, &echars);
      } else if (word3=="LO" || word3=="UP" || word3=="FX") {
        logger_->msgStream(LogError) << me_ << "ERROR: " << word3 
          << " key requires a number in line " << lcnt << std::endl;
        err = 10;
        break;
      }
      v = p->getVariable(colnames[word3]);
      if (word=="LO") {
        p->changeBound(v, Lower, dval);
      } else if (word=="UP") {
        if (dval < 0.0) {
          if (v->getLb() == 0.0) {
            p->changeBound(v, -INFINITY, dval);
          } else {
            p->changeBound(v, Upper, dval);
          }
        } else {
          p->changeBound(v, Upper, dval);
        }
      } else if (word=="FX") {
        p->changeBound(v, dval, dval);
      } else if (word=="FR") {
        p->changeBound(v, -INFINITY, INFINITY);
      } else if (word=="MI") {
        p->changeBound(v, Lower, -INFINITY);
      } else if (word=="PL") {
        p->changeBound(v, Upper, INFINITY);
      } else if (word=="BV") {
        p->setVarType(v, Binary);
      } else if (word=="LI") {
        p->setVarType(v, Integer);
        p->changeBound(v, Lower, dval);
      } else if (word=="UI") {
        p->setVarType(v, Integer);
        p->changeBound(v, Upper, dval);
      } else {
        logger_->errStream() << me_ << "ERROR: unknown bound type " << word
          << " in line " << lcnt << std::endl;
        err = 10;
        break;
      }
      break;
    case(7):
      break; // out of the mps file
    default:
      break;
    }
  }
  fs.close();

  // put all cons in p
  for (int i=0; i<m; ++i) {
    f = new Function(lfs[i]);
    switch (rowtypes[i]) {
    case ('G'):
      lb = (rowrhs[i]==INFINITY)?0.0:rowrhs[i];
      if (rowranges[i] < INFINITY) {
        ub = lb + fabs(rowranges[i]);
      } else {
        ub = INFINITY;
      }
      break;
    case ('L'):
      ub = (rowrhs[i]==INFINITY)?0.0:rowrhs[i];
      if (rowranges[i] < INFINITY) {
        lb = ub - fabs(rowranges[i]);
      } else {
        lb = -INFINITY;
      }
      break;
    case ('E'):
      if (rowranges[i]==INFINITY) {
        ub = (rowrhs[i]==INFINITY)?0.0:rowrhs[i];
        lb = ub;
      } else if (rowranges[i]>0.0) {
        lb = (rowrhs[i]==INFINITY)?0.0:rowrhs[i];
        ub = lb + rowranges[i];
      } else {
        ub = (rowrhs[i]==INFINITY)?0.0:rowrhs[i];
        lb = ub - rowranges[i];
      }
      break;
    default:
      break;
    }
    if (rowtypes[i] != 'N') {
      p->newConstraint(f,lb,ub,rownamesvec[i]);
    } else if (p->getObjective()) {
      logger_->msgStream(LogError) << me_ << "Warning: ignored objective " 
        << rownamesvec[i] << std::endl;
      delete f; f = 0;
    } else {
      lb = (rowrhs[i] == INFINITY)?0.0:-rowrhs[i];
      p->newObjective(f, lb, Minimize, rownamesvec[i]);
    }
  }

  //p->write(std::cout);
  p->calculateSize();
  return p;
}

