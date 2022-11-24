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
#include <string.h>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>

int number_of_files = 0;

Results best_result{"",-1,0,0,0};
std::unordered_map<std::string, int> words;

constexpr int MAX_WORD_SIZE = 1024;

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
static bool starts_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(0, suffix.size(), suffix);
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
Results scan(const std::string &dir,int &counter,const int &n);

Results analyzeDir(int n)
{
  std::string dot = ".";
  int count = 0;
  Results res = scan(dot,count,n);
  std::sort(res.largest_images.begin(), res.largest_images.end(), [](const auto& i, const auto& j) { return (i.width*i.height > j.width*j.height); } );
  if((int)(res.largest_images.size())>n)
    res.largest_images.resize(n);

  if((int)(res.vacant_dirs.size())>n)
    res.vacant_dirs.resize(n);

        auto compare_words = [](auto& a, auto& b) {
        return a.second > b.second;
    };


    //printf("%ld\n",words.size());
    //printf("%ld\n",res.most_common_words.size());
    for (auto& word : words){
        res.most_common_words.emplace_back(word.first, word.second);
    }
    //printf("%ld\n",res.most_common_words.size());

  if ((int)(res.most_common_words.size()) > n) {
        std::partial_sort(res.most_common_words.begin(), res.most_common_words.begin() + n, res.most_common_words.end(), compare_words);
        res.most_common_words.resize(n);
    } else {
        std::sort(res.most_common_words.begin(),res.most_common_words.end(), compare_words);
    }


  //printf(" - \"%s\"\n", res.largest_file_path.c_str());
  if( starts_with(res.largest_file_path, "./"))
    res.largest_file_path = res.largest_file_path.substr(2);

  return res;

  
  
}

Results scan(const std::string &dir,int &counter,const int &n)
{
  number_of_files = 0;
  //Results best_result{"",-1,0,0,0};
  Results local_best_result {dir,0,0,0,0};
  auto dirp = opendir(dir.c_str());
  assert(dirp != nullptr);
  for (auto de = readdir(dirp); de != nullptr; de = readdir(dirp)) 
  {
    std::string name = de->d_name;
    // skip . and .. entries
    if (name == "." or name == "..") continue;
    
    std::string path = dir + "/" + name;
    
    // make sure this is a file
    if(is_file(path)) {
      // report size of the file
      number_of_files++;
      best_result.n_files++;
      struct stat buff;
      //if(0 != stat(name.c_str(), &buff))
      if(0 != stat(path.c_str(), &buff))
      {
        continue;
        //printf("    - could not determine size of file\n");
      }
      else
      {
        //printf("    - file size: %ld bytes\n", long(buff.st_size));
        local_best_result.largest_file_size = long(buff.st_size);
        if(local_best_result.largest_file_size>best_result.largest_file_size)
        {
          best_result.largest_file_size = local_best_result.largest_file_size;
          //printf(" - \"%s\"\n", path.c_str());
          best_result.largest_file_path = (path.c_str());
        }
        best_result.all_files_size = best_result.all_files_size + local_best_result.largest_file_size;
      }
      // check if filename ends with .txt
    if (ends_with(name, ".txt"))
    {
      std::ifstream file(path);
      std::string line;
      assert(file.is_open());
      while(std::getline(file,line))
      {
        std::string word;
        line.push_back(' ');
        std::transform(line.begin(),line.end(),line.begin(),::tolower);
        for(char& each : line)
        {
          if(std::isalpha(each))
          {
            word.push_back(each);
          }
          else{
            if(word.size()>=5) words[word]++;
            word.clear();
          }
        }
      }
      file.close();

    }
    else
    {
      // let's see if this is an image and whether we can manage to get image info
      //by calling an external utility 'identify'
    std::string cmd = "identify -format '%wx%h' " + path + " 2> /dev/null";
    std::string img_size;
    auto fp = popen(cmd.c_str(), "r");
    assert(fp);
    char buff[PATH_MAX];
    if( fgets(buff, PATH_MAX, fp) != NULL) {
      img_size = buff;
    }

    int status = pclose(fp);
    if( status != 0 or img_size[0] == '0')
      img_size = "";
    if( img_size.empty())
    {
      //printf("    - not an image\n");
    }
    else
    {
      std::string delimiter = "x";
      std::string width = img_size.substr(0, img_size.find(delimiter));
      std::string height = img_size.substr((img_size.find(delimiter))+1,img_size.length());


      long width_of_image = stoi(width);
      long height_of_image = stoi(height);
      if( starts_with(best_result.largest_file_path, "./"))
      {
      best_result.largest_images.push_back({path.substr(2), width_of_image, height_of_image});   
      }
      else
      {
        best_result.largest_images.push_back({path, width_of_image, height_of_image}); 
      } 
      
      //printf("    - pixels %ld\n", (height_of_image*width_of_image));
      //printf("    - image %s\n", img_size.c_str());
      
    }
    }
      continue;
    }

    //assert(is_dir(path));
    if(is_dir(path))
    {
    //printf("   \"%s\" - is a directory\n",name.c_str());
    counter++;
    auto sub_result = scan(path,counter,n);
    if(sub_result.largest_file_size >= best_result.largest_file_size)
    {
      best_result.largest_file_size = sub_result.largest_file_size;
      best_result.largest_file_path = sub_result.largest_file_path;
    }

    if(number_of_files == 0)
    {
      if( starts_with(path, "./"))
        best_result.vacant_dirs.push_back(path.substr(2));
    }
    }
  }

  
  best_result.n_dirs = counter+1;
  return best_result;
}
