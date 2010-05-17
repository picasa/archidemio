FILE(REMOVE_RECURSE
  "CMakeFiles/doc_sources"
  "doxygen/sources/index.html"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/doc_sources.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
