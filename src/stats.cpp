/******************************************************************************\
* Copyright (c) 2019, Robert van Engelen, Genivia Inc. All rights reserved.    *
*                                                                              *
* Redistribution and use in source and binary forms, with or without           *
* modification, are permitted provided that the following conditions are met:  *
*                                                                              *
*   (1) Redistributions of source code must retain the above copyright notice, *
*       this list of conditions and the following disclaimer.                  *
*                                                                              *
*   (2) Redistributions in binary form must reproduce the above copyright      *
*       notice, this list of conditions and the following disclaimer in the    *
*       documentation and/or other materials provided with the distribution.   *
*                                                                              *
*   (3) The name of the author may not be used to endorse or promote products  *
*       derived from this software without specific prior written permission.  *
*                                                                              *
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF         *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO   *
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,       *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, *
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;  *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,     *
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF       *
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                   *
\******************************************************************************/

/**
@file      stats.cpp
@brief     collect global statistics - static, partially thread-safe
@author    Robert van Engelen - engelen@genivia.com
@copyright (c) 2019-2022, Robert van Engelen, Genivia Inc. All rights reserved.
@copyright (c) BSD-3 License - see LICENSE.txt
*/

#include "stats.hpp"
#include <cstdlib>
#include <cstdio>

// report the statistics
void Stats::report(FILE *output)
{
  size_t sf = searched_files();
  size_t sd = searched_dirs();
  size_t sl = searched_lines();
  size_t ff = found_files();
  size_t fp = found_parts();
  size_t fm = found_matches();
  size_t ws = Static::warnings;

  fprintf(output, NEWLINESTR "Searched %zu file%s", sf, (sf == 1 ? "" : "s"));
  if (sd > 0)
    fprintf(output, " in %zu director%s", sd, (sd == 1 ? "y" : "ies"));
  if (!flag_query && flag_pager == NULL)
    fprintf(output, " in %.3g seconds", 0.001 * reflex::timer_elapsed(timer));
  if (Static::threads > 1)
    fprintf(output, " with %zu threads", Static::threads);
  fprintf(output, ": %zu matching (%.4g%%)", ff, 100.0 * ff / sf);
  if (fp > ff)
    fprintf(output, " + %zu in archives", fp - ff);
  fprintf(output, NEWLINESTR);

  if (fm > 0 && !flag_quiet && !flag_files_with_matches && !flag_files_without_match)
  {
    if (flag_ungroup || (flag_count && flag_only_matching) || (!flag_count && flag_format != NULL))
      fprintf(output, "Searched %zu line%s: found %zu match%s (ungrouped)" NEWLINESTR, sl, (sl == 1 ? "" : "s"), fm, (fm == 1 ? "" : "es"));
    else
      fprintf(output, "Searched %zu line%s: %zu matching (%.4g%%)" NEWLINESTR, sl, (sl == 1 ? "" : "s"), fm, 100.0 * fm / sl);
  }

  if (flag_index && indexed)
  {
    fprintf(stderr, "Skipped %zu of %zu indexed files with indexes not matching any search patterns\n", skipped, indexed);
    if (changed > 0 || added > 0)
    {
      fprintf(output, "Detected outdated or missing index files, run ugrep-indexer to re-index:\n");
      if (changed > 1)
        fprintf(output, "  searched %zu changed files\n", changed);
      else if (changed == 1)
        fprintf(output, "  searched 1 changed file\n");
      if (added > 1)
        fprintf(output, "  searched %zu new files\n", added);
      else if (added == 1)
        fprintf(output, "  searched 1 new file\n");
    }
  }

  if (Static::warnings > 0)
    fprintf(output, "Received %zu warning%s" NEWLINESTR, ws, ws == 1 ? "" : "s");

  fprintf(output, "The following pathname selections and search constraints were applied:" NEWLINESTR);
  if (flag_config != NULL)
  {
    fprintf(output, "  --config=%s" NEWLINESTR, flag_config);
    for (const auto& i : flag_config_files)
      fprintf(output, "    using %s" NEWLINESTR, i.c_str());
  }
  if (flag_bool)
    fprintf(output, "  --bool %s" NEWLINESTR, (flag_files ? "--files" : "--lines"));
  if (flag_basic_regexp)
    fprintf(output, "  --basic-regexp" NEWLINESTR);
  else if (flag_fixed_strings)
    fprintf(output, "  --fixed-strings" NEWLINESTR);
  else if (flag_fuzzy > 0)
    fprintf(output, "  --fuzzy" NEWLINESTR);
  else if (flag_perl_regexp)
    fprintf(output, "  --perl-regexp" NEWLINESTR);
  if (flag_decompress)
    fprintf(output, "  --decompress --zmax=%zu" NEWLINESTR, flag_zmax);
  if (flag_min_depth > 0 && flag_max_depth > 0)
    fprintf(output, "  --depth=%zu,%zu" NEWLINESTR, flag_min_depth, flag_max_depth);
  else if (flag_min_depth > 0)
    fprintf(output, "  --depth=%zu," NEWLINESTR, flag_min_depth);
  else if (flag_max_depth > 0)
    fprintf(output, "  --depth=%zu" NEWLINESTR, flag_max_depth);
  if (flag_dereference)
    fprintf(output, "  --dereference" NEWLINESTR);
  else if (flag_no_dereference)
    fprintf(output, "  --no-dereference" NEWLINESTR);
  switch (flag_devices_action)
  {
    case Action::SKIP:
      fprintf(output, "  --devices=skip" NEWLINESTR);
      break;
    case Action::READ:
      fprintf(output, "  --devices=read" NEWLINESTR);
      break;
    default:
      break;
  }
  switch (flag_directories_action)
  {
    case Action::SKIP:
      fprintf(output, "  --directories=skip" NEWLINESTR);
      break;
    case Action::READ:
      fprintf(output, "  --directories=read" NEWLINESTR);
      break;
    case Action::RECURSE:
      fprintf(output, "  --directories=recurse" NEWLINESTR);
      break;
    default:
      break;
  }
  if (flag_glob_ignore_case)
    fprintf(output, "  --glob-ignore-case" NEWLINESTR);
#ifdef WITH_HIDDEN
  if (flag_hidden)
    fprintf(output, "  --hidden (default)" NEWLINESTR);
  else
    fprintf(output, "  --no-hidden" NEWLINESTR);
#else
  if (flag_hidden)
    fprintf(output, "  --hidden" NEWLINESTR);
  else
    fprintf(output, "  --no-hidden (default)" NEWLINESTR);
#endif
  for (const auto& i : flag_ignore_files)
    fprintf(output, "  --ignore-files=\"%s\"" NEWLINESTR, i.c_str());
  if (flag_index != NULL)
    fprintf(output, "  --index" NEWLINESTR);
  if (flag_min_count > 0)
    fprintf(output, "  --min-count=%zu" NEWLINESTR, flag_min_count);
  if (flag_max_count > 0)
    fprintf(output, "  --max-count=%zu" NEWLINESTR, flag_max_count);
  if (flag_max_files > 0)
    fprintf(output, "  --max-files=%zu" NEWLINESTR, flag_max_files);
  if (flag_min_line > 0)
    fprintf(output, "  --min-line=%zu" NEWLINESTR, flag_min_line);
  if (flag_max_line > 0)
    fprintf(output, "  --max-line=%zu" NEWLINESTR, flag_max_line);
  if (flag_sort != NULL)
    fprintf(output, "  --sort=%s" NEWLINESTR, flag_sort);
  for (const auto& i : ignore)
    fprintf(output, "    %s exclusions were applied to %s and its subdirectories" NEWLINESTR, i.c_str(), i.substr(0, i.find_last_of(PATHSEPCHR)).c_str());
  for (const auto& i : flag_file_magic)
  {
    if (!i.empty() && (i.front() == '!' || i.front() == '^'))
      fprintf(output, "  --file-magic=\"!%s\" (negated)" NEWLINESTR, i.c_str() + 1);
    else
      fprintf(output, "  --file-magic=\"%s\"" NEWLINESTR, i.c_str());
  }
  for (const auto& i : flag_include_fs)
    fprintf(output, "  --include-fs=\"%s\"" NEWLINESTR, i.c_str());
  for (const auto& i : flag_exclude_fs)
    fprintf(output, "  --exclude-fs=\"%s\"" NEWLINESTR, i.c_str());
  for (const auto& i : flag_all_include)
    fprintf(output, "  --include=\"%s\"%s%s" NEWLINESTR, i.c_str(), i.front() == '!' ? " (negated)" : "", &i < &flag_all_include.front() + flag_include_iglob_size ? " (ignore case)" : "");
  for (const auto& i : flag_all_exclude)
    fprintf(output, "  --exclude=\"%s\"%s%s" NEWLINESTR, i.c_str(), i.front() == '!' ? " (negated)" : "", &i < &flag_all_exclude.front() + flag_exclude_iglob_size ? " (ignore case)" : "");
  for (const auto& i : flag_all_include_dir)
    fprintf(output, "  --include-dir=\"%s\"%s%s" NEWLINESTR, i.c_str(), i.front() == '!' ? " (negated)" : "", &i < &flag_all_include_dir.front() + flag_include_iglob_dir_size ? " (ignore case)" : "");
  for (const auto& i : flag_all_exclude_dir)
    fprintf(output, "  --exclude-dir=\"%s\"%s%s" NEWLINESTR, i.c_str(), i.front() == '!' ? " (negated)" : "", &i < &flag_all_exclude_dir.front() + flag_exclude_iglob_dir_size ? " (ignore case)" : "");
}

reflex::timer_type       Stats::timer;
size_t                   Stats::files   = 0;
size_t                   Stats::dirs    = 0;
size_t                   Stats::indexed = 0;
size_t                   Stats::skipped = 0;
size_t                   Stats::changed = 0;
size_t                   Stats::added   = 0;
std::atomic_size_t       Stats::fileno;
std::atomic_size_t       Stats::partno;
std::atomic_size_t       Stats::matchno;
std::atomic_size_t       Stats::lineno;
std::vector<std::string> Stats::ignore;
