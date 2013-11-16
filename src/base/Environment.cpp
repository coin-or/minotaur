//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2013 The MINOTAUR Team.
//

/**
 * \file Environment.cpp
 * \brief Implement the Environment class
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Logger.h"
#include "Option.h"
#include "Timer.h"
#include "Version.h"

using namespace Minotaur;

const std::string Environment::me_ = "Environment: ";

Environment::Environment()
{
  logger_     = (LoggerPtr) new Logger();
  options_    = (OptionDBPtr) new OptionDB();
  timerFac_   = new TimerFactory();
  timer_      = timerFac_->getTimer();
  createDefaultOptions_();
}


Environment::~Environment()
{
  delete timer_;
  delete timerFac_;
}


void Environment::createDefaultOptions_()
{
  BoolOptionPtr b_option;
  IntOptionPtr i_option;
  DoubleOptionPtr d_option;
  StringOptionPtr s_option;
  FlagOptionPtr f_option;

  // bool options
  b_option = (BoolOptionPtr) new Option<Bool>("show_version", 
      "Write version number of Minotaur: <0/1>",
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("show_options", 
      "Write all options and their default values: <0/1>",
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("show_help", 
      "Show usage and exit: <0/1>",
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("use_internal_quad", 
      "Should quadratic functions be treated natively: <0/1>",
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("use_warmstart", 
      "Should warm starts be used to solve relaxations: <0/1>",
      true, true);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("presolve", 
      "Should presolve be used: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("display_problem", 
      "Should display problem before solving: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("display_presolved_problem", 
      "Should display problem after presolving: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("display_size", 
      "Should display problem size before solving: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("display_presolved_size", 
      "Should display problem size after presolving: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("guided_dive", 
      "Should perform guided dives after branching: <0/1>", true, true);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("solve", 
      "Should solve the problem: <0/1>", true, true);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("lin_presolve", 
      "Should presolve using linear handler: <0/1>", true, true);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("quad_presolve", 
      "Should presolve using quadratic handler: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("ignore_SOS1", 
      "Ignore all SOS1 constraints: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("ignore_SOS2", 
      "Ignore all SOS2 constraints: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("nl_presolve", 
      "Should presolve using nonlinear presolver: <0/1>", true, true);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("lin_show_stats", 
      "Should show statistics of linear handler: <0/1>", true, 
      true);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("MSheur", 
      "Use multi-start heuristic for continuous nonlinear problem: <0/1>", 
      true, false);
  options_->insert(b_option);
  
  b_option = (BoolOptionPtr) new Option<Bool>("FPump", 
      "Use feasibility pump heuristic for MINLP: <0/1>", true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("modify_rel_only", 
      "If true, apply all all modifications to relaxation only  <0/1>",
      true, true);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("convexity_type",
      "Is the problem convex or quasiconvex: convexity <=> 0 and quasiconvexity <=> 1", 
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("qg_accpm",
      "Do analytic center cutting plane cuts in qg algorithm: <0/1>", 
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("partial_BB",
      "Fix the bounds partially in QGHandler: <0/1>",
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("expand_poly", 
      "Fully expand and save polynomials in objective and constraints: <0/1>",
      true, false);
  options_->insert(b_option);

  b_option = (BoolOptionPtr) new Option<Bool>("expand_quad", 
      "Fully expand and save quadratic objective and constraints: <0/1>",
      true, true);
  options_->insert(b_option);
  b_option = (BoolOptionPtr) new Option<Minotaur::Bool> ("use_native_cgraph", 
     "If true, use Minotaur's computational graph to evaluate nonlinear functions and their derivatives. <0/1>", true, false);
  options_->insert(b_option);

  // reset, so that we don't accidently add it again.
  b_option.reset();

  // // flags (options without values)
  //f_option = (FlagOptionPtr) new Option<Bool>("AMPL", 
  //    "-AMPL option tells us to write .sol file for ampl", 
  //    true, false);
  //options_->insert(f_option, true);

  // reset, so that we don't accidently add it again.
  f_option.reset();


  // int options
  i_option = (IntOptionPtr) new Option<Int>("bnb_node_limit", 
      "Limit on number of nodes created in branch-and-bound: >0",
      true, 1000000000);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("bnb_sol_limit", 
      "Limit on the number of solutions found: >0", true, 
      1000000000);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("ampl_log_level", 
      "Verbosity of ampl interface: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("bqpd_log_level", 
      "Verbosity of bqpd engine: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("filter_sqp_log_level", 
      "Verbosity of Filter-SQP engine: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("osilp_log_level", 
      "Verbosity of OsiLP engine: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("ipopt_log_level", 
      "Verbosity of Ipopt engine: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("bqpd_ws_mode", 
      "Warm starting mode for bqpd: 0-6", true, 6);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("br_log_level", 
      "Verbosity of brancher: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("bnb_log_level", 
      "Verbosity of branch-and-bound: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("node_processor_log_level", 
      "Verbosity of node processor: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("presolve_log_level", 
      "Verbosity of presolver: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("handler_log_level", 
      "Verbosity of handlers: 0-6", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("MSheur_log_level", 
      "Verbosity of Multi Start Heuristic: 0-6", true, LogInfo);
  options_->insert(i_option);
  
  i_option = (IntOptionPtr) new Option<Int>("Divheur_log_level", 
      "Verbosity of MINLP Diving Heuristic ", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("FPump_log_level", 
      "Verbosity of Feasibility Pump Heuristic ", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("LinFPump", 
      "Use Linear feasibility pump heuristic for MINLP: <-1/0/1>", true, -1);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("LinFPump_log_level", 
      "Verbosity of Linear Feasibility Pump Heuristic ", true, LogInfo);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("Divheur", 
      "Use diving heuristic for MINLP: <-1/0/1>", 
      true, -1);
  options_->insert(i_option);

   i_option = (IntOptionPtr) new Option<Int>("strbr_pivot_limit",
      "Limit on number of QP pivots allowed during strong branching: >0",
      true, 25);
  options_->insert(i_option);
 
  i_option = (IntOptionPtr) new Option<Int>("trans_log_level", 
      "Verbosity of Transformer ", true, LogInfo);
  options_->insert(i_option);

  // Serdar added these options for MultilinearTermsHandler class
  i_option = (IntOptionPtr) new Option<Int>("ml_log_level", 
      "MultilinearTermsHandler log level.", true, 0);
  options_->insert(i_option);

  i_option = (IntOptionPtr) new Option<Int>("ml_max_group_size",
       "Maximum size of individual element in grouping: >= 2, <= 20", true, 6);
  options_->insert(i_option);

  // Serdar ended.

  i_option = (IntOptionPtr) new Option<Int>("rand_seed", 
      "Seed to random number generator: >=0 (0 = time(NULL))", true, 0);
  options_->insert(i_option);

  i_option.reset();

  
  // double options
  
  // Serdar added these options for MultilinearTermsHandler class
  d_option = (DoubleOptionPtr) new Option<Double>("ml_feastol", 
      "MultilinearTermsHandler feasibility tolerance.", true, 0.00001);
  options_->insert(d_option);

  // Serdar added these options for MultilinearTermsHandler class
  d_option = (DoubleOptionPtr) new Option<Double>("ml_cover_augmentation_factor", 
      "Covering augmentation factor for ml grouping: >= 1", true, 2.0);
  options_->insert(d_option);

  // Serdar ended.

  d_option = (DoubleOptionPtr) new Option<Double>("bnb_time_limit", 
      "Limit on time in branch-and-bound in seconds: >0",
      true, 1e20);
  options_->insert(d_option);
  
  d_option = (DoubleOptionPtr) new Option<Double>("bnb_log_interval", 
      "Display interval in seconds for branch-and-bound status: >0", true, 
      5.);
  options_->insert(d_option);

  d_option = (DoubleOptionPtr) new Option<Double>("obj_cut_off", 
      "Nodes with objective value above obj_cut_off are assumed infeasible",
      true, INFINITY);
  options_->insert(d_option);

  d_option = (DoubleOptionPtr) new Option<Double>("obj_gap_percent", 
      "Prune nodes with obj. value greater than (1-obj_gap/100)*best_value.",
      true, 0.0);
  options_->insert(d_option);

  d_option = (DoubleOptionPtr) new Option<Double>("int_tol", 
      "Tolerance for checking integrality",
      true, 0.00001);
  options_->insert(d_option);

  d_option.reset();
 
  // Serdar added default options for MultilinearTermsHandler.
  s_option = (StringOptionPtr) new Option<std::string>("ml_group_strategy",
      "Group strategy", true, "TC");
  options_->insert(s_option);
  // Serdar ended

  // string options
  s_option = (StringOptionPtr) new Option<std::string>("interface_type", 
      "What interface is this environment being used with: AMPL or C++",
      true, "C++");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("lp_engine", 
      "Engine for solving Linear Relxations: Osi, None", 
      true, "OsiClp");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("nlp_engine", 
      "Engine for solving nonlinear relaxations: Filter-SQP, IPOPT, None", 
      true, "Filter-SQP");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("qp_engine", 
      "Engine for solving QP relaxations: bqpd, None", 
      true, "bqpd");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("brancher", 
      "Name of brancher: rel, maxvio, lex, rand, maxfreq", 
      true, "rel");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("tree_search", 
      "Strategy for tree search: dfs, bfs, BthenD", true, "BthenD");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("vbc_file", 
      "File name for storing tree information for Vbctool", true, "");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("config_file", 
      "Name of file that contains parameters or options", true, "");
  options_->insert(s_option);

  s_option = (StringOptionPtr) new Option<std::string>("problem_file", 
      "Name of file that contains the instance to be solved", 
      true, "");
  options_->insert(s_option);

  s_option.reset();
}


void Environment::convertAndAddOneOption_(BoolOptionPtr &b_option, 
    IntOptionPtr &i_option, DoubleOptionPtr &d_option, 
    StringOptionPtr &s_option, FlagOptionPtr &f_option, std::string &name,
    std::string &value)
{
  std::stringstream mystream; // to convert string to int, bool, double.
  std::string off = "  ";
  // now add the option.
  if (b_option) {
    Bool b_value = getBoolValue_(value);
    b_option->setValue(b_value);
    logger_->MsgStream(LogInfo) << off << b_option->getName() << " = " <<
      b_option->getValue() << std::endl; 
  } else if (i_option) {
    Int i_value;
    mystream << std::flush;
    mystream << value;
    mystream >> i_value;
    i_option->setValue(i_value);
    logger_->MsgStream(LogInfo) << off << i_option->getName() << " = " <<
      i_option->getValue() << std::endl; 
  } else if (d_option) {
    Double d_value;
    mystream << std::flush;
    mystream << value;
    mystream >> d_value;
    d_option->setValue(d_value);
    logger_->MsgStream(LogInfo) << off << d_option->getName() << " = " <<
      d_option->getValue() << std::endl; 
  } else if (s_option) {
    s_option->setValue(value);
    logger_->MsgStream(LogInfo) << off << s_option->getName() << " = " <<
      s_option->getValue() << std::endl; 
  } else {
    // option is unknown. We will add this option with the following
    // assumption. If the string has an '=' in it (i.e. value!=""), 
    // then it is a string
    // option, otherwise it is a 'flag' option. We can not have 'int' or
    // 'double' option here.
    logger_->MsgStream(LogInfo) << off << name << " is not a known option. ";
    if (value == "") {
      logger_->MsgStream(LogInfo) << "Added as a flag." 
        << std::endl;
      f_option = (FlagOptionPtr) new Option<Bool>(name, 
          "Flag added by user", false, true);
      options_->insert(f_option, true);
    } else {
      logger_->MsgStream(LogInfo) << "Added as a string option with value \"" 
        << value << "\"" << std::endl;
      s_option = (StringOptionPtr) new Option<std::string>(name, 
          "Option added by user", false, value);
      options_->insert(s_option);
    }
  }
}


void Environment::findOption_(const std::string &name, BoolOptionPtr &b_option,
    IntOptionPtr &i_option, DoubleOptionPtr &d_option, 
    StringOptionPtr &s_option, FlagOptionPtr &f_option)
{
  b_option = BoolOptionPtr();   // NULL
  i_option = IntOptionPtr();    // NULL
  d_option = DoubleOptionPtr(); // NULL
  s_option = StringOptionPtr(); // NULL
  f_option = FlagOptionPtr();   // NULL

  b_option = options_->findBool(name);
  if (b_option) {
    return;
  } 

  i_option = options_->findInt(name);
  if (i_option) {
    return;
  } 

  d_option = options_->findDouble(name);
  if (d_option) {
    return;
  }

  s_option = options_->findString(name);
  if (s_option) {
    return;
  }

  f_option = options_->findFlag(name);
}


Bool Environment::getBoolValue_(const std::string & str)
{
  if (str=="") {
    return false;
  }

  if (str=="Y" || str=="YES" || str == "y" || str=="yes" || str == "T" ||
     str=="TRUE" || str=="t" || str=="true" || str=="1") {
    return true;
  }

  return false;
}


LoggerPtr Environment::getLogger() const
{
  return logger_;
}


Timer* Environment::getNewTimer() 
{
  return timerFac_->getTimer();
}


OptionDBPtr Environment::getOptions()
{
  return options_;
}


Double Environment::getTime(Int &err)
{
  if (timer_) {
    err = 0;
    return timer_->query();
  } 
#if SPEW
  logger_->MsgStream(LogError) << me_ <<
    "timer queried before it is started." << std::endl;
#endif
  err = 1;
  return 0.0;
}


const Timer* Environment::getTimer()
{
  return timer_;
}


std::string Environment::getVersion() 
{
  std::stringstream name_stream;
  name_stream << MINOTAUR_MAJOR_VERSION <<  '.' <<  MINOTAUR_MINOR_VERSION
              << " patch " <<  MINOTAUR_PATCH_VERSION;
  return name_stream.str();
}


void Environment::readConfigFile_(std::string fname, UInt &num_p)
{
  std::string line, w1, w2;
  std::ifstream istr(fname.c_str());
  const std::string spc = " \t\r\n";
  char comment = '#';
  BoolOptionPtr b_option;
  IntOptionPtr i_option;
  DoubleOptionPtr d_option;
  StringOptionPtr s_option;
  FlagOptionPtr f_option;

  if (!istr.is_open()) {
    logger_->ErrStream() << me_ << "cannot open file " << fname << std::endl; 
    return;
  }

  while (istr.good()) {
    getline(istr, line);
    if (line.empty()) {
      continue;
    }
    // remove everything after comment.
    line = line.substr(0, line.find_first_of(comment));

    // find first word. It is parameter name.
    w1 = line.substr(0, line.find_first_of(spc));

    // remove first word from line. Get second word. It is the value.
    line = line.substr(w1.length()); 
    w2 = line.substr(0,line.find_first_not_of(spc)); 
    if (w2.length()==line.length()) {
      w2="";
    } else {
      w2 = line.substr(w2.length());
    }
    w2 = w2.substr(0,w2.find_first_of(spc));

    if (w1.empty()) {
      continue;
    } 
    if (w2.empty()) {
      logger_->ErrStream() << me_ << "parameter " << w1 << " in configuration file " 
        << fname << " has no value. Ignored this parameter." << std::endl;
      continue;
    }
    if ("problem_file"==w1) {
      ++num_p;
    }
    // add the option.
    findOption_(w1, b_option, i_option, d_option, s_option, f_option);
    convertAndAddOneOption_(b_option, i_option, d_option, s_option,
        f_option, w1, w2);
  }
  istr.close();
}


void Environment::readOptions(Int argc, char **argv)
{
  std::string name, s_value;
  Int leading_dashes;
  UInt num_p = 0; // number of filenames provided for solve.
  BoolOptionPtr b_option;
  IntOptionPtr i_option;
  DoubleOptionPtr d_option;
  StringOptionPtr s_option;
  FlagOptionPtr f_option;
  std::string offset = "  ";

  if (argc<2) {
    logger_->MsgStream(LogInfo) << me_ 
      << "User provided no options." << std::endl;
  } else {
    logger_->MsgStream(LogInfo) << me_ 
      << "User provided options:" << std::endl;
  }
  for (Int i=1; i<argc; ++i) {
    name = argv[i];

    // removing leading dashes
    leading_dashes = 0;
    leading_dashes = removeDashes_(name);
    assert(name.size()>0);

    if (leading_dashes==0) {
      // looks like an name of instance
      s_option = options_->findString("problem_file");
      s_option->setValue(name);
      ++num_p;
      logger_->MsgStream(LogInfo) << offset << s_option->getName() << " = " <<
        s_option->getValue() << std::endl; 
    } else {
      // looks like an option. remove leading minotaur dot, if any.
      removeMinotaurPrepend_(name);

      // we have an option for which we need an argument also. First check
      // if the option contains an '=' sign followed by a string.
      s_value = separateEqualToArg_(name);

      // find if it is a recognized option
      findOption_(name, b_option, i_option, d_option, s_option, f_option);

      if (f_option) { // true means we have a known flag.
        if (s_value=="") { 
          f_option->setValue(true);
        } else {
          f_option->setValue(getBoolValue_(s_value));
        }
        logger_->MsgStream(LogInfo) << offset << f_option->getName() << " = " <<
          f_option->getValue() << std::endl; 
      } else {
        if (s_value=="" && (b_option || i_option || d_option || s_option)) {
          // the next argv is the argument
          assert(i+1<argc);       // throw exception instead
          ++i;
          s_value = argv[i];
        }
        convertAndAddOneOption_(b_option, i_option, d_option, s_option,
            f_option, name, s_value);
        if (s_option && "config_file"==s_option->getName()) {
          readConfigFile_(s_option->getValue(), num_p);
        }
      }
    }
  }
  if (argc>1) {
    logger_->MsgStream(LogInfo) << me_ 
      << "End of user provided options." << std::endl << std::endl; 
  }

  if (num_p>1) {
    logger_->MsgStream(LogInfo) << me_ 
      << "more than one filename given as input."
      << std::endl
      << me_ << "Only file \"" 
      << options_->findString("problem_file")->getValue()
      << "\" will be read." << std::endl << std::endl;
  } else if (num_p==0) {
    logger_->MsgStream(LogDebug) << me_ 
      << "No filename provided as input." << std::endl;
  }
}


UInt Environment::removeDashes_(std::string &name)
{
  size_t first_occ;

  // exit if string is NULL.
  if (name.length()<1) {
    return 0;
  }

  // check for '--'
  first_occ = name.find("--");
  if (first_occ==0) {
    name = name.substr(2); // todo catch exception out_of_range.
    return 2;
  } 

  // check for '-'
  first_occ = name.find("-");
  if (first_occ==0) {
    name = name.substr(1); // todo catch exception out_of_range.
    return 1;
  }

  return 0;
}


/// \todo implement me.
void Environment::removeMinotaurPrepend_(std::string & )
{
  // std::cout << fullname << std::endl;
}


std::string Environment::separateEqualToArg_(std::string &name)
{
  if (name.size()>1) {
    // search for the '=' sign from position 1 only (can't be at 0).
    size_t pos = name.find('=', 1);
    if (pos == std::string::npos) {
      // '=' not found
      return "";
    } else {
      std::string value = "";
      value = name.substr(pos+1);
      name = name.substr(0, pos);
      return value;
    }
  }
  return "";
}


void Environment::startTimer(Int &err)
{
  if (timer_) {
    err = 0;
    timer_->start();
  } else {
    err = 1;
  }
}


void Environment::stopTimer(Int &err)
{
  if (timer_) {
    err = 0;
    timer_->stop();
  } else {
    err = 1;
  }
}


void Environment::writeFullVersion(std::ostream &out)
{
  out << "Minotaur full version " << getVersion() << " svn version "
      << MINOTAUR_SVN_VERSION;
}


// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
