Usage
=====

Quick Start
-----------

1. **Include Patternia**
   .. code-block:: cpp

      #include <ptn/patternia.hpp>
      using namespace ptn;

2. **Write a match expression**
   .. code-block:: cpp

      auto msg = match(x)
          .when(lt(0) >> "negative")
          .when(between(0, 10) >> "small")
          .otherwise("other");

3. **Integrate with CMake**
   .. code-block:: cmake

      FetchContent_Declare(
        patternia
        GIT_REPOSITORY https://github.com/sentomk/patternia.git
        GIT_TAG main
      )
      FetchContent_MakeAvailable(patternia)
      target_link_libraries(your_target PRIVATE patternia)
