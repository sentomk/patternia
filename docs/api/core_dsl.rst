Patternia Entry Point
=====================

The `match()` function is the public entry to the DSL:

.. code-block:: cpp

   auto res = match(5)
       .when(gt(0) >> "positive")
       .when(lt(0) >> "negative")
       .otherwise("zero");

.. doxygenfile:: patternia.hpp
   :project: Patternia
