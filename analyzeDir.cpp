/// =========================================================================
/// Written by pfederl@ucalgary.ca in 2020, for CPSC457.
/// =========================================================================
/// You need to edit this file.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the analyzeDir() function as
/// defined in "analyzeDir.h".

#include "analyzeDir.h"

#include <cassert>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/// check if path refers to a directory
static bool is_dir(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISDIR(buff.st_mode);
}

/// check if path refers to a file
static bool is_file(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISREG(buff.st_mode);
}

/// check if string ends with another string
static bool ends_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

// ======================================================================
// You need to re-implement this function !!!!
// ======================================================================
//
// analyzeDir(n) computes stats about current directory
//   n = how many words and images to report in restuls
//
// The code below is incomplate/incorrect. Feel free to re-use any of it
// in your implementation, or delete all of it.
Results analyzeDir(int n)
{
  // The code below does a subset of the functionality you need to implement.
  // It also contains several debugging printf() statements. When you submit
  // your code for grading, please remove any dubugging pritf() statements.
  std::string dir_name = ".";
  auto dir = opendir(dir_name.c_str());
  assert(dir != nullptr);
  for (auto de = readdir(dir); de != nullptr; de = readdir(dir)) {
    std::string name = de->d_name;
    // skip . and .. entries
    if (name == "." or name == "..") continue;
    printf(" - \"%s\"\n", name.c_str());

    // check if this is directory
    if (is_dir(name)) {
      printf("    - is a directory\n");
      continue;
    }
    // make sure this is a file
    if( not is_file(name)) {
      printf("    - not a file and not a directory!!!\n");
      continue;
    }
    // report size of the file
    {
      struct stat buff;
      if (0 != stat(name.c_str(), &buff)) 
        printf("    - could not determine size of file\n");
      else
        printf("    - file size: %ld\n", long(buff.st_size));

    }
    // check if filename ends with .txt
    if (ends_with(name, ".txt")) 
      printf("    - is a .txt file\n");
    else
      printf("    - is not a .txt file\n");

    // let's see if this is an image and whether we can manage to get image info
    // by calling an external utility 'identify'
    std::string cmd = "identify -format '%wx%h' " + name + " 2> /dev/null";
    std::string img_size;
    auto fp = popen(cmd.c_str(), "r");
    if( fp) {
      char buff[PATH_MAX];
      if( fgets(buff, PATH_MAX, fp) != NULL) {
        img_size = buff;
      }
      int status = pclose(fp);
      if( status != 0 or img_size[0] == '0')
        img_size = "";
      if( img_size.empty())
        printf("    - not an image\n");
      else
        printf("    - image %s\n", img_size.c_str());
    }
  }
  closedir(dir);

  // The results below are all hard-coded, to show you all the fields
  // you need to calculate. You should delete all code below and
  // replace it with your own code.
  Results res;
  res.largest_file_path = "some_dir/some_file.txt";
  res.largest_file_size = 123;
  res.n_files = 321;
  res.n_dirs = 333;
  res.all_files_size = 1000000;

  res.most_common_words.push_back({ "hello", 3 });
  res.most_common_words.push_back({ "world", 1 });

  ImageInfo info1 { "img1", 640, 480 };
  res.largest_images.push_back(info1);
  res.largest_images.push_back({ "img2.png", 200, 300 });

  res.vacant_dirs.push_back("path1/subdir1");
  res.vacant_dirs.push_back("test2/xyz");
  return res;
}
