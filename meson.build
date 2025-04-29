project('latex-depcheck', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++20'])

tree_sitter_latex = dependency('tree-sitter-latex')
tree_sitter = dependency('tree-sitter')

exe = executable('latex-depcheck', [
  'src/main.cc'
], dependencies: [tree_sitter_latex, tree_sitter], install : true)
