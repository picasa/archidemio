FILE(REMOVE_RECURSE
  "CMakeFiles/doc_modeling"
  "doxygen/modeling/index.html"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/doc_modeling.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
