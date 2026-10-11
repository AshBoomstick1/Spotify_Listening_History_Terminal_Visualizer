#include "spotify.hpp"

// Compile command:  clear && g++ -std=c++11 main.cpp -o spotify && ./spotify

enum display_mode {
  artist_time,
  artist_hits,
  album_time,
  album_hits,
  song_time,
  song_hits
};

struct data_bar {
  std::string label;
  int value;
};

std::unordered_map<std::string, int> colors = {
  {"Row1", 236},
  {"Row2", 239},
  {"Text", 254}
};

const int scale = 200;
std::vector<data_bar> data = {};

std::string ms_to_string(const int &ms)
{
  std::string time;

  double total_seconds = (double)ms/1000;
  int hours = total_seconds/60/60;
  int minutes = (total_seconds/60/60 - hours) * 60;
  int seconds = (((total_seconds/60/60 - hours) * 60) - minutes) * 60;

  time = std::to_string(hours) + ":";
  if (minutes < 10) { time += "0";}
  time += std::to_string(minutes) + ":";
  if (seconds < 10) { time += "0";}
  time += std::to_string(seconds);

  return time;
}

bool is_sorted(const std::vector<data_bar> &data)
{
  for (int i = 1; i < data.size(); i++)
  {
    if (data[i - 1].value < data[i].value)
    {
      return false;
    }
  }

  return true;
}

void bubble_sort(std::vector<data_bar> &data)
{
  while (!is_sorted(data))
  {
    for (int i = 1; i < data.size(); i++)
    {
      if (data[i - 1].value < data[i].value)
      {
        data_bar temp = data[i];
        data[i] = data[i - 1];
        data[i - 1] = temp;
      }
      
    }
  }
}

int where_in_data(const std::string &label)
{
  for (int i = 0; i < data.size(); i++)
  {
    if (data[i].label == label)
    {
      return i;
    }
  }

  //doesn't exist yet
  return -1;
}

void skip_lines(std::basic_fstream<char> &file, const int num_of_lines) 
{
  for (int i = 0; i < num_of_lines; i++)
  {
    std::string str;
    std::getline(file, str);
  }
}

void read_data(const display_mode mode)
{
  std::fstream file;
  file.open("./data/StreamingHistory_music_0.json", std::ios::in);
  file.seekg(4, std::fstream::beg);
  std::string label;
  std::string value_str;
  int value;
  int label_idx;

  
  switch(mode)
  {
    case song_hits:
      skip_lines(file, 1);
    case artist_hits:
      skip_lines(file, 1);
      //for (int i = 0; i <= 10000; i++) //22200
      while(!file.eof())
      {
        std::getline(file, label);

        if (mode == artist_hits)
        {
          label = label.substr(18);
          label = label.substr(0, label.size() - 2);
        }
        else
        {
          label = label.substr(17);
          label = label.substr(0, label.size() - 2);
        }
        
        
        label_idx = where_in_data(label);
        if (label_idx == -1) //if the label is not already present in the dataset read
        {
          data_bar new_data;
          new_data.label = label;
          new_data.value = 1;
          data.push_back(new_data);
        }
        else
        {
          data[label_idx].value++;
        }
        skip_lines(file, 4);
      }
      break;

    case song_time:
      skip_lines(file, 1);

      std::getline(file, label);

      label = label.substr(17);
      label = label.substr(0, label.size() - 2);

      skip_lines(file, 2);
      std::getline(file, value_str);

      break;
    case artist_time:
      skip_lines(file, 2);
      while(!file.eof())
      {
        std::getline(file, label);
        label = label.substr(17);
        label = label.substr(0, label.size() - 2);

        std::getline(file, value_str);
        value_str = value_str.substr(15);
        value = std::stoi(value_str);
              
        label_idx = where_in_data(label);
        if (label_idx == -1) //if the label is not already present in the dataset read
        {
          data_bar new_data;
          new_data.label = label;
          new_data.value = value;
          data.push_back(new_data);
        }
        else
        {
          data[label_idx].value += value;
        }
        skip_lines(file, 3);
      }
      break;
  }

  file.close();
}

void display_data(const display_mode mode)
{
  bubble_sort(data);
  int scalar = data[0].value;
  int row_length = data[0].value / scalar * scale;

  if (mode == artist_time || mode == album_time || mode == song_time)
  {
    for (int i = 0; i < data.size(); i++)
    {
      int time_length = ms_to_string(data[i].value).size();
      int background_color = colors["Row1"];
      if (i % 2 == 0)
      {
        background_color = colors["Row2"];
      }
    
      row_length = (float)data[i].value / (float)scalar * scale;
      if (row_length < data[i].label.size()) //if the number is too small for the bar to contain the name and the value
      {
        for (int j = 0; j < row_length; j++)
        {
          std::cout << "\x1B[48:5:" << background_color << "m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }
      
        for (int j = row_length; j < data[i].label.size(); j++)
        {
          std::cout << "\x1B[0m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }
        
        std::cout << "  " << ms_to_string(data[i].value) << "\x1B[0m";
      }
      else if (row_length < data[i].label.size() + time_length + 4) //if the number is too small for the bar to contain the value
      {
        for (int j = 0; j < data[i].label.size(); j++)
        {
          std::cout << "\x1B[48:5:" << background_color << "m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }
      
        for (int j = data[i].label.size(); j < row_length; j++)
        {
          std::cout << "\x1B[0m\x1B[48:5:" << background_color << "m ";
        }
      
      
        std::cout << "\x1B[0m  \x1B[38:5:" << colors["Text"] << "m" << ms_to_string(data[i].value) << " \x1B[0m";
      }
      else
      {
        for (int j = 0; j < data[i].label.size(); j++)
        {
          std::cout << "\x1B[48:5:" << background_color << "m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }
      
        for (int j = data[i].label.size(); j < row_length - time_length + 3; j++)
        {
          std::cout << "\x1B[0m\x1B[48:5:" << background_color << "m ";
        }
        std::cout << "  \x1B[38:5:" << colors["Text"] << "m" << ms_to_string(data[i].value) << " \x1B[0m";
      }
      std::cout << "\n";
    }
  }
  else
  {
    for (int i = 0; i < data.size(); i++)
    {
      int background_color = colors["Row1"];
      if (i % 2 == 0)
      {
        background_color = colors["Row2"];
      }

      row_length = (float)data[i].value / (float)scalar * scale;
      if (row_length < data[i].label.size()) //if the number is too small for the bar to contain the name and the value
      {
        for (int j = 0; j < row_length; j++)
        {
          std::cout << "\x1B[48:5:" << background_color << "m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }

        for (int j = row_length; j < data[i].label.size(); j++)
        {
          std::cout << "\x1B[0m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }

        std::cout << "  " << data[i].value << "\x1B[0m";
      }
      else if (row_length < data[i].label.size() + std::log10(data[i].value) + 4) //if the number is too small for the bar to contain the value
      {
        for (int j = 0; j < data[i].label.size(); j++)
        {
          std::cout << "\x1B[48:5:" << background_color << "m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }

        for (int j = data[i].label.size(); j < row_length; j++)
        {
          std::cout << "\x1B[0m\x1B[48:5:" << background_color << "m ";
        }


        std::cout << "\x1B[0m  \x1B[38:5:" << colors["Text"] << "m" << data[i].value << " \x1B[0m";
      }
      else
      {
        for (int j = 0; j < data[i].label.size(); j++)
        {
          std::cout << "\x1B[48:5:" << background_color << "m\x1B[38:5:" << colors["Text"] << "m" << data[i].label[j];
        }

        for (int j = data[i].label.size(); j < row_length - std::log10(data[i].value) + 3; j++)
        {
          std::cout << "\x1B[0m\x1B[48:5:" << background_color << "m ";
        }
        std::cout << "  \x1B[38:5:" << colors["Text"] << "m" << data[i].value << " \x1B[0m";
      }
      std::cout << "\n";
    }
  }
}


int main()
{
  enum display_mode mode = artist_time;

  read_data(mode);
  display_data(mode);
  return 0;
}