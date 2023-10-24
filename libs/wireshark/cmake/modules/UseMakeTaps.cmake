#
MACRO(REGISTER_TAP_FILES _outputfile)
    set(_sources ${ARGN})
    ADD_CUSTOM_COMMAND(
        OUTPUT
          ${_outputfile}
        COMMAND
          ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/tools/make-regs.py taps ${_outputfile} ${_sources}
        DEPENDS
          ${PROJECT_SOURCE_DIR}/tools/make-regs.py
          ${_sources}
        COMMENT
          "Making ${_outputfile}"
)
ENDMACRO(REGISTER_TAP_FILES)
