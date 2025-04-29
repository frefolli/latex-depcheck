#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <filesystem>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-latex.h>
#include <unordered_map>

struct Index {
  std::unordered_map<std::string, bool> modules;

  void index_directory(const std::string& directory) {
    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(directory)) {
      if (dir_entry.is_regular_file()) {
        std::string filename = dir_entry.path().filename();
        if (filename.ends_with(".sty")) {
          std::string module_name = filename.substr(0, filename.size() - strlen(".sty"));
          this->modules[module_name] = true;
        }
      }
    }
  }

  void print() {
    for (auto it : modules) {
      printf("> %s\n", it.first.c_str());
    }
  }
};

inline char* read_source_code(const char* filepath) {
    char* text = NULL;
    FILE* file = fopen(filepath, "r");
    if (!file)
      return NULL;
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    text = (char*) malloc(fsize + 1);
    if (!text) {
      fclose(file);
      return NULL;
    }
    text[fsize] = '\0';
    fsize = fread(text, fsize, 1, file);
    fclose(file);
    return text;
}

std::string slice_string(const char* source, uint32_t start_idx, uint32_t end_idx) {
  const char* slice_start = source + start_idx;
  uint32_t slice_length = end_idx - start_idx;

  std::string buffer;
  buffer.resize(slice_length);

  memcpy(buffer.data(), slice_start, slice_length);
  return buffer;
}

std::string strip_string(std::string consumed) {
  uint32_t idx = 0;
  uint32_t len = consumed.size();
  while (idx < len && consumed[idx] == ' ') {
    idx++;
  }

  if (idx >= len) {
    return "";
  }

  uint32_t start_idx = idx;

  idx = len;
  while (idx > 0 && consumed[idx - 1] == ' ') {
    idx--;
  }

  if (idx == 0) {
    if (consumed[idx] == ' ') {
      return "";
    } else {
      return consumed.substr(1, 1);
    }
  }

  uint32_t end_idx = idx;
  uint32_t length = end_idx - start_idx;
  return consumed.substr(start_idx, length);
}

void parse_latex_file(const Index& index, const TSLanguage* language, const char* source_code, TSNode& root_node) {
  std::string suggestion = "";
  uint32_t node_count = ts_node_named_child_count(root_node);
  for (uint32_t idx = 0; idx < node_count; ++idx) {
    TSNode child_node = ts_node_named_child(root_node, idx);
    const char* child_symbol = ts_language_symbol_name(language, ts_node_symbol(child_node));
    if (strcmp(child_symbol, "package_include") == 0) {
      TSNode paths_node = ts_node_child_by_field_name(child_node, "paths", strlen("paths"));
      const char* paths_symbol = ts_language_symbol_name(language, ts_node_symbol(paths_node));
      if (strcmp(paths_symbol, "curly_group_path_list") == 0) {
        uint32_t node_count = ts_node_named_child_count(paths_node);
        for (uint32_t idx = 0; idx < node_count; ++idx) {
          TSNode path_node = ts_node_named_child(paths_node, idx);
          std::string path = strip_string(slice_string(source_code, ts_node_start_byte(path_node), ts_node_end_byte(path_node)));
          if (!index.modules.contains(path)) {
            printf("Missing module \"%s\"\n", path.c_str());
            if (!suggestion.empty())
              suggestion += " ";
            suggestion += "texlive-" + path;
          }
        }
      }
    }
  }

  if (!suggestion.empty()) {
    printf("Suggestion: `sudo dnf install %s`\n", suggestion.c_str());
  }
}

int main(int argc, char **argv) {
  Index index;
  index.index_directory("/usr/share/texlive");
  index.index_directory(".");
  // index.print();

  const char* source_file = "./file.tex";
  if (argc > 1) {
    source_file = argv[1];
  }

  char* source = read_source_code(source_file);
  
  if (!source) {
    fprintf(stderr, "Unable to read source file: %s\n", source_file);
    return 1;
  }

  const TSLanguage* language = tree_sitter_latex();
  TSParser* parser = ts_parser_new();
  ts_parser_set_language(parser, language);
  TSTree* tree = ts_parser_parse_string(parser, NULL, source, strlen(source));

  TSNode root_node = ts_tree_root_node(tree);
  parse_latex_file(index, language, source, root_node);

  ts_tree_delete(tree);
  ts_parser_delete(parser);
  free(source);

  return 0;
}
