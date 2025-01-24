// 
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//

// /**
// Some headers in external packages like Ipopt and Osi redefine the
// following. To avoid compiler warnings, we undefine them here. This should
// not be included from any header files because if someone else includes our
// headers in their package, then this will clear their definitions.
//
// This should be called from .cpp files only much like MinotaurConfig.h
//
// MinotaurConfig.h is not present in the source code but is generated at
// build time by configure. This file should not be included in any of the
// Minotaur headers as it has generic names that may conflict with similar
// definitions in other packages.
//
// \todo For now we have MINOTAUR_RUSAGE also defined in MinotaurConfig.h. Not
// sure if it belongs there.
// */

#undef PACKAGE_BUGREPORT

#undef PACKAGE_NAME

#undef PACKAGE_STRING

#undef PACKAGE_TARNAME

#undef PACKAGE_VERSION

#undef F77_FUNC_

