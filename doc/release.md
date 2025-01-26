// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

\page release How to make a new release

Update copyright information
  * run scripts/change_copyright.sh
  * modify other files manually (doc/\*md, LICENSE etc)

Then prepare the binaries
  * Prepare new third-party libraries 
  * Update the version number in src/base/Version.h.cmake and Version.h.manual
  * commit to git
  * compile the binaries to be distributed
  * test

Finally
  * create a tag in git and push
  * host the binaries on the IEOR server
  * update the website
  * announce over email/coin 

