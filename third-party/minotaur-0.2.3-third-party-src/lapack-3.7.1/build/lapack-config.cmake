# Load lapack targets from the build tree if necessary.
set(_LAPACK_TARGET "blas")
if(_LAPACK_TARGET AND NOT TARGET "${_LAPACK_TARGET}")
  include("/home/mustafa.vora/minotaur/third-party/minotaur-0.2.3-third-party-src/lapack-3.7.1/build/lapack-targets.cmake")
endif()
unset(_LAPACK_TARGET)

# Report the blas and lapack raw or imported libraries.
set(LAPACK_blas_LIBRARIES "blas")
set(LAPACK_lapack_LIBRARIES "lapack")
