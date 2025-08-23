include(FindPackageHandleStandardArgs)

# Sphinx is expected to be installed to the Python install found with
# FindPython3, thus use the script folder of that Python install as a hint path
execute_process(
  COMMAND "${Python3_EXECUTABLE}" -c
          "import sysconfig; print(sysconfig.get_path('scripts'))"
  OUTPUT_VARIABLE _Python3_Script_Path
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Look for an executable called sphinx-build
find_program(
  SPHINX_EXECUTABLE
  NAMES sphinx-build sphinx-build.exe
  HINTS "${_Python3_Script_Path}"
  DOC "Path to sphinx-build executable")
mark_as_advanced(SPHINX_EXECUTABLE)

# Handle standard arguments to find_package like REQUIRED and QUIET
find_package_handle_standard_args(
  Sphinx "Failed to find sphinx-build executable" SPHINX_EXECUTABLE)
