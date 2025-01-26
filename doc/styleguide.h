// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

//
// Minotaur source style guide.
//

/**
\page styleguide Minotaur style guide for developers

The most important guideline is to be consistent!

  - No line should exceed 80 characters including spaces.
  - Use spaces for indentation. No tabs.
  - All warnings from the compiler must be fixed.

<b>Naming Convention</b>
  - Class names are camel-case with first letter in upper case, e.g.
    TreeManager and not tree_manager or treeManager.  
  - Function names are camel-case with first letter in lower case, e.g.
    TreeManager::getCandidate, and not get_candidate or GetCandidate.
  - Variable names should be brief.
  - All private and protected variables and functions must end in an 
    underscore, e.g. n_, m_, myvar_, doFun_()

<b>Header Files</b>
  - Header file must have doxygen documentation.
  - Header file must have author, file and brief doxygen tags
  - File name should end in '.h'
  - Members of a class should be declared in the following order:
    -# Constructors
    -# Destructor
    -# public functions in alphabetic order
    -# protected variables in alphabetic order
    -# protected functions in alphabetic order
    -# private variables in alphabetic order
    -# private functions in alphabetic order

<b>Source Files</b>
  - Header file must have author, file and brief doxygen tags
  - File name should end in '.cpp'
  - Members of a class should be defined in the following order:
    -# Constructors
    -# Destructor
    -# All other functions in alphabetic order.

<b>Coding Guidelines</b>
  - All functions should be at most 80 lines including blank lines, and debug
    messages. 
  - All if, for, while etc statements must have { } following them. Don't do \n
    if (x) ++i; \n
    Instead do\n
    if (x) { \n
    &nbsp;&nbsp;++i; \n
    }
  - Do not throw exceptions. We do not know what to do with them.
  - Code in base/ directory should not include any files from interfaces/
    solvers/ or engines/ directories
*/

